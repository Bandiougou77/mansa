<?php

namespace App\Http\Controllers;

use App\Http\Requests\GriotTalkRequest;
use App\Services\Griot\GriotService;
use App\Services\Griot\NpcRepository;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;

class GriotController extends Controller
{
    public function __construct(private readonly GriotService $griot) {}

    /** POST /api/griot/talk → {"reply", "source": "claude"|"fallback", "latency_ms"} */
    public function talk(GriotTalkRequest $request): JsonResponse
    {
        $reply = $this->griot->talk(
            (string) $request->validated('npc_id'),
            (string) $request->validated('quest_step'),
            (string) $request->validated('session_id'),
            $request->validated('message'),
            $request->validated('suggestion_id'),
        );

        return response()->json($reply->toClientArray(), 200, [], JSON_UNESCAPED_UNICODE);
    }

    /** POST /api/griot/reset : oublie la conversation de cette session. */
    public function reset(Request $request, NpcRepository $npcs): JsonResponse
    {
        $data = $request->validate([
            'npc_id' => ['required', 'string', 'in:'.implode(',', $npcs->npcIds())],
            'session_id' => ['required', 'string', 'regex:/^[A-Za-z0-9_-]{8,64}$/'],
        ]);
        $this->griot->reset($data['npc_id'], $data['session_id']);

        return response()->json(['ok' => true]);
    }

    /** GET /api/griot/suggestions/{npc} : les questions suggérées (le jeu peut aussi les avoir en Data Table). */
    public function suggestions(string $npc, NpcRepository $npcs): JsonResponse
    {
        abort_unless($npcs->exists($npc), 404);

        return response()->json([
            'npc_id' => $npc,
            'questions' => array_values((array) ($npcs->knowledge($npc)['questions_suggerees'] ?? [])),
        ], 200, [], JSON_UNESCAPED_UNICODE);
    }
}
