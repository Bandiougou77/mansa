<?php

namespace App\Services\Griot;

/**
 * Estimation du coût d'un appel (USD) à partir des tarifs de config/griot.php.
 */
class CostEstimator
{
    /** @param array{input:int,output:int,cache_write:int,cache_read:int} $usage */
    public function estimate(string $model, array $usage): ?float
    {
        $prices = config("griot.pricing.{$model}");
        if (! is_array($prices)) {
            return null;
        }

        $cost = ($usage['input'] * $prices['input']
            + $usage['output'] * $prices['output']
            + $usage['cache_write'] * $prices['cache_write']
            + $usage['cache_read'] * $prices['cache_read']) / 1_000_000;

        return round($cost, 6);
    }
}
