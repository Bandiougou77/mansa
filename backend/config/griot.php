<?php

/*
|--------------------------------------------------------------------------
| Griot PNJ (MANSA, vertical slice "Djenné")
|--------------------------------------------------------------------------
|
| Tous les réglages du griot sont ici et lus depuis .env. Aucune valeur de
| réglage n'est codée en dur dans les services.
|
| SÉCURITÉ : ANTHROPIC_API_KEY vit uniquement dans backend/.env (jamais
| commité, jamais renvoyé au client, jamais logué). Clé vide = mode
| fallback intégral (répliques de secours), le backend reste fonctionnel.
|
*/

$nullableFloat = static function (string $key): ?float {
    $value = env($key);

    return ($value === null || $value === '') ? null : (float) $value;
};

return [

    // --- API Claude ---------------------------------------------------------
    'api_key' => env('ANTHROPIC_API_KEY', ''),
    'base_url' => env('ANTHROPIC_BASE_URL', 'https://api.anthropic.com'),

    // Modèles vérifiés sur la doc officielle (platform.claude.com, 2026-11-23) :
    // claude-haiku-5-5 (le plus rapide, défaut), claude-sonnet-5-5, claude-opus-5-5.
    'model' => env('GRIOT_MODEL', 'claude-haiku-5-5'),

    // 60 mots en français ~ 100-130 tokens (tokenizer actuel). 200 laisse une marge
    // sans permettre de longues tirades. La réponse est de toute façon tronquée à 60 mots.
    'max_tokens' => (int) env('GRIOT_MAX_TOKENS', 200),

    // Délai total maximum d'un appel à Claude (GDD §6.3 : 6 s puis fallback).
    'timeout' => (float) env('GRIOT_TIMEOUT', 6),
    'connect_timeout' => (float) env('GRIOT_CONNECT_TIMEOUT', 2),

    // Certificats SSL (vérification toujours active). Sous Windows, PHP n'a souvent aucun
    // magasin configuré pour cURL (« cURL error 60 ») : on utilise celui de Windows.
    // GRIOT_CA_BUNDLE = chemin d'un cacert.pem, prioritaire si renseigné.
    'ssl' => [
        'ca_bundle' => env('GRIOT_CA_BUNDLE', ''),
        'native_ca' => filter_var(env('GRIOT_SSL_NATIVE_CA', PHP_OS_FAMILY === 'Windows'), FILTER_VALIDATE_BOOL),
    ],

    // ATTENTION : la version d'API 2026-10-21 (celle du SDK) a SUPPRIMÉ temperature,
    // top_p et top_k (erreur 400 sur tous les modèles). Laisser vide. N'est envoyée
    // que si une valeur est fournie (utile seulement avec une ancienne version d'API).
    'temperature' => $nullableFloat('GRIOT_TEMPERATURE'),

    // Latence : effort "low" + réflexion désactivée = réponse la plus rapide sur Haiku 5.5.
    // Valeurs effort : low | medium | high (vide = défaut du modèle).
    'effort' => env('GRIOT_EFFORT', 'low'),
    // Valeurs thinking : disabled | adaptive | (vide = ne pas envoyer).
    // Opus 5.5 refuse "disabled" (400) : mettre vide si GRIOT_MODEL=claude-opus-5-5.
    'thinking' => env('GRIOT_THINKING', 'disabled'),

    // Prompt caching du prompt système + fiche (identiques pour toutes les sessions).
    'prompt_cache' => filter_var(env('GRIOT_PROMPT_CACHE', true), FILTER_VALIDATE_BOOL),

    // --- PNJ disponibles ----------------------------------------------------
    'npcs' => [
        'griot_djenne' => [
            'knowledge' => 'resources/griot/griot_djenne.json',
            'prompt' => 'resources/prompts/griot_djenne.v2.md',
            'prompt_version' => 'griot_djenne.v2',
            'fallback' => 'resources/griot/fallback_griot_djenne.json',
        ],
    ],

    // --- Conversation -------------------------------------------------------
    'history_exchanges' => (int) env('GRIOT_HISTORY_EXCHANGES', 6),   // GDD §6.3 : 6 derniers échanges
    'history_ttl_minutes' => (int) env('GRIOT_HISTORY_TTL_MINUTES', 120),
    'max_words' => (int) env('GRIOT_MAX_WORDS', 60),                 // GDD §6.3 : 60 mots max
    'message_max_chars' => (int) env('GRIOT_MESSAGE_MAX_CHARS', 300),

    // --- Limites de débit (coût borné, GDD Q10) -----------------------------
    'rate_limit' => [
        'per_session_per_minute' => (int) env('GRIOT_RATE_SESSION_PER_MINUTE', 10),
        'per_session_total' => (int) env('GRIOT_RATE_SESSION_TOTAL', 60),     // sur 24 h
        'per_ip_per_minute' => (int) env('GRIOT_RATE_IP_PER_MINUTE', 30),
    ],

    // --- Jeton client (optionnel) --------------------------------------------
    // Si non vide, le jeu doit envoyer l'en-tête X-Mansa-Token avec cette valeur.
    // Ce n'est PAS un secret fort (il est dans le build) : il filtre les appels
    // anonymes. La vraie protection du coût est le rate limit.
    'client_token' => env('GRIOT_CLIENT_TOKEN', ''),

    // --- Logs ---------------------------------------------------------------
    // false = on ne logue que la longueur du message joueur, pas son contenu.
    'log_content' => filter_var(env('GRIOT_LOG_CONTENT', false), FILTER_VALIDATE_BOOL),

    // --- Tarifs (USD par million de tokens) pour l'estimation du coût -------
    // Source : platform.claude.com (2026-11-23), prompts < 100 000 tokens.
    // Écriture de cache Sonnet/Opus : estimée à 1,25 x l'entrée (à revérifier sur la page Pricing).
    'pricing' => [
        'claude-haiku-5-5' => ['input' => 0.10, 'output' => 0.50, 'cache_write' => 0.125, 'cache_read' => 0.01],
        'claude-sonnet-5-5' => ['input' => 2.00, 'output' => 10.00, 'cache_write' => 2.50, 'cache_read' => 0.10],
        'claude-opus-5-5' => ['input' => 4.00, 'output' => 20.00, 'cache_write' => 5.00, 'cache_read' => 0.20],
    ],
];
