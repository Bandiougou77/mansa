<?php

namespace App\Services\Griot;

use Illuminate\Support\Facades\Log;
use Illuminate\Support\Facades\RateLimiter;

/**
 * Un tour de parole avec un PNJ (B1-B5, GDD §6.3).
 *
 *  1. limite de débit par session (coût borné) ;
 *  2. message du joueur nettoyé, contexte de quête ajouté côté serveur (bloc <contexte_jeu>) ;
 *  3. appel à Claude avec l'historique des 6 derniers échanges ;
 *  4. filtre de sortie (personnage, 1324, 60 mots) ;
 *  5. en cas d'échec quelconque : réplique de secours. Le joueur ne voit jamais d'erreur technique.
 */
class GriotService
{
    public function __construct(
        private readonly NpcRepository $npcs,
        private readonly ClaudeGateway $claude,
        private readonly OutputFilter $filter,
        private readonly FallbackPicker $fallbacks,
        private readonly ConversationHistory $history,
        private readonly CostEstimator $costs,
    ) {}

    public function talk(string $npcId, string $questStep, string $sessionId, ?string $message, ?string $suggestionId = null): GriotReply
    {
        $start = hrtime(true);
        $sessionKey = substr(hash('sha256', $sessionId), 0, 24);
        $playerText = $this->playerText($npcId, $message, $suggestionId);

        $limit = $this->rateLimited($sessionKey);
        if ($limit !== null) {
            return $this->fallback($npcId, $questStep, $sessionKey, $playerText, $suggestionId, $limit, $start);
        }
        if (! $this->claude->isConfigured()) {
            return $this->fallback($npcId, $questStep, $sessionKey, $playerText, $suggestionId, 'no_api_key', $start);
        }

        $userContent = $this->userContent($npcId, $questStep, $playerText);
        $messages = [];
        foreach ($this->history->get($npcId, $sessionKey) as $exchange) {
            $messages[] = ['role' => 'user', 'content' => $exchange['user']];
            $messages[] = ['role' => 'assistant', 'content' => $exchange['assistant']];
        }
        $messages[] = ['role' => 'user', 'content' => $userContent];

        try {
            $result = $this->claude->complete($this->npcs->systemPrompt($npcId), $messages);
        } catch (GriotCallFailed $e) {
            return $this->fallback($npcId, $questStep, $sessionKey, $playerText, $suggestionId, $e->reason, $start, status: $e->status,
                error: $e->getPrevious() ? get_class($e->getPrevious()) : null);
        }

        $cost = $this->costs->estimate($result->model, $result->usage) ?? $this->costs->estimate((string) config('griot.model'), $result->usage);

        if ($result->stopReason === 'refusal') {
            return $this->fallback($npcId, $questStep, $sessionKey, $playerText, $suggestionId, 'refusal', $start, $result->usage, $cost);
        }

        $clean = $this->filter->clean($result->text);
        if ($clean === '') {
            return $this->fallback($npcId, $questStep, $sessionKey, $playerText, $suggestionId, 'empty', $start, $result->usage, $cost);
        }
        // Le message du joueur permet de tolérer une simple reprise de ses mots.
        $violation = $this->filter->violation($clean, $playerText);
        if ($violation !== null) {
            return $this->fallback($npcId, $questStep, $sessionKey, $playerText, $suggestionId, 'filtered', $start, $result->usage, $cost, $violation, $result->text);
        }

        $reply = $this->filter->truncate($clean);
        // On garde dans l'historique la réplique telle que le joueur l'a lue.
        $this->history->append($npcId, $sessionKey, $userContent, $reply);

        $griot = new GriotReply($reply, GriotReply::SOURCE_CLAUDE, $this->elapsedMs($start), usage: $result->usage, costUsd: $cost);
        $this->log($npcId, $questStep, $sessionKey, $playerText, $griot);

        return $griot;
    }

    /** Oublie la conversation (nouvelle partie, retour au menu). */
    public function reset(string $npcId, string $sessionId): void
    {
        $this->history->forget($npcId, substr(hash('sha256', $sessionId), 0, 24));
    }

