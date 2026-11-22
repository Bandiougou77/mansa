# Kit Claude Code — MANSA

## Installation

1. Crée un dossier vide pour ton jeu, par exemple `C:\Dev\Mansa-Projet\`.
2. Copie dedans **tout le contenu de ce kit** :
   ```
   Mansa-Projet/
   ├── CLAUDE.md
   ├── PROMPT_DEMARRAGE.md
   └── .claude/
       └── agents/
           ├── directeur-creatif.md
           ├── historien-mande.md
           ├── dev-gameplay-unreal.md
           ├── ingenieur-ia-pnj.md
           ├── tech-art-blender.md
           ├── sound-designer.md
           └── qa-relecteur.md
   ```
   Attention : le dossier `.claude` commence par un point, il peut être caché. Sous Windows, active « Afficher les éléments masqués ».
3. Ouvre un terminal dans ce dossier et lance `claude`.
4. Tape `/agents` pour vérifier que les 7 agents apparaissent.
5. Ouvre `PROMPT_DEMARRAGE.md` et colle le premier prompt.

## À savoir

- Claude Code **écrit le code et les scripts**, mais c'est toi qui cliques dans l'éditeur Unreal et dans Blender. Il te donnera les étapes.
- Mets à jour la section « État actuel » de `CLAUDE.md` à chaque fin de session : c'est la mémoire du projet entre deux sessions.
- Tu peux modifier les agents à tout moment (ce sont de simples fichiers texte).
