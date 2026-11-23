<?php

namespace App\Services\Griot;

use Illuminate\Support\Facades\Cache;

/**
 * Historique court par PNJ et par session (GDD §6.3 : 6 derniers échanges), stocké dans le cache
 * Laravel (fichier par défaut). Pas de mémoire persistante entre sessions (hors scope de la slice).
 */
class ConversationHistory
{
    /** @return list<array{user:string,assistant:string}> */
    public function get(string $npcId, string $sessionKey): array
    {
        $history = Cache::get($this->key($npcId, $sessionKey), []);

        return is_array($history) ? array_values($history) : [];
    }

    public function append(string $npcId, string $sessionKey, string $userContent, string $assistantReply): void
    {
        $history = $this->get($npcId, $sessionKey);
        $history[] = ['user' => $userContent, 'assistant' => $assistantReply];
        $history = array_slice($history, -max(1, (int) config('griot.history_exchanges', 6)));

        Cache::put($this->key($npcId, $sessionKey), $history, now()->addMinutes((int) config('griot.history_ttl_minutes', 120)));
    }

    public function forget(string $npcId, string $sessionKey): void
    {
        Cache::forget($this->key($npcId, $sessionKey));
    }

    private function key(string $npcId, string $sessionKey): string
    {
        return "griot:hist:{$npcId}:{$sessionKey}";
    }
}
