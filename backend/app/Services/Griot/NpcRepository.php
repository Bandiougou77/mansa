<?php

namespace App\Services\Griot;

use InvalidArgumentException;
use RuntimeException;

/**
 * Charge les données d'un PNJ (fiche de connaissances, prompt système versionné, répliques de secours).
 * Les chemins viennent de config/griot.php ; rien n'est codé en dur.
 */
class NpcRepository
{
    /** @var array<string, mixed> */
    private array $cache = [];

    public function exists(string $npcId): bool
    {
        return array_key_exists($npcId, (array) config('griot.npcs', []));
    }

    /** @return list<string> */
    public function npcIds(): array
    {
        return array_keys((array) config('griot.npcs', []));
    }

    /** @return array<string, mixed> */
    public function knowledge(string $npcId): array
    {
        return $this->json($npcId, 'knowledge');
    }

    /** @return array<string, mixed> */
    public function fallback(string $npcId): array
    {
        return $this->json($npcId, 'fallback');
    }

    /** @return list<string> */
    public function questSteps(string $npcId): array
    {
        return array_keys((array) ($this->knowledge($npcId)['etapes_quete'] ?? []));
    }

    /** @return array<string, mixed>|null */
    public function questStep(string $npcId, string $step): ?array
    {
        $steps = (array) ($this->knowledge($npcId)['etapes_quete'] ?? []);

        return isset($steps[$step]) ? (array) $steps[$step] : null;
    }

    /** @return list<string> identifiants des questions suggérées */
    public function suggestionIds(string $npcId): array
    {
        return array_values(array_map(
            fn ($q) => (string) ($q['id'] ?? ''),
            (array) ($this->knowledge($npcId)['questions_suggerees'] ?? [])
        ));
    }

    public function promptVersion(string $npcId): string
    {
        return (string) ($this->definition($npcId)['prompt_version'] ?? 'inconnue');
    }

    /**
     * Prompt système final : gabarit versionné + fiche JSON insérée. Identique pour toutes
     * les sessions (aucune donnée de session dedans) afin de profiter du prompt caching.
     */
    public function systemPrompt(string $npcId): string
    {
        $key = $npcId.':system';
        if (! isset($this->cache[$key])) {
            $template = $this->read($this->definition($npcId)['prompt']);
            $template = (string) preg_replace('/<!--.*?-->\s*/s', '', $template);
            $fiche = json_encode($this->knowledge($npcId), JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES | JSON_PRETTY_PRINT);

            $this->cache[$key] = trim(strtr($template, [
                '{{FICHE_JSON}}' => (string) $fiche,
                '{{MAX_WORDS}}' => (string) config('griot.max_words', 60),
            ]));
        }

        return $this->cache[$key];
    }

    /** @return array<string, mixed> */
    private function json(string $npcId, string $kind): array
    {
        $key = $npcId.':'.$kind;
        if (! isset($this->cache[$key])) {
            $decoded = json_decode($this->read($this->definition($npcId)[$kind]), true);
            if (! is_array($decoded)) {
                throw new RuntimeException("Fichier {$kind} invalide pour le PNJ {$npcId}.");
            }
            $this->cache[$key] = $decoded;
        }

        return $this->cache[$key];
    }

    /** @return array<string, string> */
    private function definition(string $npcId): array
    {
        $def = config("griot.npcs.{$npcId}");
        if (! is_array($def)) {
            throw new InvalidArgumentException("PNJ inconnu : {$npcId}");
        }

        return $def;
    }

    private function read(string $relativePath): string
    {
        $path = base_path($relativePath);
        if (! is_file($path)) {
            throw new RuntimeException("Fichier introuvable : {$relativePath}");
        }

        return (string) file_get_contents($path);
    }
}
