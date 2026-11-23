<?php

namespace App\Services\Griot;

use Illuminate\Support\Facades\Cache;

/**
 * Choisit une réplique de secours (GDD §6.3 : >= 15 répliques, 3 par question suggérée + génériques).
 * Ordre : catégorie explicite (question suggérée) > mots-clés du message > génériques.
 * Les répliques tournent par session pour éviter les répétitions.
 */
class FallbackPicker
{
    public function __construct(
        private readonly NpcRepository $npcs,
        private readonly OutputFilter $filter,
    ) {}

    /** @return array{0:string,1:string} [réplique, catégorie] */
    public function pick(string $npcId, string $sessionKey, string $message = '', ?string $category = null): array
    {
        $data = $this->npcs->fallback($npcId);
        $categories = (array) ($data['categories'] ?? []);

        $category = ($category !== null && ! empty($categories[$category]))
            ? $category
            : $this->detect($message, (array) ($data['mots_cles'] ?? []), $categories);

        $lines = array_values((array) ($categories[$category] ?? $categories['generiques'] ?? []));
        if ($lines === []) {
            return ['…', 'vide'];
        }

        $counterKey = "griot:fb:{$npcId}:{$sessionKey}:{$category}";
        $index = (int) Cache::get($counterKey, 0);
        Cache::put($counterKey, $index + 1, now()->addMinutes((int) config('griot.history_ttl_minutes', 120)));

        return [(string) $lines[$index % count($lines)], $category];
    }

    /**
     * @param  array<string, list<string>>  $keywords
     * @param  array<string, mixed>  $categories
     */
    private function detect(string $message, array $keywords, array $categories): string
    {
        $normalized = $this->filter->normalize($message);

        foreach ($keywords as $category => $words) {
            if (empty($categories[$category])) {
                continue;
            }
            foreach ($words as $word) {
                if ($word !== '' && str_contains($normalized, $this->filter->normalize($word))) {
                    return $category;
                }
            }
        }

        return 'generiques';
    }
}
