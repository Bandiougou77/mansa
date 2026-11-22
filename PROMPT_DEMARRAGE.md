# Prompts à coller dans Claude Code

## 1. Premier prompt (lancement du projet)

Copie-colle ceci dans Claude Code, ouvert à la racine de ton dossier de projet :

---

Salut ! On démarre ensemble le jeu **MANSA — L'Héritier du Fleuve**. Lis d'abord `CLAUDE.md` en entier : il contient la vision, la stack, les règles et la roadmap. Les agents spécialisés sont dans `.claude/agents/`.

Je suis développeur (Laravel, Angular, Spring Boot/Kotlin, PHP, MySQL, Git/Gitflow) mais **débutant sur Unreal Engine et Blender** : explique-moi les étapes dans l'éditeur de façon claire et numérotée.

Ce que je veux pour cette première session (Phase 0) :

1. Vérifie mon environnement : `git`, `git lfs`, la version d'Unreal Engine installée si tu peux la détecter, PHP et Composer. Dis-moi ce qui manque et comment l'installer.
2. Crée la structure de dossiers du `CLAUDE.md`, un `.gitignore` adapté à Unreal + Laravel, et un `.gitattributes` pour Git LFS (`.uasset`, `.umap`, `.fbx`, `.wav`, `.png`, etc.). Initialise le dépôt avec une branche `develop` (Gitflow).
3. Donne-moi les étapes exactes pour créer le projet **Unreal Engine 5 en C++** nommé `Mansa` (template Third Person) dans le dossier `/Mansa`.
4. Utilise l'agent `directeur-creatif` pour rédiger `/design/GDD.md` v1, centré sur la **vertical slice Djenné** (Phase 1), et l'agent `historien-mande` pour créer `/design/lore/glossaire.md` et `/design/lore/djenne-1324.md`.
5. Propose-moi un **plan de la Phase 1** découpé en tâches de 1 à 3 heures, dans l'ordre, avec l'agent responsable de chaque tâche.
6. Mets à jour la section « État actuel » du `CLAUDE.md`.

Avant de créer quoi que ce soit, montre-moi ton plan et attends mon accord.

---

## 2. Prompts utiles pour la suite

**Coder une mécanique**
> Utilise `directeur-creatif` pour écrire la fiche `/design/mecaniques/escalade.md`, puis `dev-gameplay-unreal` pour l'implémenter en C++ dans un `UMansaClimbComponent`. Termine par les étapes dans l'éditeur et comment tester. Ensuite, fais relire par `qa-relecteur`.

**Faire parler le griot**
> Utilise `historien-mande` pour définir la personnalité du griot (nom, âge, style de parole, ce qu'il sait), puis `ingenieur-ia-pnj` pour créer le backend Laravel avec l'endpoint `/api/griot/raconter` et son prompt système. Donne-moi un exemple `curl` pour tester, puis demande à `dev-gameplay-unreal` le code C++ pour l'appeler depuis le jeu.

**Créer des assets**
> Utilise `tech-art-blender` pour écrire un script Python Blender qui génère des maisons de Djenné en banco paramétrables (hauteur, nombre de toron, portes), et le pipeline d'export vers Unreal.

**Combat rythmique**
> Utilise `sound-designer` pour définir la grille rythmique et l'event FMOD du combat, puis `dev-gameplay-unreal` pour le `UMansaRhythmComponent` qui juge le timing du joueur.

**Fin de session (à faire à chaque fois)**
> Fais le point : qu'est-ce qu'on a fait, qu'est-ce qui reste, quels sont les problèmes ouverts ? Mets à jour « État actuel » dans `CLAUDE.md` et fais un commit propre.
