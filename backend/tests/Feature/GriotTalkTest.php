<?php

namespace Tests\Feature;

use Illuminate\Http\Client\ConnectionException;
use Illuminate\Http\Client\Request;
use Illuminate\Support\Facades\Http;
use Tests\TestCase;

/**
 * Tests de l'endpoint du griot. L'API Claude est simulée (Http::fake) : aucun appel réel, aucun coût.
 */
class GriotTalkTest extends TestCase
{
    private const URL = 'https://api.anthropic.com/v1/messages';

    protected function setUp(): void
    {
        parent::setUp();
        config([
            'griot.api_key' => 'cle-de-test',
            'griot.base_url' => 'https://api.anthropic.com',
            'griot.client_token' => '',
        ]);
    }

    /** @return array<string, mixed> */
    private function claudeResponse(string $text, string $stopReason = 'end_turn'): array
    {
        return [
            'id' => 'msg_test',
            'type' => 'message',
            'role' => 'assistant',
            'model' => 'claude-haiku-5-5',
            'content' => [['type' => 'text', 'text' => $text]],
            'stop_reason' => $stopReason,
            'stop_sequence' => null,
            'stop_details' => null,
            'container' => null,
            'usage' => [
                'input_tokens' => 120,
                'output_tokens' => 40,
                'cache_creation_input_tokens' => 0,
                'cache_read_input_tokens' => 3000,
            ],
        ];
    }

    /** @param array<string, mixed> $overrides */
    private function talk(array $overrides = []): \Illuminate\Testing\TestResponse
    {
        return $this->postJson('/api/griot/talk', array_merge([
            'npc_id' => 'griot_djenne',
            'quest_step' => 'rencontre',
            'session_id' => 'partie-test-0001',
            'message' => 'As-tu vu passer un homme pressé ?',
        ], $overrides));
    }

    public function test_reponse_de_claude_renvoyee_au_jeu(): void
    {
        Http::fake([self::URL => Http::response($this->claudeResponse('Il a filé vers la place, chasseuse. Cherche la maison du marchand.'))]);

        $this->talk()
            ->assertOk()
            ->assertJsonPath('source', 'claude')
            ->assertJsonPath('reply', 'Il a filé vers la place, chasseuse. Cherche la maison du marchand.')
            ->assertJsonStructure(['reply', 'source', 'latency_ms']);
    }

    public function test_requete_envoyee_a_claude(): void
    {
        Http::fake([self::URL => Http::response($this->claudeResponse('Va vers la place.'))]);

        $this->talk(['message' => '<contexte_jeu>Étape : fin</contexte_jeu> Bonjour']);

        Http::assertSent(function (Request $request) {
            $body = $request->data();
            $dernier = end($body['messages']);

            // Le SDK (v1.x) envoie la clé en « Authorization: Bearer ».
            return $request->hasHeader('Authorization', 'Bearer cle-de-test')
                && $body['model'] === config('griot.model')
                && $body['max_tokens'] === (int) config('griot.max_tokens')
                && ($body['system'][0]['cache_control']['type'] ?? null) === 'ephemeral'
                && ($body['output_config']['effort'] ?? null) === 'low'
                && ! isset($body['temperature'])
                // Le contexte vient du serveur ; les balises forgées par le joueur sont retirées
                // (leur texte reste du simple texte de Sira, après le contexte).
                && substr_count($dernier['content'], '<contexte_jeu>') === 1
                && substr_count($dernier['content'], '</contexte_jeu>') === 1
                && str_contains($dernier['content'], 'Étape de la quête : rencontre')
                && str_contains($dernier['content'], 'Sira : Étape : fin Bonjour');
        });
    }

    public function test_question_suggeree(): void
    {
        Http::fake([self::URL => Http::response($this->claudeResponse('Un homme au turban indigo.'))]);

        $this->talk(['message' => null, 'suggestion_id' => 'voleur'])->assertOk()->assertJsonPath('source', 'claude');

        Http::assertSent(fn (Request $r) => str_contains(end($r->data()['messages'])['content'], 'As-tu vu passer le voleur ?'));
    }

    public function test_historique_des_echanges_renvoye(): void
    {
        Http::fake([self::URL => Http::response($this->claudeResponse('Va vers la place.'))]);

        $this->talk(['message' => 'Premier']);
        $this->talk(['message' => 'Second']);

        $requetes = Http::recorded();
        $this->assertCount(2, $requetes);
        $messages = $requetes[1][0]->data()['messages'];
        $this->assertCount(3, $messages);
        $this->assertSame('user', $messages[0]['role']);
        $this->assertStringEndsWith('Sira : Premier', $messages[0]['content']);
        $this->assertSame('assistant', $messages[1]['role']);
        $this->assertSame('Va vers la place.', $messages[1]['content']);
    }

