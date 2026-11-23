<?php

namespace App\Services\Griot;

/** Réponse brute de Claude (avant filtre). */
final class ClaudeResult
{
    /** @param array{input:int,output:int,cache_write:int,cache_read:int} $usage */
    public function __construct(
        public readonly string $text,
        public readonly string $stopReason,
        public readonly array $usage,
        public readonly string $model,
    ) {}
}
