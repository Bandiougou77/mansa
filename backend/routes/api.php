<?php

use App\Http\Controllers\GriotController;
use App\Http\Middleware\VerifyMansaClientToken;
use Illuminate\Support\Facades\Route;

/*
| API du jeu MANSA (préfixe /api). Le client Unreal n'appelle que ces routes :
| la clé Claude reste dans backend/.env.
*/

Route::prefix('griot')
    ->middleware([VerifyMansaClientToken::class, 'throttle:griot-ip'])
    ->group(function () {
        Route::post('talk', [GriotController::class, 'talk']);
        Route::post('reset', [GriotController::class, 'reset']);
        Route::get('suggestions/{npc}', [GriotController::class, 'suggestions']);
    });
