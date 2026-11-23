<?php

namespace App\Http\Requests;

use App\Services\Griot\NpcRepository;
use Illuminate\Contracts\Validation\Validator;
use Illuminate\Foundation\Http\FormRequest;
use Illuminate\Http\Exceptions\HttpResponseException;
use Illuminate\Validation\Rule;

/**
 * Corps attendu de POST /api/griot/talk :
 *   npc_id        identifiant du PNJ (config/griot.php)
 *   quest_step    étape de la quête (fiche du PNJ, ex. « rencontre »)
 *   session_id    identifiant de partie généré par le jeu (8-64 caractères [A-Za-z0-9_-])
 *   message       texte libre du joueur (max 300 caractères)  ─┐ l'un ou l'autre
 *   suggestion_id question suggérée (ex. « voleur »)           ─┘
 */
class GriotTalkRequest extends FormRequest
{
    public function authorize(): bool
    {
        return true;
    }

    /** @return array<string, mixed> */
    public function rules(): array
    {
        $npcs = app(NpcRepository::class);
        $npcId = (string) $this->input('npc_id');
        $known = $npcs->exists($npcId);

        return [
            'npc_id' => ['required', 'string', Rule::in($npcs->npcIds())],
            'quest_step' => ['required', 'string', Rule::in($known ? $npcs->questSteps($npcId) : [])],
            'session_id' => ['required', 'string', 'regex:/^[A-Za-z0-9_-]{8,64}$/'],
            'message' => ['nullable', 'string', 'max:'.(int) config('griot.message_max_chars', 300), 'required_without:suggestion_id'],
            'suggestion_id' => ['nullable', 'string', Rule::in($known ? $npcs->suggestionIds($npcId) : [])],
        ];
    }

    /** Erreur de validation en JSON (le jeu affiche alors une réplique de secours locale). */
    protected function failedValidation(Validator $validator): void
    {
        throw new HttpResponseException(response()->json([
            'error' => 'requete_invalide',
            'details' => $validator->errors(),
        ], 422));
    }
}
