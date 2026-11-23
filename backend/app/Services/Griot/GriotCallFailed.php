<?php

namespace App\Services\Griot;

use RuntimeException;
use Throwable;

/** Échec d'un appel à Claude, avec une raison courte pour les logs (timeout, network, api_error...). */
final class GriotCallFailed extends RuntimeException
{
    public function __construct(
        public readonly string $reason,
        public readonly ?int $status = null,
        ?Throwable $previous = null,
    ) {
        parent::__construct("Appel au griot échoué : {$reason}".($status ? " (HTTP {$status})" : ''), 0, $previous);
    }
}