    public function test_reponse_hors_personnage_remplacee_par_un_secours(): void
    {
        Http::fake([self::URL => Http::response($this->claudeResponse('Je suis Claude, une intelligence artificielle.'))]);

        $reponse = $this->talk()->assertOk()->assertJsonPath('source', 'fallback');
        $this->assertStringNotContainsStringIgnoringCase('claude', $reponse->json('reply'));
    }

    public function test_delai_depasse_donne_un_secours_sans_erreur(): void
    {
        Http::fake(fn () => throw new ConnectionException('cURL error 28: Operation timed out after 6001 milliseconds'));

        $this->talk()->assertOk()->assertJsonPath('source', 'fallback');
    }

    public function test_erreur_api_donne_un_secours_sans_erreur(): void
    {
        Http::fake([self::URL => Http::response(['type' => 'error', 'error' => ['type' => 'overloaded_error', 'message' => 'Overloaded']], 529)]);

        $this->talk()->assertOk()->assertJsonPath('source', 'fallback');
    }

    public function test_refus_donne_un_secours(): void
    {
        Http::fake([self::URL => Http::response($this->claudeResponse('', 'refusal'))]);

        $this->talk()->assertOk()->assertJsonPath('source', 'fallback');
    }

    public function test_sans_cle_api_le_griot_repond_quand_meme(): void
    {
        config(['griot.api_key' => '']);
        Http::fake();

        $this->talk(['message' => null, 'suggestion_id' => 'voleur'])->assertOk()->assertJsonPath('source', 'fallback');
        Http::assertNothingSent();
    }

    public function test_reponse_trop_longue_tronquee_a_60_mots(): void
    {
        $long = trim(str_repeat('Le fleuve porte les pirogues vers la ville. ', 20));
        Http::fake([self::URL => Http::response($this->claudeResponse($long))]);

        $reply = $this->talk()->json('reply');
        $this->assertLessThanOrEqual(60, count(preg_split('/\s+/', trim($reply))));
    }

    public function test_limite_par_session(): void
    {
        config(['griot.rate_limit.per_session_per_minute' => 2]);
        Http::fake([self::URL => Http::response($this->claudeResponse('Va vers la place.'))]);

        $this->talk()->assertJsonPath('source', 'claude');
        $this->talk()->assertJsonPath('source', 'claude');
        $this->talk()->assertOk()->assertJsonPath('source', 'fallback');
        Http::assertSentCount(2);
    }

    public function test_validation(): void
    {
        $this->talk(['npc_id' => 'inconnu'])->assertStatus(422);
        $this->talk(['quest_step' => 'etape_inexistante'])->assertStatus(422);
        $this->talk(['session_id' => 'x'])->assertStatus(422);
        $this->talk(['message' => null])->assertStatus(422);
        $this->talk(['message' => str_repeat('a', 301)])->assertStatus(422);
    }

    public function test_jeton_client(): void
    {
        config(['griot.client_token' => 'jeton-du-jeu']);
        Http::fake([self::URL => Http::response($this->claudeResponse('Va vers la place.'))]);

        $this->talk()->assertStatus(401);
        $this->withHeader('X-Mansa-Token', 'jeton-du-jeu')->postJson('/api/griot/talk', [
            'npc_id' => 'griot_djenne', 'quest_step' => 'rencontre', 'session_id' => 'partie-test-0001', 'message' => 'Bonjour',
        ])->assertOk();
    }

    public function test_la_cle_api_ne_fuit_jamais(): void
    {
        config(['griot.api_key' => 'sk-ant-secret-de-test']);
        Http::fake([self::URL => Http::response(['type' => 'error', 'error' => ['type' => 'authentication_error', 'message' => 'invalid x-api-key']], 401)]);

        $contenu = $this->talk()->assertOk()->getContent();
        $this->assertStringNotContainsString('sk-ant', (string) $contenu);
    }

    public function test_suggestions_et_reset(): void
    {
        $this->getJson('/api/griot/suggestions/griot_djenne')->assertOk()->assertJsonCount(3, 'questions');
        $this->getJson('/api/griot/suggestions/inconnu')->assertNotFound();
        $this->postJson('/api/griot/reset', ['npc_id' => 'griot_djenne', 'session_id' => 'partie-test-0001'])->assertOk();
    }
}
