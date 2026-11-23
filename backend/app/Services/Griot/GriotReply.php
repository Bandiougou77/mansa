<?php

namespace App\Services\Griot;

/**
 * Résultat d'un tour de parole du griot.
 * Seuls reply, source et latencyMs sont renvoyés au client ; le reste sert aux logs et à griot:eval.
 */
final class GriotReply
{
    public const SOURCE_CLAUDE = 'claude';

    public const SOURCE_FALLBACK = 'fallback';

    /**
     * @param  array{input:int,output:int,cache_write:int,cache_read:int}|null  $usage
     * @param  string|null  $fallbackReason  no_api_key | timeout | network | api_error | filtered | empty | rate_limited
     * @param  string|null  $rawText  texte brut de Claude s'il a été filtré (eval locale uniquement, jamais renvoyé au client)
     */
    public function __construct(
        public readonly string $reply,
        public readonly string $source,
        public readonly int $latencyMs,
        public readonly ?string $fallbackReason = null,
        public readonly ?array $usage = null,
        public readonly ?float $costUsd = null,
        public readonly ?string $filterViolation = null,
        public readonly ?string $rawText = null,
        public readonly ?string $fallbackCategory = null,
    ) {}

    /** @return array{reply:string,source:string,latency_ms:int} */
    public function toClientArray(): array
    {
        return [
            'reply' => $this->reply,
            'source' => $this->source,
            'latency_ms' => $this->latencyMs,
        ];
    }
}
