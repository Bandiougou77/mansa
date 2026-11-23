# MANSA — Contrôles

> Conventions des jeux d'aventure AAA (Assassin's Creed, Uncharted). Source de vérité : ce fichier.
> Mise en œuvre : `tools/unreal/a4_creer_inputs.py` (Input Actions + contextes Enhanced Input),
> libellés affichés : `design/data/DT_LibellesTouches.csv`.

## Clavier / souris

Disposition **détectée automatiquement** (AZERTY ou QWERTY, d'après Windows) ; forçable dans les réglages.

| Action | AZERTY | QWERTY | Remarque |
|---|---|---|---|
| Se déplacer | Z Q S D | W A S D | Flèches aussi (non remappables) |
| Caméra | Souris | Souris | |
| Courir / parkour rapide | Maj gauche (maintenir) | Maj gauche | |
| Marcher lentement | Ctrl gauche (maintenir) | Ctrl gauche | Ajout (Alt sert à l'esquive) |
| Sauter / grimper vers le haut | Espace | Espace | En escalade : saut vers la prise visée au stick / ZQSD, « se hisser » sur une corniche |
| S'accroupir | C (bascule) | C | |
| Lâcher la prise (escalade) | C | C | Le contexte Escalade prend la main sur « s'accroupir » |
| Interagir (parler, ramasser, ouvrir) | E | E | |
| Attaque légère | Clic gauche | Clic gauche | Combat : bloc E |
| Attaque lourde | Clic droit | Clic droit | Combat : bloc E |
| Parer / bloquer | **A** | **Q** | Même position physique (à droite de Tab) |
| Esquive / roulade | Alt gauche | Alt gauche | Sans effet en escalade |
| Verrouiller la cible | Clic molette | Clic molette | |
| Sens du chasseur | V | V | Révèle les prises (traces et ennemis : plus tard) |
| Carte / Journal / Inventaire | M / J / I | M / J / I | Écrans : bloc H |
| Pause | Échap | Échap | ⚠️ En Play dans l'éditeur, Échap arrête la partie |

## Manette (Xbox / PlayStation)

| Action | Xbox | PlayStation |
|---|---|---|
| Se déplacer / caméra | Stick gauche / stick droit | Stick gauche / stick droit |
| Courir | L3 (LS, bascule) | L3 |
| Sauter, grimper | A | Croix |
| Esquive au sol / **lâcher en escalade** | B | Rond |
| Attaque légère | X | Carré |
| Attaque lourde | Y | Triangle |
| Parer | RB | R1 |
| Verrouiller la cible | LB | L1 |
| Sens du chasseur | R3 (RS) | R3 |
| Interagir | Croix directionnelle haut | Croix directionnelle haut |
| Pause | Menu | Options |
| Carte | Vue | Pavé tactile |

S'accroupir, journal et inventaire n'ont pas de bouton manette dédié (accès par la pause / la carte en bloc H).

## Règles

1. **Remappage** : chaque touche porte un nom stable (ex. `Sauter`, `Sauter_Manette`, `Parer_AZERTY`) déclaré aux réglages Enhanced Input du joueur. Le menu Options (bloc H) n'a qu'à lister ces noms et appeler le remappage ; aucune donnée de touche n'est en dur dans le C++.
2. **AZERTY / QWERTY** : deux contextes séparés (`IMC_Clavier_AZERTY`, `IMC_Clavier_QWERTY`), jamais actifs ensemble, pour éviter qu'une touche ait deux rôles (le A d'AZERTY ne déplace pas à gauche). Détection : la lettre produite par la touche physique à droite de Tab.
3. **Prompts à l'écran** : « [Espace] Grimper » au clavier, « [A] Grimper » ou « [Croix] Grimper » à la manette, d'après le **dernier périphérique utilisé** et la touche réellement mappée (remaps compris). Prompts actuels : Grimper (prise à portée), Sauter / Se hisser et Lâcher (suspendue).
4. **Icônes** : la DT `DT_LibellesTouches` a des colonnes d'icônes (clavier, Xbox, PlayStation). Tant qu'elles sont vides, le libellé est affiché dans un cadre. À remplir en bloc H avec un pack d'icônes libre de droits (à choisir et vérifier : licence, style compatible avec la DA).

## Réglages joueur (en attendant le menu Options du bloc H)

Sauvegardés dans `GameUserSettings.ini`. Commandes console (touche **²** ou **F2**) :

| Commande | Effet |
|---|---|
| `MansaClavier Auto` / `AZERTY` / `QWERTY` | Disposition du clavier |
| `MansaManette Xbox` / `PlayStation` | Icônes manette |
| `MansaImmersion 1` / `0` | Mode immersion (masque boussole et marqueurs, quand ils existeront) |
| `MansaControles` | Logue la disposition, la touche de chaque action (clavier et manette) et les prompts affichés |
