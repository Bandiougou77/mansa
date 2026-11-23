# MANSA — L'Héritier du Fleuve

> Ce fichier est lu automatiquement par Claude Code à chaque session. Il définit la vision, la stack et les règles du projet.

## Vision

Jeu d'action-aventure à la troisième personne, inspiré d'Uncharted et d'Assassin's Creed, situé dans l'**Empire du Mali en 1324**.

- **Pitch** : pendant le pèlerinage de Mansa Moussa vers La Mecque, une relique sacrée est volée. Sira, jeune chasseuse donso (exception que le griot commente, en écho à la femme-buffle du Do), remonte la piste jusqu'au mystère de la **flotte disparue d'Aboubakari II**.
- **Narrateur** : un griot raconte l'épopée des siècles plus tard. Il commente, corrige (« non, ce n'est pas comme ça que ça s'est passé… ») et adapte l'épopée chantée aux choix du joueur.
- **Ton** : épique, respectueux, authentique. Aucun cliché « Afrique générique ».

## Piliers de gameplay (ne jamais les perdre de vue)

1. **Le griot narrateur** : narration dynamique, épopée finale qui dépend des choix.
2. **Le nyama** : force spirituelle gagnée en combat et à la chasse ; trop en accumuler corrompt.
3. **Combat rythmique** : frapper en rythme avec le djembé déclenche des combos plus forts.
4. **Monde saisonnier** : crue et décrue du Niger, crépissage annuel de Djenné qui change l'escalade.
5. **Mémoire orale** : les devises (*fasa*) et les chants servent de clés (dialogues, portes, alliances).

## Stack technique

| Domaine | Outil |
|---|---|
| Moteur | Unreal Engine **5.8** (5.8.4) — C++ + Blueprints |
| IDE C++ | Visual Studio 2026 (« Développement de jeux en C++ ») |
| 3D | Blender (scripts Python pour automatiser) |
| Textures | Substance Painter, Quixel Megascans |
| Audio | FMOD Studio |
| Backend IA des PNJ | Laravel (PHP) + API Claude |
| Voix | ElevenLabs (TTS), Whisper (STT) |
| Versioning | Git + Git LFS, workflow Gitflow |

## Structure du dépôt

```
/Mansa/                 → projet Unreal (Source/, Content/, Config/)
/Mansa/Source/Mansa/    → code C++ du jeu
/backend/               → API Laravel des PNJ (griot, personnages)
/tools/blender/         → scripts Python Blender
/tools/unreal/          → scripts Python éditeur Unreal
/design/                → GDD, lore, fiches de mécaniques (Markdown)
/design/lore/           → personnages, lieux, chronologie, glossaire
/audio/                 → projet FMOD
```

## Conventions

- **C++ Unreal** : préfixes standard (`A` acteurs, `U` objets/composants, `F` structs, `E` enums). Classes du jeu préfixées `Mansa` (ex. `AMansaCharacter`, `UMansaNyamaComponent`).
- **Exposer au Blueprint** tout ce que le développeur voudra régler sans recompiler (`UPROPERTY(EditAnywhere, BlueprintReadWrite)`).
- **Assets** : `SM_` static mesh, `SK_` skeletal mesh, `M_` material, `MI_` material instance, `T_` texture, `BP_` blueprint, `ABP_` anim blueprint, `DT_` data table.
- **Données de design** (dialogues, devises, stats) dans des Data Tables ou des fichiers JSON/CSV, jamais en dur dans le code.
- **Commits** : messages en français, format `type(zone): description` (ex. `feat(combat): ajoute la fenêtre de rythme`).

## Règles de travail pour Claude

1. **Toujours travailler vers la vertical slice actuelle** (voir Roadmap). Refuser poliment le scope creep et le noter dans `/design/idees-plus-tard.md`.
2. **Expliquer avant les gros changements** : architecture nouvelle, suppression de fichiers, dépendance ajoutée → proposer un plan court et attendre mon accord.
3. **Claude ne pilote pas l'éditeur Unreal à la souris.** Il écrit du C++, des scripts Python éditeur et des fichiers de config, puis me donne des **étapes claires et numérotées** pour ce que je dois faire à la main dans l'éditeur (créer un Blueprint, assigner un mesh, etc.).
4. **Sécurité** : la clé API Claude n'est **jamais** dans le client du jeu. Seul le backend Laravel la connaît (`.env`, jamais commité).
5. **Authenticité historique** : tout élément de lore passe par l'agent `historien-mande`. Si un fait est incertain, le dire et proposer une option « inspirée de » assumée.
6. **Petites étapes testables** : chaque tâche se termine par « comment tester ça ».
7. Je communique en **français**. Réponses directes et pratiques.

