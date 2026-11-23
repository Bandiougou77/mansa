# MANSA — Backend du griot (Laravel 12 + API Claude)

Le jeu n'appelle **que** ce serveur. La clé API Claude vit uniquement dans `backend/.env`
(jamais commitée, jamais dans le client Unreal, jamais renvoyée ni loguée).

## Installation (une fois)

```bash
cd backend
composer install
cp .env.example .env          # puis renseigner ANTHROPIC_API_KEY (facultatif)
php artisan key:generate
php artisan migrate           # base SQLite (cache, sessions)
```

Sans clé API, tout fonctionne : le griot répond avec ses répliques de secours.

## Lancer le serveur

```bash
php artisan serve --port=8000
```

## API

### `POST /api/griot/talk`

```json
{ "npc_id": "griot_djenne", "quest_step": "rencontre", "session_id": "partie-1234abcd", "message": "As-tu vu un homme pressé ?" }
```
ou, pour une question suggérée : `"suggestion_id": "voleur"` à la place de `message`.

Réponse (toujours HTTP 200 si la requête est valide, même si Claude est injoignable) :

```json
{ "reply": "Il a filé vers la place, chasseuse…", "source": "claude", "latency_ms": 1450 }
```

`source` vaut `fallback` quand une réplique de secours a été utilisée (pas de clé, délai de 6 s dépassé,
erreur réseau ou API, réponse rejetée par le filtre, limite atteinte). Le joueur ne voit jamais d'erreur technique.

| Champ | Règle |
|---|---|
| `npc_id` | PNJ déclaré dans `config/griot.php` |
| `quest_step` | `rencontre`, `objectif_donne`, `refus_mosquee`, `indice_trouve` |
| `session_id` | 8 à 64 caractères `[A-Za-z0-9_-]`, généré par le jeu à chaque partie |
| `message` | texte libre, 300 caractères max |
| `suggestion_id` | `voleur`, `djenne`, `chasseuse` |

Autres routes : `POST /api/griot/reset` (`npc_id`, `session_id`) oublie la conversation ;
`GET /api/griot/suggestions/griot_djenne` liste les questions suggérées.
Erreurs : `422` requête invalide, `401` jeton client manquant (si `GRIOT_CLIENT_TOKEN`), `429` trop de requêtes pour cette IP.

## Garde-fous (GDD §6.3)

- **Prompt système versionné** : `resources/prompts/griot_djenne.v1.md` + fiche de connaissances `resources/griot/griot_djenne.json` (seule source de faits, relue par historien-mande). Mis en cache côté API.
- **Contexte de quête** ajouté par le serveur (bloc `<contexte_jeu>`) ; le joueur ne peut pas le forger.
- **Historique** : 6 derniers échanges par session (cache Laravel, 2 h).
- **Filtre de sortie** (`OutputFilter`) : rejette IA, Claude, jeu, monde moderne, anachronismes, clichés ; coupe à 60 mots.
- **Coût borné** : 10 messages / minute et 60 / jour par session, 30 / minute par IP ; un seul essai, 6 s max.
- **Journal** : `storage/logs/griot-AAAA-MM-JJ.log` (latence, source, raison, tokens, coût estimé ; pas le texte, sauf `GRIOT_LOG_CONTENT=true`).

## Modèle

`GRIOT_MODEL=claude-haiku-5-5` par défaut (le plus rapide et le moins cher, adapté à des répliques
de 60 mots avec une cible de 2,5 s). Effort `low`, réflexion désactivée.
Pour comparer la qualité : `GRIOT_MODEL=claude-sonnet-5-5` ou `claude-opus-5-5` (plus lents et plus chers ;
avec Opus 5.5, mettre `GRIOT_THINKING=` vide, car il refuse `disabled`).

## Tests

```bash
php artisan test               # 17 tests, API Claude simulée : aucun appel réel, aucun coût
php artisan griot:eval         # banc de 20 questions pièges (objectif : ≥ 19/20)
```

⚠️ `griot:eval` appelle **réellement** Claude si une clé est présente (20 appels, coût affiché à la fin,
de l'ordre du centime avec Haiku). Sans clé, il vérifie seulement la chaîne de secours.
