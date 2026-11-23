<?php

namespace App\Providers;

use App\Services\Griot\ClaudeGateway;
use App\Services\Griot\NpcRepository;
use App\Services\Griot\OutputFilter;
use Illuminate\Cache\RateLimiting\Limit;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\RateLimiter;
use Illuminate\Support\ServiceProvider;

class AppServiceProvider extends ServiceProvider
{
    public function register(): void
    {
        // Griot : fiches chargées une fois, client Claude réutilisé.
        $this->app->singleton(NpcRepository::class);
        $this->app->singleton(ClaudeGateway::class);
        $this->app->bind(OutputFilter::class, fn () => new OutputFilter((int) config('griot.max_words', 60)));
    }

    public function boot(): void
    {
        // Limite par adresse IP sur /api/griot/* (en plus des limites par session du GriotService).
        RateLimiter::for('griot-ip', fn (Request $request) => Limit::perMinute((int) config('griot.rate_limit.per_ip_per_minute', 30))
            ->by((string) $request->ip())
            ->response(fn () => response()->json(['error' => 'trop_de_requetes'], 429)));
    }
}
