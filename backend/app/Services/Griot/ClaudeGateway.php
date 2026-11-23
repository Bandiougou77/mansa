<?php

namespace App\Services\Griot;

use Anthropic\Client;
use Anthropic\Core\Exceptions\APIConnectionException;
use Anthropic\Core\Exceptions\APIStatusException;
use Anthropic\Core\Exceptions\APITimeoutException;
use Anthropic\Core\Exceptions\AuthenticationException;
use Anthropic\Core\Exceptions\RateLimitException;
use Anthropic\RequestOptions;
use Illuminate\Support\Facades\Log;
use Throwable;

/**
 * Seul point de contact avec l'API Claude (SDK officiel anthropic-ai/sdk).
 *
 * - La clé vient de config('griot.api_key') (backend/.env) : jamais logguée, jamais renvoyée.
 * - Délai total borné par LaravelHttpTransport (6 s par défaut), sans nouvelle tentative :
 *   au-delà, le griot répond avec une réplique de secours (GDD §6.3).
 * - Prompt système mis en cache (identique pour toutes les sessions).
 */
class ClaudeGateway
{
    private ?Client $client = null;

    public function isConfigured(): bool
    {
        return trim((string) config('griot.api_key', '')) !== '';
    }

    /**
     * @param  list<array{role:string,content:string}>  $messages
     *
     * @throws GriotCallFailed
     */
    public function complete(string $systemPrompt, array $messages): ClaudeResult
    {
        $model = (string) config('griot.model');
        $system = config('griot.prompt_cache', true)
            ? [['type' => 'text', 'text' => $systemPrompt, 'cacheControl' => ['type' => 'ephemeral']]]
            : $systemPrompt;

        $effort = (string) config('griot.effort', '');
        $thinking = (string) config('griot.thinking', '');

        try {
            $message = $this->client()->messages->create(
                model: $model,
                maxTokens: (int) config('griot.max_tokens', 200),
                system: $system,
                messages: $messages,
                outputConfig: $effort !== '' ? ['effort' => $effort] : null,
                thinking: $thinking !== '' ? ['type' => $thinking] : null,
            );
        } catch (APITimeoutException $e) {
            throw new GriotCallFailed('timeout', previous: $e);
        } catch (APIConnectionException $e) {
            // Le transport Laravel remonte les délais dépassés comme des erreurs de connexion (cURL 28).
            $reason = preg_match('/timed?\s*out|cURL error 28/i', $e->getMessage()) ? 'timeout' : 'network';
            throw new GriotCallFailed($reason, previous: $e);
        } catch (AuthenticationException $e) {
            Log::channel('griot')->critical('Clé API refusée par Anthropic (401) : vérifier ANTHROPIC_API_KEY dans backend/.env.');
            throw new GriotCallFailed('api_error', 401, $e);
        } catch (RateLimitException $e) {
            throw new GriotCallFailed('api_rate_limited', 429, $e);
        } catch (APIStatusException $e) {
            throw new GriotCallFailed('api_error', $e->status ?? null, $e);
        } catch (Throwable $e) {
            throw new GriotCallFailed('api_error', previous: $e);
        }

        $text = '';
        foreach ($message->content as $block) {
            if ($block->type === 'text') {
                $text .= $block->text;
            }
        }

        $usage = [
            'input' => $message->usage->inputTokens,
            'output' => $message->usage->outputTokens,
            'cache_write' => (int) ($message->usage->cacheCreationInputTokens ?? 0),
            'cache_read' => (int) ($message->usage->cacheReadInputTokens ?? 0),
        ];

        return new ClaudeResult(trim($text), (string) $message->stopReason, $usage, $message->model);
    }

    private function client(): Client
    {
        if ($this->client === null) {
            $timeout = (float) config('griot.timeout', 6);
            $this->client = new Client(
                apiKey: (string) config('griot.api_key'),
                baseUrl: (string) config('griot.base_url'),
                requestOptions: RequestOptions::with(
                    timeout: $timeout,
                    maxRetries: 0,
                    transporter: new LaravelHttpTransport($timeout, (float) config('griot.connect_timeout', 2)),
                ),
            );
        }

        return $this->client;
    }
}