## Agents disponibles (`.claude/agents/`)

| Agent | Rôle |
|---|---|
| `directeur-creatif` | Vision, GDD, arbitrages de design, cohérence des piliers |
| `historien-mande` | Exactitude historique et culturelle (Mali 1324, Mandé, Sahel) |
| `dev-gameplay-unreal` | C++ / Blueprints Unreal : personnage, combat, escalade, systèmes |
| `ingenieur-ia-pnj` | Backend Laravel + API Claude, griot et PNJ intelligents |
| `tech-art-blender` | Pipeline d'assets, scripts Blender, matériaux, optimisation |
| `sound-designer` | FMOD, musique dynamique, combat au djembé |
| `qa-relecteur` | Relecture de code, performance, bugs, tests |

## Roadmap

- **Phase 0 — Fondations** : dépôt Git + LFS, projet UE5 C++, structure de dossiers, GDD v1.
- **Phase 1 — Vertical slice « Djenné » (objectif actuel)** : une ruelle de Djenné et la façade de la grande mosquée ; déplacement + escalade ; un combat contre 2 ennemis avec rythme djembé ; un PNJ griot qui répond via l'API Claude ; 10-15 minutes de jeu.
- **Phase 2 — Systèmes** : nyama, mémoire orale (fasa), saisons.
- **Phase 3 — Contenu** : Niani, Tombouctou, caravane de sel, mines de Bambouk.
- **Phase 4 — Finale** : la piste d'Aboubakari II, l'océan.

## État actuel

_(À mettre à jour à la fin de chaque session : ce qui est fait, ce qui reste, les problèmes ouverts.)_

_Dernière mise à jour : 2026-11-23 (session 1)._

- **Phase** : 0 terminée → prochaine étape : Phase 1, tâche **A1** de `design/plan-phase1.md`.
- **Fait** :
  - Environnement : Git 2.52, Git LFS 3.7.1, PHP 8.2, Composer 2.9, UE 5.8.4, VS 2026 (« Développement de jeux en C++ »).
  - Dépôt Git (Gitflow : `main` + `develop`), `.gitignore` Unreal/Laravel, `.gitattributes` LFS, structure de dossiers.
  - Projet UE 5.8 C++ `Mansa` (template Third Person, avec les variantes Combat/Platforming/SideScrolling ajoutées par le template) dans `/Mansa`, commité, assets en LFS.
  - `design/GDD.md` v1.2 (relu par historien-mande), `design/lore/glossaire.md`, `design/lore/djenne-1324.md`, `design/plan-phase1.md`.
  - Décisions : djembé gardé et assumé par le narrateur ; escalade + combat sur une maison de marchand, Sira refuse de grimper sur la mosquée (décor) ; Sira chasseuse, couteau + lance courte ; griot PNJ au ngoni.
- **En cours** : —
- **À faire bientôt** :
  - Proposer un dépôt GitHub privé comme sauvegarde (attention au quota LFS gratuit).
  - Trancher GDD Q12 (hauteur de la maison du marchand) en tâche C1.
  - Vérifier la disponibilité du plugin FMOD pour UE 5.8 (tâche E0).
- **Points ouverts** :
  - Relectures extérieures à trouver : locuteur (prénom Sira, mots mandingues), connaisseur des donso, lecteur musulman malien (scène du refus).
  - `EngineAssociation` du `.uproject` est un GUID local (moteur enregistré hors Launcher) : à remplacer par `"5.8"` si on ouvre le projet sur une autre machine.
  - Le lore a été rédigé sans recherche en ligne : vérifier les points signalés dans les fiches (Hunwick 1999, Bedaux 1994, Marchand 2009).
- **Bloquants** : aucun.
