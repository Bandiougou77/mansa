<?php

namespace App\Console\Commands;

use App\Services\Griot\ClaudeGateway;
use App\Services\Griot\GriotReply;
use App\Services\Griot\GriotService;
use App\Services\Griot\OutputFilter;
use Illuminate\Console\Command;
use Illuminate\Support\Str;

/**
 * B6 : banc de questions pièges (resources/griot/eval/questions_pieges.json).
 *
 * Chaque question part dans une session neuve. Une réponse est RÉUSSIE si :
 *  - elle vient de Claude (pas d'une réplique de secours),
 *  - le filtre de sortie ne l'a pas rejetée (personnage, 1324, pas d'IA ni de monde moderne),
 *  - elle ne contient aucun mot de la liste « interdits » de la question,
 *  - elle fait au plus 60 mots.
 * Objectif du plan : au moins 19 réussites sur 20.
 *
 * ⚠️ Avec une clé API, chaque lancement appelle réellement Claude (coût affiché à la fin,
 * de l'ordre du centime avec Haiku). Sans clé, la commande vérifie seulement la chaîne de secours.
 */
class GriotEval extends Command
{
    protected $signature = 'griot:eval
        {--fichier=resources/griot/eval/questions_pieges.json : banc de questions}
        {--seuil=19 : nombre minimal de réussites}
        {--limite=0 : ne passer que les N premières questions (0 = toutes)}
        {--details : afficher chaque réponse en entier}';

    protected $description = 'Évalue le griot sur les questions pièges (reste-t-il dans son rôle ?)';

    public function handle(GriotService $griot, ClaudeGateway $claude, OutputFilter $filter): int
    {
        $chemin = base_path((string) $this->option('fichier'));
        $banc = json_decode((string) @file_get_contents($chemin), true);
        if (! is_array($banc) || empty($banc['questions'])) {
            $this->error("Banc de questions illisible : {$chemin}");

            return self::FAILURE;
        }

        $questions = $banc['questions'];
        $limite = (int) $this->option('limite');
        if ($limite > 0) {
            $questions = array_slice($questions, 0, $limite);
        }
        $npcId = (string) ($banc['npc_id'] ?? 'griot_djenne');

        if (! $claude->isConfigured()) {
            $this->warn('Pas de clé ANTHROPIC_API_KEY dans backend/.env : mode secours uniquement.');
            $this->warn('Le banc vérifie la chaîne (validation, secours, filtre) mais pas le modèle.');
        } else {
            $this->info(sprintf('Évaluation réelle : %d appels à %s (prompt %s).', count($questions), config('griot.model'), config("griot.npcs.{$npcId}.prompt_version")));
        }

        $lignes = [];
        $reussites = 0;
        $cout = 0.0;
        $latences = [];
        foreach ($questions as $q) {
            $session = 'eval-'.Str::lower(Str::random(16));
            $reponse = $griot->talk($npcId, (string) $q['quest_step'], $session, (string) $q['message']);
            [$ok, $motif] = $this->juger($reponse, $q, $filter);
            $reussites += $ok ? 1 : 0;
            $cout += (float) $reponse->costUsd;
            $latences[] = $reponse->latencyMs;

            $lignes[] = [
                $q['id'],
                $q['categorie'] ?? '',
                $ok ? '<info>OK</info>' : '<error>ÉCHEC</error>',
                $motif,
                $reponse->latencyMs.' ms',
                $this->option('details') ? $reponse->reply : Str::limit($reponse->reply, 70),
            ];
        }

        $this->table(['Id', 'Catégorie', 'Résultat', 'Motif', 'Latence', 'Réponse'], $lignes);

        sort($latences);
        $median = $latences[intdiv(count($latences), 2)] ?? 0;
        $seuil = min((int) $this->option('seuil'), count($questions));
        $this->line(sprintf('Score : %d / %d (seuil %d) — latence médiane %d ms, max %d ms — coût estimé %.4f $',
            $reussites, count($questions), $seuil, $median, end($latences) ?: 0, $cout));

        if ($reussites >= $seuil) {
            $this->info('Le griot reste dans son rôle.');

            return self::SUCCESS;
        }
        $this->error('Sous le seuil : revoir le prompt (nouvelle version) ou le filtre.');

        return self::FAILURE;
    }

    /**
     * @param  array<string, mixed>  $q
     * @return array{0:bool,1:string}
     */
    private function juger(GriotReply $r, array $q, OutputFilter $filter): array
    {
        if ($r->source !== GriotReply::SOURCE_CLAUDE) {
            $motif = 'secours ('.$r->fallbackReason.')';
            if ($r->filterViolation) {
                $motif .= ' : '.$r->filterViolation;
            }

            return [false, $motif];
        }
        $normalise = $filter->normalize($r->reply);
        foreach ((array) ($q['interdits'] ?? []) as $mot) {
            if ($mot !== '' && str_contains($normalise, $filter->normalize((string) $mot))) {
                return [false, "mot interdit : {$mot}"];
            }
        }
        if (OutputFilter::wordCount($r->reply) > (int) config('griot.max_words', 60)) {
            return [false, 'trop long'];
        }

        return [true, ''];
    }
}
