<?php

namespace App\Services\Griot;

use Normalizer;

/**
 * Filtre de sortie du griot (GDD §6.3) :
 *  - nettoie la mise en forme (markdown, didascalies, étiquettes « Griot : »),
 *  - refuse toute réponse qui sort du personnage ou de 1324 (IA, Claude, jeu, monde moderne, anachronismes),
 *  - tronque à N mots (60 par défaut) en coupant si possible à la fin d'une phrase.
 *
 * Les motifs s'appliquent sur un texte en minuscules et sans accents.
 */
class OutputFilter
{
    /** @var array<string, string> libellé => motif (texte normalisé, sans accents) */
    public const FORBIDDEN = [
        'ia' => '/\bi\.?a\b/u',
        'intelligence artificielle' => '/intelligence\s+artificielle|artificial\s+intelligence/u',
        'claude/anthropic' => '/\bclaude\b|\banthropic\b/u',
        'autres IA' => '/chat\s*gpt|openai|\bgpt\b|\bllm\b|chatbot/u',
        'modèle de langage' => '/modele\s+(de\s+langage|linguistique)|language\s+model/u',
        'assistant/programme' => '/assistant\s+virtuel|\brobot|algorithme|logiciel|ordinateur|informatique|\bprogramme\s+(informatique|d.?ia)/u',
        'jeu/joueur' => '/\bjeux?\b|\bjoueu(r|se)s?\b|\bmanette|\bpnj\b|\bnpc\b|personnage\s+non/u',
        'méta-consignes' => '/\bprompt|\bconsignes?\s+(systeme|precedentes)|instructions?\s+(systeme|precedentes)|\bunreal\b|\bapi\b|\bserveur/u',
        'secret' => '/sk-ant|cle\s+api|api\s*key/u',
        'monde moderne' => '/internet|reseaux?\s+sociaux|telephone|smartphone|television|\bradio\b|voiture|avion|electricit|\bpolice\b|president|republique|\busine/u',
        'anachronisme musical' => '/\bkora\b|djembe/u',
        'anachronisme géographique' => '/amerique|\beurope|colonis|\bfrance\b/u',
        'date postérieure' => '/\b(1[4-9]\d\d|2\d\d\d)\b|\bxixe\b|\bxxe\b|\bxxie\b/u',
        'plantes américaines / thé' => '/manioc|arachide|\btabac|\bpiment|the\s+a\s+la\s+menthe/u',
        'clichés' => '/\btribus?\b|\bsorcier|\bfetiche|tam-tam|tamtam|\bsavane|\bjungle/u',
    ];

    private const ENGLISH_MARKERS = '/\b(the|and|you|your|i\'m|i am|is|are|what|with|this)\b/u';

    public function __construct(private readonly int $maxWords = 60) {}

    /** Nettoyage de forme (sans jugement de fond). */
    public function clean(string $text): string
    {
        $text = (string) preg_replace('/\*[^*\n]{1,120}\*/u', ' ', $text);              // *didascalies*
        $text = (string) preg_replace('/^\s*(le\s+)?(griot|jeli)\s*:\s*/iu', '', $text);  // « Griot : »
        $text = str_replace(['*', '#', '`'], '', $text);
        $text = (string) preg_replace('/\s+/u', ' ', $text);
        $text = trim($text);

        // Guillemets qui entourent toute la réponse
        if (preg_match('/^[«"“]\s*(.*?)\s*[»"”]$/us', $text, $m) && ! preg_match('/[«"“»”]/u', $m[1])) {
            $text = trim($m[1]);
        }

        return $text;
    }

    /** Renvoie le libellé de la première règle violée, ou null si la réponse est acceptable. */
    public function violation(string $text): ?string
    {
        $normalized = $this->normalize($text);

        foreach (self::FORBIDDEN as $label => $pattern) {
            if (preg_match($pattern, $normalized)) {
                return $label;
            }
        }

        if (preg_match_all(self::ENGLISH_MARKERS, $normalized) >= 3) {
            return 'langue (anglais)';
        }

        return null;
    }

    public function truncate(string $text): string
    {
        $words = preg_split('/\s+/u', trim($text), -1, PREG_SPLIT_NO_EMPTY) ?: [];
        if (count($words) <= $this->maxWords) {
            return trim($text);
        }

        $cut = implode(' ', array_slice($words, 0, $this->maxWords));

        // Couper à la dernière fin de phrase si elle garde au moins la moitié du texte.
        if (preg_match_all('/[.!?…](?=\s|$)/u', $cut, $m, PREG_OFFSET_CAPTURE)) {
            $last = end($m[0]);
            $end = $last[1] + strlen($last[0]);
            if ($end >= strlen($cut) / 2) {
                return trim(substr($cut, 0, $end));
            }
        }

        return rtrim($cut, ' ,;:-–—').'…';
    }

    public static function wordCount(string $text): int
    {
        return count(preg_split('/\s+/u', trim($text), -1, PREG_SPLIT_NO_EMPTY) ?: []);
    }

    public function normalize(string $text): string
    {
        $text = mb_strtolower($text, 'UTF-8');
        $text = str_replace(['œ', 'æ', '’'], ['oe', 'ae', "'"], $text);
        if (class_exists(Normalizer::class)) {
            $decomposed = Normalizer::normalize($text, Normalizer::FORM_D);
            if ($decomposed !== false) {
                $text = (string) preg_replace('/\p{Mn}+/u', '', $decomposed);
            }
        }

        return $text;
    }
}
