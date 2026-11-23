<?php

namespace App\Services\Griot;

use Illuminate\Http\Client\ConnectionException;
use GuzzleHttp\Psr7\Response as Psr7Response;
use Illuminate\Support\Facades\Http;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Client\NetworkExceptionInterface;
use Psr\Http\Message\RequestInterface;
use Psr\Http\Message\ResponseInterface;
use RuntimeException;
use Throwable;

/**
 * Transport PSR-18 pour le SDK officiel Anthropic, appuyé sur le client HTTP de Laravel.
 *
 * Pourquoi : (1) le timeout est réellement appliqué par le transport (le SDK le dit « indicatif ») ;
 * (2) Http::fake() fonctionne dans les tests, sans appel réseau réel.
 */
class LaravelHttpTransport implements ClientInterface
{
    public function __construct(
        private readonly float $timeout,
        private readonly float $connectTimeout,
    ) {}

    public function sendRequest(RequestInterface $request): ResponseInterface
    {
        $headers = [];
        foreach ($request->getHeaders() as $name => $values) {
            $headers[$name] = implode(', ', $values);
        }

        try {
            $response = Http::withHeaders($headers)
                ->timeout($this->timeout)
                ->connectTimeout($this->connectTimeout)
                ->withBody((string) $request->getBody(), $request->getHeaderLine('Content-Type') ?: 'application/json')
                ->send($request->getMethod(), (string) $request->getUri());
        } catch (ConnectionException $e) {
            throw new class($e->getMessage(), $request, $e) extends RuntimeException implements NetworkExceptionInterface
            {
                public function __construct(string $message, private readonly RequestInterface $psrRequest, Throwable $previous)
                {
                    parent::__construct($message, 0, $previous);
                }

                public function getRequest(): RequestInterface
                {
                    return $this->psrRequest;
                }
            };
        }

        // Réponse PSR-7 neuve (corps lu une fois ici) : le SDK peut la relire sans dépendre
        // de la position du flux d'origine.
        return new Psr7Response($response->status(), $response->headers(), $response->body());
    }
}
