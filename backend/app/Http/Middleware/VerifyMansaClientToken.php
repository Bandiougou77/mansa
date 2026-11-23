<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

/**
 * Si GRIOT_CLIENT_TOKEN est défini, exige l'en-tête X-Mansa-Token correspondant.
 * Ce n'est pas un secret fort (il est dans le build du jeu) : il écarte les appels anonymes.
 * La vraie protection du coût, ce sont les limites de débit.
 */
class VerifyMansaClientToken
{
    public function handle(Request $request, Closure $next): Response
    {
        $expected = (string) config('griot.client_token', '');
        if ($expected !== '' && ! hash_equals($expected, (string) $request->header('X-Mansa-Token', ''))) {
            return response()->json(['error' => 'non_autorise'], 401);
        }

        return $next($request);
    }
}