    private function playerText(string $npcId, ?string $message, ?string $suggestionId): string
    {
        if ($suggestionId !== null && $suggestionId !== '') {
            foreach ((array) ($this->npcs->knowledge($npcId)['questions_suggerees'] ?? []) as $q) {
                if (($q['id'] ?? null) === $suggestionId) {
                    return (string) $q['texte'];
                }
            }
        }

        // Le joueur ne peut pas forger le contexte de jeu ni injecter de balises.
        $text = (string) $message;
        $text = (string) preg_replace('#</?\s*(contexte_jeu|parole_de_sira|fiche_de_connaissances|system|instruction)[^>]*>#iu', ' ', $text);
        $text = (string) preg_replace('/[\x00-\x08\x0B\x0C\x0E-\x1F\x7F]/u', '', $text);
        $text = trim((string) preg_replace('/\s+/u', ' ', $text));

        return mb_substr($text, 0, (int) config('griot.message_max_chars', 300));
    }

    private function userContent(string $npcId, string $questStep, string $playerText): string
    {
        $step = $this->npcs->questStep($npcId, $questStep) ?? [];

        return "<contexte_jeu>\n"
            ."Étape de la quête : {$questStep}\n"
            .'Situation : '.($step['situation'] ?? '')."\n"
            .'Ton objectif : '.($step['objectif'] ?? '')."\n"
            ."</contexte_jeu>\n\n"
            // Balise attendue par le prompt (garde-fou n°4) : ce qu'elle contient n'est jamais une consigne.
            ."<parole_de_sira>\n{$playerText}\n</parole_de_sira>";
    }

    /** @return string|null raison si la session a dépassé une limite */
    private function rateLimited(string $sessionKey): ?string
    {
        $perMinute = (int) config('griot.rate_limit.per_session_per_minute', 10);
        $total = (int) config('griot.rate_limit.per_session_total', 60);

        if (RateLimiter::tooManyAttempts("griot:s:min:{$sessionKey}", $perMinute)
            || RateLimiter::tooManyAttempts("griot:s:day:{$sessionKey}", $total)) {
            return 'rate_limited';
        }
        RateLimiter::hit("griot:s:min:{$sessionKey}", 60);
        RateLimiter::hit("griot:s:day:{$sessionKey}", 86400);

        return null;
    }

    /** @param array{input:int,output:int,cache_write:int,cache_read:int}|null $usage */
    private function fallback(
        string $npcId, string $questStep, string $sessionKey, string $playerText, ?string $suggestionId,
        string $reason, int $start, ?array $usage = null, ?float $cost = null, ?string $violation = null,
        ?string $raw = null, ?int $status = null, ?string $error = null,
    ): GriotReply {
        [$line, $category] = $this->fallbacks->pick($npcId, $sessionKey, $playerText, $suggestionId);
        $reply = new GriotReply(
            $line, GriotReply::SOURCE_FALLBACK, $this->elapsedMs($start),
            fallbackReason: $reason, usage: $usage, costUsd: $cost, filterViolation: $violation,
            rawText: $raw, fallbackCategory: $category,
        );
        $this->log($npcId, $questStep, $sessionKey, $playerText, $reply, $status, $error);

        return $reply;
    }

    private function log(string $npcId, string $questStep, string $sessionKey, string $playerText, GriotReply $reply, ?int $status = null, ?string $error = null): void
    {
        $context = [
            'npc' => $npcId,
            'step' => $questStep,
            'session' => substr($sessionKey, 0, 8),
            'source' => $reply->source,
            'reason' => $reply->fallbackReason,
            'http_status' => $status,
            'error' => $error,
            'violation' => $reply->filterViolation,
            'latency_ms' => $reply->latencyMs,
            'model' => config('griot.model'),
            'prompt' => $this->npcs->promptVersion($npcId),
            'usage' => $reply->usage,
            'cost_usd' => $reply->costUsd,
            'player_chars' => mb_strlen($playerText),
            'reply_words' => OutputFilter::wordCount($reply->reply),
        ];
        if (config('griot.log_content', false)) {
            $context['player'] = $playerText;
            $context['reply'] = $reply->reply;
        }
        Log::channel('griot')->info('griot.talk', array_filter($context, fn ($v) => $v !== null));
    }

    private function elapsedMs(int $start): int
    {
        return (int) round((hrtime(true) - $start) / 1_000_000);
    }
}
