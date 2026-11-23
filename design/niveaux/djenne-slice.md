# Niveau : Djenné, vertical slice (Phase 1) — fiche greybox chiffrée

> Rédigé par `directeur-creatif`, 2026-11-23. Destination finale : `design/niveaux/djenne-slice.md`.
> Source : GDD v1.2 (§3, §4.4, §5.6, §7, §12 Q12), `design/lore/djenne-1324.md` (§5, §6, §6 bis).
> Usage : cette fiche sert d'entrée à un script Python éditeur Unreal qui génère le greybox (cubes, cylindres, cônes).
> Toutes les valeurs sont des **valeurs de départ** à régler en greybox.

---

## 0. Décisions prises dans cette fiche (à reporter dans le GDD)

1. **Q12 tranchée** : maison R+1 (acrotère à 7,5 m, pinacles à 8,5-9,1 m). La verticalité vient d'une annexe basse (D1) et du bord de butte (D3 au-dessus du fleuve). Paragraphe prêt à copier en §8.
2. **Le plan est inversé par rapport au croquis du GDD §7.2** : la **mosquée est à l'EST de la place**, la **maison du marchand au NORD-OUEST**. Pourquoi : le soleil de l'aube vient de l'est-nord-est. Avec la mosquée à l'est, son ombre tombe vers l'ouest-sud-ouest, donc sur la place et pas sur la façade escaladée. Les façades escaladées (est et nord) sont alors éclairées, et la mosquée est à contre-jour depuis l'entrée de la place (plan « wow ») et depuis la terrasse.
3. **Sections d'escalade redéfinies** (elles remplacent « D1 5 m / D2 7 m / D3 8 m ») :
   - D1 : face est de l'annexe, 0 → 3,6 m ;
   - D2 : traversée de la façade est, 9,7 m de long, au-dessus de la descente vers le quai (jusqu'à 6,2 m de vide) ;
   - D3 : face nord, côté fleuve, avec 12 m de vide et 2 options.
4. **CP2** n'est plus « sur la corniche intermédiaire » mais **debout sur le palier du contrefort d'angle nord-est** (Z 300). On réapparaît debout, ce qui est plus simple à coder qu'une réapparition suspendu à une prise.

---

## 1. Repère et conventions pour le script

| Convention | Valeur |
|---|---|
| Unités | centimètres (unités Unreal) |
| Axes | **X = est**, **Y = nord** (vers le fleuve), **Z = haut** |
| Origine | (0,0,0) = spawn, au bas de la ruelle (zone A), au sol |
| Sol de la ville (sommet de la butte) | Z = 0 |
| Berge (pied de la butte, côté nord) | Z = −450 |
| Surface du fleuve | Z = −500 |
| `centre` | centre de la **boîte englobante alignée sur les axes du monde** |
| `dims` | taille totale (x, y, z) de cette boîte englobante, en cm |
| Boîte | `/Engine/BasicShapes/Cube` (100³, pivot au centre). Échelle = dims / 100 |
| Cylindre | `/Engine/BasicShapes/Cylinder` (Ø100 × h100, pivot au centre). Colonne `axe` : Z par défaut. **Axe X** : rotation (Pitch 90, Yaw 0, Roll 0), échelle locale (Ø/100, Ø/100, L/100). **Axe Y** : rotation (Pitch 0, Yaw 0, Roll 90), même échelle locale |
| Cône | `/Engine/BasicShapes/Cone` (Ø100 × h100, pointe vers +Z). À vérifier : le pivot est au centre de la boîte englobante. Sinon, décaler de +h/2 |
| Rotation | `—` = aucune. Sinon, Yaw en degrés (0 = +X, 90 = +Y) |
| Rôles | `sol`, `mur` (îlot de maisons plein), `muret`, `obstacle`, `décor`, `prise`, `volume` |
| Dossiers Outliner | `Greybox/A_Ruelle`, `B_Griot`, `C_Place`, `C_Mosquee`, `D_Maison`, `D_Prises`, `E_Terrasse`, `F_Fleuve`, `Gameplay` |
| Tags acteur | `ID` de la ligne + rôle (ex. `D2-05`, `Prise:BancoFragile`) |

**Note sur le repère** : Unreal est un repère « main gauche ». Avec X = est et Y = nord, la vue de dessus de l'éditeur est **en miroir** de la boussole réelle. C'est sans conséquence pour la slice, puisque toutes les directions de cette fiche (soleil, caméras) sont données en **composantes X/Y** ou en **Yaw Unreal**, et non en points cardinaux réels.

**Les murs de la ruelle sont des îlots pleins** : chaque îlot est une boîte qui remplit l'espace entre deux ruelles, ce qui évite les murs fins et le vide derrière. Hauteurs : 350 (maison de plain-pied) ou 600 (maison à étage).

---

## 2. Parcours, rythme et objectifs (résumé)

| # | Zone | Durée cible | Tension | Objectif joueur | Point d'intérêt |
|---|---|---|---|---|---|
| 1 | A. Ruelle (≈ 60 m de chemin) | 2:00 | Repos | Avancer, sauter le muret, se faufiler dans le passage étroit | Rigole, jarres, passants, appel à la prière |
| 2 | B. Coin du griot | 2:30 | Repos / curiosité | Parler au griot [G] | Acacia, griot qui accorde son ngoni |
| 3 | C. Place + mosquée | 1:00 | Montée | Traverser, vivre le plan « wow », refus [R] | Mosquée à contre-jour, étals, [J] |
| 4 | D1 annexe | 0:35 | Montée douce | Apprendre : grimper, saut latéral, mantle | Toron, gouttière |
| 5 | Toit de l'annexe | 0:10 | Mini-repos | Respirer, voir la mosquée au sud-est | Vue sur la place |
| 6 | D2 façade est | 0:50 | Montée | Traverser au-dessus du vide, survivre à la prise fragile | Descente vers le quai sous les pieds |
| 7 | CP2 palier du contrefort | 0:10 | Repos haut | Regarder (indice caméra vers la tour), choisir sa voie | Premier regard sur le fleuve |
| 8 | D3 face nord | 0:35 | Pic de vertige | Option A (corniche, sûre) ou B (saut entre contreforts) | 12 m de vide, berge, pirogues |
| 9 | E. Terrasse | 2:30 | Pic | Combat contre 2 ennemis, au rythme du djembé | Mosquée en fond, [J] en contrebas |
| 10 | Fin | 1:30 | Retombée | Indice, choix, plan final | Fleuve + tour à contre-jour |

Escalade D1 à D3 : **environ 2:50 à 3:10** pour un premier essai, chute et rattrapage compris. Cela fait environ 25 m de mur, 23 prises et 2 checkpoints.

---

## 3. Éléments du greybox, zone par zone

### 3.0 Sols, butte, fleuve (dossier `F_Fleuve` et sols)

| ID | Nom | Forme | Centre (x,y,z) | Dims (x,y,z) | Axe / Rot. | Rôle |
|---|---|---|---|---|---|---|
| G1 | Sol ville (butte) | boîte | (400, 4500, −25) | (8800, 11000, 50) | — | sol. X[−4000,4800], Y[−1000,10000] |
| G2 | Masse de la butte ouest (sous la maison) | boîte | (−2300, 10300, −225) | (3400, 600, 450) | — | sol. Sa face nord, Y=10600, est le **soutènement** |
| G3 | Masse de la butte est | boîte | (3050, 10300, −225) | (3500, 600, 450) | — | sol |
| F1 | Berge | boîte | (400, 12300, −475) | (8800, 3400, 50) | — | sol. Sommet à Z −450 |
| F2 | Fleuve (Bani) | boîte | (500, 24000, −520) | (17000, 20000, 40) | — | décor. Surface à Z −500, matériau eau |
| F3 | Rive opposée (plaine inondable, rizières) | boîte | (500, 37000, −490) | (17000, 6000, 60) | — | décor |
| F4 | Pirogue 1 (tirée sur la berge) | boîte | (−1800, 13600, −425) | (900, 110, 50) | Yaw 8 | décor |
| F5 | Pirogue 2 | boîte | (−300, 13800, −425) | (900, 110, 50) | Yaw −5 | décor |
| F6 | Pirogue 3 (sur l'eau) | boîte | (1500, 17000, −480) | (1000, 120, 40) | Yaw 20 | décor |
| F7 | Rônier 1, tronc | cylindre | (−3000, 12500, −20) | (40, 40, 860) | Z | décor |
| F8 | Rônier 1, couronne | cylindre | (−3000, 12500, 460) | (500, 500, 100) | Z | décor |
| F9 | Rônier 2, tronc | cylindre | (2200, 12000, −20) | (40, 40, 860) | Z | décor |
| F10 | Rônier 2, couronne | cylindre | (2200, 12000, 460) | (500, 500, 100) | Z | décor |

### 3.1 Zone A : ruelle sinueuse (dossier `A_Ruelle`)

Espace libre (marchable, Z=0) : on le donne pour vérification. Les îlots en sont le complément.

| Tronçon | X | Y | Largeur libre |
|---|---|---|---|
| F1 (départ, muret) | [−150, 150] | [−300, 1900] | 300 |
| F2 (coude vers l'est) | [−150, 550] | [1600, 1900] | 300 |
| F3 | [250, 550] | [1600, 2900] | 300 |
| F4 **passage étroit** | [330, 470] | [2900, 3500] | **140** |
| F5 (coude vers l'ouest) | [−150, 470] | [3500, 3800] | 300 |
| F6 | [−150, 200] | [3500, 4800] | 350 |

Longueur du chemin : environ 55 à 60 m.

| ID | Nom | Forme | Centre (x,y,z) | Dims (x,y,z) | Axe / Rot. | Rôle |
|---|---|---|---|---|---|---|
| A01 | Îlot ouest, sud | boîte | (−675, 550, 175) | (1050, 2100, 350) | — | mur |
| A02 | Îlot ouest, milieu (à étage) | boîte | (−675, 2400, 300) | (1050, 1600, 600) | — | mur |
| A03 | Îlot ouest, nord | boîte | (−675, 4000, 175) | (1050, 1600, 350) | — | mur |
| A04 | Fond de ruelle derrière le spawn | boîte | (0, −400, 175) | (300, 200, 350) | — | mur |
| A05 | Îlot est, sud | boîte | (675, 550, 175) | (1050, 2100, 350) | — | mur |
| A06 | Îlot est, coude (à étage) | boîte | (875, 2250, 300) | (650, 1300, 600) | — | mur |
| A07 | Îlot central, sud | boîte | (50, 2400, 175) | (400, 1000, 350) | — | mur |
| A08 | Îlot central, nord (bord ouest du passage étroit) | boîte | (90, 3200, 175) | (480, 600, 350) | — | mur |
| A09 | Îlot est, bord est du passage étroit | boîte | (835, 3200, 175) | (730, 600, 350) | — | mur |
| A10 | Îlot est, coude nord | boîte | (835, 3650, 175) | (730, 300, 350) | — | mur |
| A11 | Îlot est, nord (à étage) | boîte | (700, 4300, 300) | (1000, 1000, 600) | — | mur |
| A12 | **Muret** (tutoriel enjambement) | boîte | (0, 1100, 35) | (300, 40, **70**) | — | muret (≤ 90 ✓) |
| A13 | Rigole centrale | boîte | (0, 800, 1) | (30, 2000, 2) | — | décor (sombre, sans collision) |
| A14 | Jarre derrière le spawn | cylindre | (110, −230, 40) | (60, 60, 80) | Z | décor |
| A15 | Jarre dans le coin du coude F5 | cylindre | (−115, 3545, 40) | (60, 60, 80) | Z | obstacle (laisse 285 libres) |
| A16 | Porte basse en bois | boîte | (551, 2300, 100) | (6, 120, 200) | — | décor |
| A17 | Gouttière (au-dessus de F3) | cylindre | (520, 2600, 570) | (60, 20, 20) | X | décor |
| A18 | Gouttière (au-dessus de F6) | cylindre | (230, 4300, 570) | (60, 20, 20) | X | décor |

### 3.2 Zone B : coin du griot, placette de 10 × 10 m (dossier `B_Griot`)

Espace libre : X[−500, 500], Y[4800, 5800]. Sortie vers la place par le passage BC, X[−150, 250] et Y[5800, 6200] (400 de large).

| ID | Nom | Forme | Centre (x,y,z) | Dims (x,y,z) | Axe / Rot. | Rôle |
|---|---|---|---|---|---|---|
| B01 | Îlot ouest | boîte | (−850, 5300, 175) | (700, 1000, 350) | — | mur |
| B02 | Îlot est (à étage) | boîte | (850, 5300, 300) | (700, 1000, 600) | — | mur |
| B03 | Îlot nord-ouest (bord du passage BC) | boîte | (−675, 6000, 175) | (1050, 400, 350) | — | mur |
| B04 | Îlot nord-est (bord du passage BC) | boîte | (725, 6000, 175) | (950, 400, 350) | — | mur |
| B05 | Banc de banco du griot | boîte | (−475, 5450, 22.5) | (50, 180, 45) | — | décor |
| B06 | Acacia, tronc | cylindre | (300, 5550, 150) | (30, 30, 300) | Z | décor |
| B07 | Acacia, houppier | cylindre | (300, 5550, 330) | (400, 400, 50) | Z | décor (sans collision) |
| B08 | Mortier à piler | cylindre | (400, 5000, 30) | (50, 50, 60) | Z | décor |

### 3.3 Zone C : place du marché, 40 × 30 m (dossier `C_Place`)

Place nominale X[−2800, 1200], Y[6200, 9300]. Sa bande est (X > 700) est le pied du tertre de la mosquée. **Recoin** au pied de l'annexe : X[0, 1300], Y[9300, 9860]. **Couloir du chemin critique à garder libre** : X[−200, 700] de Y 6200 à 9300, puis le recoin.

| ID | Nom | Forme | Centre (x,y,z) | Dims (x,y,z) | Axe / Rot. | Rôle |
|---|---|---|---|---|---|---|
| C01 | Maisons sud-ouest | boîte | (−2400, 5800, 175) | (2400, 800, 350) | — | mur |
| C02 | Maisons sud-est | boîte | (3000, 5800, 175) | (3600, 800, 350) | — | mur |
| C03 | Maisons ouest, sud | boîte | (−3200, 7000, 175) | (800, 1600, 350) | — | mur |
| C04 | Maisons ouest, nord (à étage) | boîte | (−3200, 8550, 300) | (800, 1500, 600) | — | mur |
| C05 | Voisin mitoyen ouest du marchand (bord de butte) | boîte | (−2600, 9950, 175) | (800, 1300, 350) | — | mur |
| C06 | Maisons à l'est de la mosquée | boîte | (4450, 7750, 175) | (700, 3100, 350) | — | mur |
| C07 | Maisons au nord de la mosquée (bord est du recoin) | boîte | (1850, 9950, 175) | (1100, 1300, 350) | — | mur |
| C08 | Maisons au nord-est de la mosquée (à étage) | boîte | (3600, 9950, 300) | (2400, 1300, 600) | — | mur |
| C09 | Étal 1, table | boîte | (−2300, 6800, 40) | (220, 120, 80) | — | obstacle |
| C10 | Étal 1, auvent | boîte | (−2300, 6800, 224) | (260, 160, 8) | — | décor |
| C11 | Étal 2, table | boîte | (−2300, 7600, 40) | (220, 120, 80) | — | obstacle |
| C12 | Étal 2, auvent | boîte | (−2300, 7600, 224) | (260, 160, 8) | — | décor |
| C13 | Étal 3, table | boîte | (−1600, 7100, 40) | (220, 120, 80) | — | obstacle |
| C14 | Étal 3, auvent | boîte | (−1600, 7100, 224) | (260, 160, 8) | — | décor |
| C15 | Étal 4, table | boîte | (−1000, 6800, 40) | (220, 120, 80) | — | obstacle |
| C16 | Étal 4, auvent | boîte | (−1000, 6800, 224) | (260, 160, 8) | — | décor |
| C17 | Étal 5, table | boîte | (−2350, 8600, 40) | (220, 120, 80) | — | obstacle |
| C18 | Étal 5, auvent | boîte | (−2350, 8600, 224) | (260, 160, 8) | — | décor |
| C19 | Étal 6, table | boîte | (−1100, 7900, 40) | (220, 120, 80) | — | obstacle |
| C20 | Étal 6, auvent | boîte | (−1100, 7900, 224) | (260, 160, 8) | — | décor |
| C21 | Puits (margelle) | cylindre | (−600, 7400, 40) | (160, 160, 80) | Z | obstacle |
| C22 | Tabouret du joueur de djembé | cylindre | (−1800, 8060, 20) | (40, 40, 40) | Z | décor |
| C23 | Ballots (déchargement qui bloque la descente) | boîte | (500, 9925, 65) | (400, 130, 130) | — | obstacle (doublé par `BV_Descente`) |
| C24 | Pirogue retournée (déchargement) | boîte | (1000, 9930, 40) | (700, 120, 80) | — | obstacle |

Les poteaux des auvents sont optionnels en greybox.

### 3.4 Mosquée, décor non escaladable (dossier `C_Mosquee`)

🔶 inspiré de (assumé), cf. lore §5 : silhouette « médiévale imaginée », plus basse et plus massive que celle de 1907. Une seule tour, pyramidale, au nord-est, côté qibla. Contreforts épais coiffés de pinacles arrondis, sans œufs d'autruche. Tertre surélevé.
**AUCUN `BP_PriseEscalade`, aucun prompt « Grimper ».** Les toron décoratifs ont un profil de collision sans tag d'escalade. Le tertre (200 cm) dépasse le mantle automatique (≤ 180) : le joueur ne peut pas y monter.

| ID | Nom | Forme | Centre (x,y,z) | Dims (x,y,z) | Axe / Rot. | Rôle |
|---|---|---|---|---|---|---|
| M01 | Tertre | boîte | (2400, 7750, 100) | (3400, 3100, 200) | — | décor (bloquant) |
| M02 | Corps de la mosquée (salle hypostyle) | boîte | (2400, 7750, 625) | (2800, 2500, 850) | — | décor. Sommet à Z 1050 |
| M03 | Tour, étage 1 | boîte | (3400, 8600, 775) | (800, 800, 1150) | — | décor. Z[200, 1350] |
| M04 | Tour, étage 2 | boîte | (3400, 8600, 1625) | (600, 600, 550) | — | décor. Z[1350, 1900] |
| M05 | Tour, étage 3 | boîte | (3400, 8600, 2025) | (400, 400, 250) | — | décor. Z[1900, 2150] |
| M06 | Tour, pinacle sommital | cône | (3400, 8600, 2200) | (200, 200, 100) | Z | décor. **Sommet à 22,5 m (20,5 m au-dessus du tertre)** |
| M07 | Contrefort ouest 1 | boîte | (925, 6800, 675) | (150, 150, 950) | — | décor |
| M08 | Contrefort ouest 2 | boîte | (925, 7300, 675) | (150, 150, 950) | — | décor |
| M09 | Contrefort ouest 3 | boîte | (925, 7800, 675) | (150, 150, 950) | — | décor |
| M10 | Contrefort ouest 4 | boîte | (925, 8300, 675) | (150, 150, 950) | — | décor |
| M11 | Contrefort ouest 5 | boîte | (925, 8800, 675) | (150, 150, 950) | — | décor |
| M12-M16 | Pinacles des contreforts ouest | cône ×5 | (925, Y, 1200), Y ∈ {6800, 7300, 7800, 8300, 8800} | (150, 150, 100) | Z | décor |
| M17 | Contrefort nord 1 | boîte | (1400, 9075, 675) | (150, 150, 950) | — | décor |
| M18 | Contrefort nord 2 | boîte | (2000, 9075, 675) | (150, 150, 950) | — | décor |
| M19 | Contrefort nord 3 | boîte | (2600, 9075, 675) | (150, 150, 950) | — | décor |
| M20-M22 | Pinacles des contreforts nord | cône ×3 | (X, 9075, 1200), X ∈ {1400, 2000, 2600} | (150, 150, 100) | Z | décor |
| M23-M31 | **Toron décoratifs** (le « chemin le plus court » vu depuis [R]) | cylindre ×9 | (975, Y, Z), Y ∈ {8450, 8550, 8650}, Z ∈ {500, 650, 800} | (50, 12, 12) | X | décor, **PAS une prise** |
| M32 | Porte de la mosquée (face ouest) | boîte | (998, 7550, 330) | (6, 160, 260) | — | décor |

### 3.5 Zone D : maison du marchand et annexe (dossier `D_Maison`)

Maison : X[−2200, −600], Y[9300, 10600]. Rez de 350 + étage de 300, **terrasse à Z 650**, **acrotère à Z 750**, pinacles jusqu'à 850-910.
Annexe (magasin) mitoyenne à l'est : X[−600, 0], Y[9300, 10000], toit à Z 300, parapet de 60.
**Façade est au-dessus de Z 600 : enduit refait cette année, lisse, sans aucune prise.** Cela justifie la traversée de D2 et le passage par la face nord ; c'est aussi un clin d'œil au pilier « saisons », pour Phase 2.

| ID | Nom | Forme | Centre (x,y,z) | Dims (x,y,z) | Axe / Rot. | Rôle |
|---|---|---|---|---|---|---|
| D01 | Corps de la maison (R+1) | boîte | (−1400, 9950, 325) | (1600, 1300, 650) | — | mur. Son sommet est le sol de la terrasse |
| D02 | Acrotère sud | boîte | (−1400, 9325, 700) | (1600, 50, 100) | — | muret (parapet) |
| D03 | Acrotère nord | boîte | (−1400, 10575, 700) | (1600, 50, 100) | — | muret (parapet) |
| D04 | Acrotère ouest | boîte | (−2175, 9950, 700) | (50, 1200, 100) | — | muret (parapet) |
| D05 | Acrotère est | boîte | (−625, 9950, 700) | (50, 1200, 100) | — | muret (parapet) |
| D06 | Annexe, corps | boîte | (−300, 9650, 150) | (600, 700, 300) | — | mur. Son toit, à Z 300, est marchable |
| D07 | Annexe, parapet sud | boîte | (−300, 9320, 330) | (600, 40, 60) | — | muret |
| D08 | Annexe, parapet est | boîte | (−20, 9650, 330) | (40, 620, 60) | — | muret. Son bord supérieur est D1-05 |
| D09 | Annexe, parapet nord | boîte | (−300, 9980, 330) | (600, 40, 60) | — | muret |
| D10 | Annexe, pinacle 1 (bloque le raccourci) | cône | (−20, 9420, 385) | (40, 40, 50) | Z | décor |
| D11 | Annexe, pinacle 2 | cône | (−20, 9580, 385) | (40, 40, 50) | Z | décor |
| D12 | **Contrefort d'angle nord-est** (palier CP2) | boîte | (−500, 10635, −75) | (200, 330, 750) | — | mur. Z[−450, 300], sommet marchable de 200 × 330 |
| D13 | **Contrefort nord N1** | boîte | (−1060, 10660, 35) | (120, 120, 970) | — | mur. X[−1120, −1000], Z[−450, 520] |
| D14 | **Contrefort nord N2** | boîte | (−1380, 10660, 35) | (120, 120, 970) | — | mur. X[−1440, −1320]. **Écart N1-N2 = 200** |
| D15 | Pilastre sud gauche | boîte | (−1600, 9287.5, 425) | (50, 25, 850) | — | décor |
| D16 | Pilastre sud droit | boîte | (−1200, 9287.5, 425) | (50, 25, 850) | — | décor |
| D17 | Pinacle sur le pilastre gauche | cône | (−1600, 9287.5, 885) | (50, 50, 70) | Z | décor |
| D18 | Pinacle sur le pilastre droit | cône | (−1200, 9287.5, 885) | (50, 50, 70) | Z | décor |
| D19 | Porte basse en bois épais | boîte | (−1400, 9298, 95) | (120, 4, 190) | — | décor |
| D20 | Petite fenêtre haute sud 1 | boîte | (−1800, 9298, 500) | (50, 4, 40) | — | décor (sombre) |
| D21 | Petite fenêtre haute sud 2 | boîte | (−1000, 9298, 500) | (50, 4, 40) | — | décor (sombre) |
| D22 | Fenêtre haute est (son rebord est D2-02) | boîte | (−598, 9620, 570) | (4, 60, 45) | — | décor (sombre) |
| D23 | Enduit frais, haut de la façade est | boîte | (−599, 9950, 675) | (2, 1300, 150) | — | décor (teinte « enduit frais ») |
| D24-D29 | Pinacles serrés de l'acrotère nord (au-dessus de CorA : on ne peut pas s'y hisser) | cône ×6 | (X, 10575, 780), X ∈ {−700, −820, −940, −1060, −1180, −1300} | (40, 40, 60) | Z | décor |
| D30 | Pinacle d'angle sud-ouest | cône | (−2175, 9325, 800) | (60, 60, 100) | Z | décor |
| D31 | Pinacle d'angle sud-est | cône | (−625, 9325, 800) | (60, 60, 100) | Z | décor |
| D32 | Pinacle d'angle nord-ouest | cône | (−2175, 10575, 800) | (60, 60, 100) | Z | décor |
| D33 | Pinacle d'angle nord-est | cône | (−625, 10575, 800) | (60, 60, 100) | Z | décor |
| D34 | Gouttière sud 1 | cylindre | (−2000, 9265, 630) | (20, 70, 20) | Y | décor |
| D35 | Gouttière sud 2 | cylindre | (−800, 9265, 630) | (20, 70, 20) | Y | décor |
| D36 | Gouttière nord (décor, loin de CorB) | cylindre | (−1900, 10635, 630) | (20, 70, 20) | Y | décor |
| D37 | Marche 1 de la descente vers le quai | boîte | (350, 10116.5, −262.5) | (1900, 233, 375) | — | sol (sommet à Z −75) |
| D38 | Marche 2 | boîte | (350, 10350, −300) | (1900, 234, 300) | — | sol (sommet à Z −150) |
| D39 | Marche 3 | boîte | (450, 10583.5, −337.5) | (1700, 233, 225) | — | sol (sommet à Z −225) |
| D40 | Marche 4 | boîte | (450, 10816.5, −375) | (1700, 233, 150) | — | sol (sommet à Z −300) |
| D41 | Marche 5 (rejoint la berge) | boîte | (450, 11050, −412.5) | (1700, 234, 75) | — | sol (sommet à Z −375) |

La descente vers le quai est **hors du chemin du joueur** : fermée par C23, C24 et `BV_Descente`. Elle sert de « vide » sous D2. Si le joueur y tombe, le volume de rattrapage le ramène au checkpoint.

### 3.6 Zone E : terrasse du marchand, arène (dossier `E_Terrasse`)

Intérieur libre : **X[−2150, −650] × Y[9350, 10550] = 1500 × 1200** ✓. Sol à Z 650, parapet de **100 cm** (D02 à D05) ✓.

| ID | Nom | Forme | Centre (x,y,z) | Dims (x,y,z) | Axe / Rot. | Rôle |
|---|---|---|---|---|---|---|
| E01 | Jarres, groupe 1 (grande) | cylindre | (−1700, 9550, 695) | (80, 80, 90) | Z | obstacle bas |
| E02 | Jarres, groupe 1 (petite) | cylindre | (−1790, 9500, 690) | (70, 70, 80) | Z | obstacle bas |
| E03 | Bois empilé | boîte | (−2000, 10150, 680) | (70, 220, 60) | — | obstacle bas |
| E04 | Natte roulée | cylindre | (−1350, 9500, 672.5) | (180, 45, 45) | X | obstacle bas |
| E05 | Jarre isolée | cylindre | (−750, 10400, 695) | (80, 80, 90) | Z | obstacle bas |

Aucun obstacle ne dépasse 90 cm. Le point d'arrivée du mantle (−1510, 10450) et les positions S1 et S2 restent dégagés (≥ 250 cm).

---

## 4. Prises d'escalade D1-D3 (dossier `D_Prises`)

Le script fait apparaître pour chaque ligne la géométrie indiquée et un `BP_PriseEscalade` au même centre, avec `PriseType`. Si le BP n'existe pas encore, il met un tag `Prise:<Type>`.
`Normale` = direction vers l'extérieur du mur. Yaw de l'acteur : +X → 0, +Y → 90, −Y → −90.
Toron : Ø 12, saillie de 50. Gouttière (type `Toron`) : Ø 20, saillie de 60. `BancoRelief` / `BancoFragile` : petit relief de banco. `Corniche` : marqueur fin le long d'une arête.

### D1 : face est de l'annexe (X = 0), tutoriel, 0 → 3,6 m

| ID | Type | Centre (x,y,z) | Géométrie (forme, dims, axe) | Normale | Écart avec la prise précédente (latéral / vertical) | Note |
|---|---|---|---|---|---|---|
| D1-01 | Toron | (25, 9420, 170) | cyl. (50, 12, 12), X | +X | depuis le sol : saisie à 170 | Prompt « Grimper » |
| D1-02 | Toron | (25, 9420, 280) | cyl. (50, 12, 12), X | +X | 0 / +110 | Apprend la montée |
| D1-03 | Toron | (25, 9600, 250) | cyl. (50, 12, 12), X | +X | 180 / −30 | **Apprend le saut latéral** |
| D1-04 | Toron (gouttière) | (30, 9780, 290) | cyl. (60, 20, 20), X | +X | 180 / +40 | Traînée d'érosion sombre sous la gouttière |
| D1-05 | Corniche | (−5, 9830, 355) | boîte (10, 260, 10) | +X | 0 / +70 | Segment Y[9700, 9960] seulement (pinacles D10 et D11 ailleurs). **Mantle** sur le toit de l'annexe |

Contrôle : de D1-02 à D1-05, il y a 280 cm en latéral (> 200). Le saut latéral est donc obligatoire ✓.

### D2 : façade est de la maison (X = −600), traversée vers le nord, au-dessus de l'annexe puis du vide

Départ : debout sur le toit de l'annexe (Z 300).

| ID | Type | Centre (x,y,z) | Géométrie | Normale | Écart (lat. / vert.) | Note |
|---|---|---|---|---|---|---|
| D2-01 | Toron | (−575, 9450, 480) | cyl. (50, 12, 12), X | +X | depuis le toit : saisie à 180 | |
| D2-02 | BancoRelief | (−592, 9620, 520) | boîte (15, 40, 10) | +X | 170 / +40 | Rebord de la fenêtre D22 |
| D2-03 | Toron | (−575, 9800, 520) | cyl. (50, 12, 12), X | +X | 180 / 0 | |
| D2-04 | BancoRelief | (−592, 9980, 560) | boîte (15, 40, 10) | +X | 180 / +40 | Assez haut pour que le corps passe au-dessus du parapet nord de l'annexe (Z 360). **Début du vide** |
| D2-05 | **BancoFragile** | (−592, 10150, 470) | boîte (15, 40, 12), banco fissuré | +X | 170 / −90 | **La seule de la slice.** S'effrite après 1,5 s. 5,5 m de vide dessous |
| D2-06 | Toron (**secours**) | (−575, 10300, 470) | cyl. (50, 12, 12), X | +X | **150** / 0 | **Toron de secours latéral à 150 cm** ✓ |
| D2-07 | Toron | (−575, 10420, 380) | cyl. (50, 12, 12), X | +X | 120 / −90 | |
| D2-08 | Corniche | (−500, 10465, 295) | boîte (200, 10, 10) | −Y | 50 / −80 | Arête sud du sommet du contrefort D12, X[−600, −400]. Transition en coin intérieur, puis **mantle** sur le palier → **CP2** |

Contrôles :
- De D2-04 à D2-06, il y a 320 cm (> 200) : la prise fragile est **obligatoire**, on ne peut pas la contourner ✓.
- Au-dessus de Z 600, l'enduit est frais : aucune prise ✓.
- Si le système ne gère pas le coin intérieur de D2-08, faire un mantle scripté depuis D2-07 vers (−500, 10600, 300).

### D3 : face nord, côté fleuve (Y = 10600), 12 m de vide jusqu'à la berge

Départ : debout sur le palier CP2 (Z 300), bord ouest à X = −600.

| ID | Type | Centre (x,y,z) | Géométrie | Normale | Écart (lat. / vert.) | Note |
|---|---|---|---|---|---|---|
| D3-01 | Toron | (−710, 10625, 470) | cyl. (12, 50, 12), Y | +Y | depuis le palier : 110 / saisie à 170 | Saut vers la prise |
| D3-02 | Toron | (−870, 10625, 520) | cyl. (12, 50, 12), Y | +Y | 160 / +50 | **Embranchement** |
| **Option A, sûre et lente** | | | | | | |
| D3-A1 | Toron (gouttière) | (−870, 10630, 640) | cyl. (20, 60, 20), Y | +Y | 0 / +120 | Colonne toron + gouttière |
| D3-A2 | Corniche « CorA » | (−1010, 10605, 745) | boîte (600, 10, 10) | +Y | 0 / +110 | Segment X[−1310, −710]. **Déplacement latéral seulement, mantle interdit** (`bMantleAutorise = false`, pinacles D24-D29). Le corps suspendu (bas à Z ≈ 570) passe au-dessus de N1 et N2 (sommets à 520) ✓ |
| **Option B, spectaculaire et rapide** | | | | | | |
| D3-B1 | BancoRelief | (−995, 10725, 450) | boîte (20, 20, 15) | +Y | 160 / −70 | Arête est de N1 |
| D3-B2 | BancoRelief | (−1125, 10725, 450) | boîte (20, 20, 15) | +Y | 130 / 0 | Arête ouest de N1 |
| D3-B3 | BancoRelief | (−1315, 10725, 470) | boîte (20, 20, 15) | +Y | **190** / +20 | **Saut entre contreforts** au-dessus de 9 m de vide |
| D3-B4 | BancoRelief | (−1445, 10725, 470) | boîte (20, 20, 15) | +Y | 130 / 0 | Arête ouest de N2 |
| D3-B5 | Toron | (−1530, 10625, 610) | cyl. (12, 50, 12), Y | +Y | 131 / +140 | |
| **Fin commune** | | | | | | |
| D3-CorB | Corniche « CorB » | (−1510, 10605, 745) | boîte (380, 10, 10) | +Y | depuis B5 : 0 / +140. Depuis CorA : continu | Segment X[−1700, −1320], acrotère usé, sans pinacles. **Mantle autorisé** → arrivée sur la terrasse en (−1510, 10450, 650) → combat |

Temps estimés :
- A : 3 saisies + environ 6 m de déplacement sur la corniche, à 60 cm/s, soit environ 14 s ;
- B : 6 saisies (environ 0,8 s chacune) dont 1 saut, soit environ 7 s.

B est plus rapide et plus spectaculaire, A est plus sûre. Les deux sont lisibles depuis CP2.

### Récapitulatif des règles

| Règle (GDD §4.4) | Vérification |
|---|---|
| Écart vertical ≤ 150 | Maximum 140 (D3-B4→B5, B5→CorB), hors saisies depuis le sol ou un palier (170-180, qui sont des saisies après saut) ✓ |
| Écart latéral ≤ 200 | Maximum 190 (D3-B2→B3), 180 dans D1 et D2 ✓ |
| 1 seule BancoFragile, avec un toron de secours latéral à 150 cm | D2-05 → D2-06 = 150 ✓ |
| 2 checkpoints | CP1 au pied de D1, CP2 sur le palier du contrefort ✓ (le checkpoint de combat du GDD §5.6 s'y ajoute) |
| Mosquée sans prise | 0 `BP_PriseEscalade` dans `C_Mosquee` ✓ |
| Total | 23 prises : D1 = 5, D2 = 8, D3 = 10 |

---

## 5. Points de gameplay (dossier `Gameplay`)

| ID | Type | Position (x,y,z) | Orientation / dims | Détail |
|---|---|---|---|---|
| `PS_Spawn` | PlayerStart | (0, 0, 0) | Yaw 90 (vers le nord) | Bas de la ruelle |
| `NPC_Griot` [G] | PNJ assis | (−450, 5450, 45) | Yaw 0 (vers l'est) | Sur le banc B05 |
| `TV_Griot` | volume sphère | (−200, 5450, 90) | rayon 300 | Prompt « Parler ». Le dialogue donne l'objectif (GDD §7.3) |
| `TV_PlanWow` | volume boîte | (50, 6300, 150) | (400, 300, 300) | Sortie du passage BC, déclenché une fois |
| `CAM_Wow` | caméra scriptée | (−300, 6600, 220) | Pitch +15,6, Yaw 28,4, FOV 60 | 4 s, skippable, travelling avant de 150 cm. Cadre la tour (3400, 8600, 1400). **Le soleil est 8° à gauche de la tour, à la même hauteur : contre-jour complet** |
| `TV_RefusMosquee` [R] | volume boîte | (−150, 8850, 150) | (1700, 900, 300) → X[−1000, 700], Y[8400, 9300] | Une seule fois. Tout chemin vers le recoin et D1 passe dedans. Cible du regard : toron M23-M31 (975, 8550, 650), puis la caméra pivote vers D1 (25, 9600, 250) |
| `CP1` | checkpoint | (350, 9650, 0) | Yaw 180 (face à D1) | Pied de l'annexe, dans le recoin |
| `CP2` | checkpoint | (−480, 10680, 300) | Yaw 180 (face à D3) | Palier du contrefort nord-est. À l'arrivée, **indice caméra de 2 s** vers la tour (Yaw −28), puis retour vers D3 |
| `KV_Rattrapage` | volume boîte | (−550, 10800, −260) | (3700, 1600, 480) → X[−2400, 1300], Y[10000, 11600], Z[−500, −20] | Fondu au noir de 0,5 s, retour au dernier checkpoint, réplique « Sira n'était pas un oiseau » |
| `BV_Descente` | BlockingVolume | (650, 9930, 250) | (1300, 140, 500) | Ferme la descente vers le quai |
| `TV_Combat` | volume boîte | (−1400, 9950, 750) | (1500, 1200, 200) | Déclenché au mantle sur CorB : combat, djembé, checkpoint de combat |
| `CP_Combat` | checkpoint | (−1510, 10400, 650) | Yaw −37 | Début du combat (GDD §5.6) |
| `BV_Terrasse_S/N/O/E` | BlockingVolume ×4 | au-dessus de D02 à D05, Z[750, 1150] | mêmes X et Y que les acrotères, dims z = 400 | Activés à l'arrivée sur la terrasse : impossible de sauter par-dessus le parapet |
| `S1` voleur agile | Ennemi | (−1050, 10050, 650) | Yaw 143 (vers le joueur) | Attaque en premier |
| `S2` brute | Ennemi | (−800, 9600, 650) | Yaw 132 | Observe, puis entre en jeu (GDD §5.6) |
| `CAM_CombatIntro` | caméra scriptée | (−1700, 10500, 900) | Pitch −10, Yaw −30, FOV 85 | 2 s : cadre S1, S2 et la mosquée en fond (la tour est à Yaw −20 vu d'ici) |
| `NPC_Djembe` [J] | PNJ joueur de djembé | (−1800, 8100, 0) | Yaw 90 (face à la maison) | Visible depuis la terrasse : la ligne de vue passe le parapet sud à Z ≈ 790 > 750 ✓. Hors de l'ombre de la mosquée |
| `CAM_Fin` | caméra scriptée | (−1800, 10450, 850) | Départ Pitch −5, Yaw 60 (fleuve, pirogues) → panoramique de 6 s vers Pitch +5, Yaw −12 (tour à contre-jour, soleil 32° à droite), FOV 75 | Plan final |
| `NPC_Passant_1..6` | passants (boucles) | (−1400, 7500), (−2000, 7200), (300, 7100), (−500, 8100), (−2600, 8200), (−800, 6500) | — | Z = 0 |
| `NPC_Passant_A1..A2` | passants de la ruelle | (400, 2600), (50, 4400) | — | On les contourne (tutoriel « se faufiler ») |
| `SND_Fleuve` | émetteur audio | (−500, 14000, −480) | — | Clapotis, piroguiers |

---

## 6. Lumière d'aube et palette

### Soleil (DirectionalLight « Atmosphere Sun Light »)
- **Direction d'où vient la lumière** : vecteur horizontal (X 0,940 ; Y 0,342), soit l'est-nord-est, élévation 16°.
- **Rotation Unreal de la lumière** (sens de propagation) : **Pitch −16°, Yaw −160° (= 200°), Roll 0**.
- Cohérence : à quelques degrés près, cela correspond au soleil environ 1 h après son lever, en mai (fin de saison sèche, avant le crépissage) à Djenné (latitude ≈ 13,9° N).
- Effets sur le décor :

| Surface | Éclairage | Rôle |
|---|---|---|
| Face est de l'annexe (D1) | Frontal, doré | Tutoriel bien lisible |
| Façade est (D2) | Frontal, doré ; l'enduit frais au-dessus de Z 600 est lisse et uniforme | Le contraste montre où sont les prises |
| Face nord (D3) | **Rasant** (environ 70° d'incidence) : les toron et les contreforts N1 et N2 projettent de longues ombres vers l'ouest | La lumière dessine le chemin |
| Façade sud et place | Façade dans l'ombre. L'ombre de la mosquée couvre le sud-est de la place et la zone [R] | Le refus se joue dans l'ombre de la mosquée |
| Mosquée vue de la place | Contre-jour complet | Plan « wow » |
| Mosquée vue de la terrasse | Contre-jour partiel : face ouest sombre, liseré doré sur la face nord, soleil dans le cadre | Combat et plan de fin |

- Contrôle des ombres : les ombres de la mosquée partent vers −Y à mesure que X diminue. Elles n'atteignent **jamais** Y ≥ 9300, donc ni D1, ni D2, ni D3, ni la terrasse ✓.
- Réglages indicatifs :
  - température 3500 K, intensité 6-10 lux (échelle par défaut d'UE5, à caler avec l'exposition) ;
  - SkyAtmosphere + SkyLight (temps réel, Lumen) ;
  - ExponentialHeightFog : densité 0,03, couleur de diffusion ocre (190, 140, 95) pour la poussière d'harmattan ;
  - Volumetric Fog activé (rayons autour de la tour).
- **Paramètres exposés** : `SoleilYaw`, `SoleilPitch`. Pour un contre-jour plus franc depuis la terrasse, passer le Yaw à −175° : le soleil se rapproche de l'axe de la tour, mais la face nord (D3) est alors moins rasante.

### Palette indicative (albédo sRGB)

| Élément | RGB | Hex |
|---|---|---|
| Banco clair (enduit sec, ancien) | (190, 146, 100) | #BE9264 |
| Banco ombré (zones érodées, humides, sous les gouttières) | (128, 92, 64) | #805C40 |
| Ombre perçue à l'aube (référence d'étalonnage, bleutée) | (92, 86, 98) | #5C5662 |
| Enduit frais (haut de la façade est) | (168, 118, 78) | #A8764E |
| Banco de la mosquée (plus ancien) | (170, 126, 88) | #AA7E58 |
| Bois des toron et gouttières (rônier) | (72, 50, 34) | #483222 |
| Sol de terre battue | (158, 120, 84) | #9E7854 |
| Berge humide | (96, 78, 60) | #604E3C |
| Eau du fleuve | (70, 92, 96) | #465C60 |
| Ciel à l'horizon, côté soleil | (255, 186, 120) | #FFBA78 |
| Ciel au zénith | (110, 140, 185) | #6E8CB9 |

Pour le greybox, prévoir 4 matériaux : banco clair (murs), bois sombre (prises), terre (sols), et un matériau translucide pour les volumes (visible en éditeur seulement). **Les prises sont lisibles par la forme et la couleur sombre, sans peinture jaune** (GDD §4.4).

---

## 7. Croquis

### 7.1 Vue de dessus (nord en haut, est à droite, échelle approximative)

```
 Y
34000 ~~~~~~~~~~~~~~~~~~~~~~~~ rive opposée (Z -490) ~~~~~~~~~~~~~~~~~~~~~~~~~~
14000 ~~~~~~~~~~~~~~~~~~~ FLEUVE / Bani (surface Z -500) ~~~~~~~~~~~~~~~~~~~~~~
         F4 pirogue     F5 pirogue                 (F6 pirogue sur l'eau, Y 17000)
      F7 rônier (-3000,12500)      BERGE Z -450       F9 rônier (2200,12000)
10600 ===soutènement / bord de butte=====+--marches D41..D39--+====================
      | C05  | N2(-1380) N1(-1060)  D12  |                    |  C07   |  C08      |
      | h350 |<-D3 face nord, côté fleuve| marches D38 D37    |  h350  |  h600     |
      |      |  MAISON DU MARCHAND  CP2  |  (descente fermée) |        |           |
      |      |  terrasse Z650, 15 x 12 m |                    |        |           |
10000 |      |   S1(-1050,10050)   +-----+ ==== BV_Descente ===        |           |
      |      |                     | AN- |  C23 ballots  C24  |        |           |
      |      |  S2(-800,9600)  D2->| NEXE|                    |        |           |
      |      |  X[-2200,-600]      | D1->|  CP1(350,9650)     |        |           |
 9300 +------+==[D15 porte D16]====+-----+--------------------+--tertre M01 (Z200)-+
 C04  :                              ^ [R] TV_RefusMosquee     |CN1  CN2  CN3 [TOUR]| C06
 h600 :  C17 étal 5                  | X[-1000,700]           |      M17-M19  (3400,|
      :              [J](-1800,8100) | Y[8400,9300]           |               8600) |
      :         C19 étal 6           |                        |   MOSQUÉE           |
 C03  :  C11 étal 2      C21 puits   |  couloir libre         |   corps Z200-1050   |
 h350 :         C13 étal 3           |  X[-200,700]           |CW  M07-M11          |
      :  C09 étal 1    C15 étal 4    |                        |   (face ouest X1000)|
 6200 +---------- C01 ----------+B03-+ BC +-B04-+---------- C02 ------------------+
  ^ CAM_Wow (-300,6600) regarde la tour, Yaw 28    ^ TV_PlanWow (50,6300)
                                | B. GRIOT [G](-450,5450)  acacia (300,5550) |
 4800                           +----+  F6  +-------------------------------+
                                     |  F6  |  A11 (h600)
 3800                                |  F5 coude --------+
                            A03      |  A15 jarre  A08 |F4| A09/A10
 3500                                +--------------+  |  | <- passage étroit 140
                                        A07        |  F3  |  A06 (h600)
 1900                                +-------------+      |
                            A02      |  F1     F2 coude ->+
 1600                                |      ------------- +
                            A01      |  A12 muret (Y 1100, h70)     A05
    0                                |  [SPAWN] (0,0)  Yaw 90
 -300                                +---- A04 ----+
     X: -3600   -2800  -2200   -600  0   250  700   1300     2400     4100  4800
```

### 7.2 Élévation est (D1 sur l'annexe à X=0, D2 sur la maison à X=−600 ; vue depuis l'est, nord à droite)

```
 Z
 750 +-------------------- acrotère D05 ----------------------------+
 600 |  ENDUIT FRAIS (D23) : lisse, AUCUNE prise                    |
 560 |                          D2-04 #                              |
 520 |        D2-02 #   D2-03 o                                      |
 480 |  D2-01 o                          D2-05 X   D2-06 o           |
 380 |                                                     D2-07 o   |
 360 +---annexe---------------------+                         ====== D2-08
 300 | toit de l'annexe (repos)     |  ... vide : marches      | D12 palier CP2
 290 |   D1-05 ======== D1-04 o     |      -75 / -150 ...      | (Z 300)
 280 | D1-02 o                      |                          |
 250 |         D1-03 o              |                          |
 170 | D1-01 o                      |                          |
   0 +------------------------------+--------------------------+
     Y 9300   9420   9600   9780  10000   10150  10300  10420 10470..10800
     o = Toron   # = BancoRelief   X = BancoFragile   ==== = Corniche
```

### 7.3 Élévation nord (D3, face côté fleuve ; vue depuis le nord, **est à gauche**)

```
 Z
 810        ^  ^  ^  ^  ^  ^      pinacles D24-D29 (pas de mantle)
 750  ===== CorA (déplacement latéral seul) ======|==== CorB (mantle) ====|
 640      D3-A1 o (gouttière)                     |            |          |
 610                                              |  D3-B5 o   |          |
 520  D3-02 o          +--N1--+          +--N2--+  (sommets des contreforts)
 470 D3-01 o           |      |          |      |
 450        D3-B1 # +--+ B2 # | --saut-> | B3 # B4 #
 300 [D12 palier CP2]  |      |   190 cm |      |
   0 - - - - - - - - - |      | - - - - -|      | - - - - - (sol de la ville, derrière)
-450 ===================berge==soutènement G2========================
     X -600  -710 -870  -1000 -1120     -1320 -1440  -1530   -1700
```

---

## 8. Q12 tranché (paragraphe à recopier dans le GDD §12)

> 12. **Hauteur de la maison du marchand** : ✅ décidé (2026-11-23, `directeur-creatif` par délégation du développeur). La maison reste à l'échelle de Djenné : R+1 en banco, rez de 3,5 m + étage de 3 m, terrasse à 6,5 m, **acrotère à 7,5 m**, pinacles à 8,5-9,1 m. Elle reste nettement sous la mosquée (corps à 10,5 m, tour à 22,5 m). La verticalité ne vient pas de la hauteur, mais de **l'implantation** :
> 1. une **annexe basse mitoyenne** (magasin, 3 m) sert de D1, le tutoriel ;
> 2. D2 est une **traversée de la façade est** au-dessus de la descente vers le quai, avec jusqu'à 6 m de vide ;
> 3. la maison est **au bord de la butte** : sa façade nord tombe sur le soutènement et la berge, 4,5 m plus bas, ce qui donne **12 m de vide sous D3**, qui propose 2 options (corniche ou saut entre contreforts).
>
> Le parcours compte environ 25 m de mur et 23 prises, pour environ 3 min. Justification : selon l'avis historique, 7 à 10 m sont plausibles, alors que 20 m seraient invraisemblables et feraient concurrence à la mosquée, ce qui affaiblirait le refus de Sira. Le frisson vient de l'exposition au vide, pas de la hauteur absolue. En bonus, on voit le fleuve dès D3, ce qui prépare le plan final. Corollaire : la mosquée passe à l'est de la place et la maison au nord-ouest, pour que le soleil de l'aube éclaire les façades escaladées et mette la mosquée à contre-jour (croquis §7.2 à mettre à jour). 🔶 inspiré de (assumé) : l'implantation en bord de butte, l'annexe et l'enduit refait sur une seule façade.

---

## 9. Vérifications de cohérence

| Contrôle | Résultat |
|---|---|
| Du spawn à D1 sans obstacle infranchissable | F1 → muret A12 (70 cm ≤ 90) → F2 → F3 → F4 → F5 → F6 → B → BC → couloir libre de la place → recoin → D1 ✓ |
| Largeurs de ruelle ≥ 250 | F1, F2, F3, F5 : 300 ; F6 : 350 (285 à la jarre A15) ; BC : 400 ✓ |
| Passage étroit ≥ 120 | F4 : 140 ✓ |
| Terrasse de 15 × 12 m, parapet de 100 | Intérieur 1500 × 1200, sol à 650, parapet à 750 ✓ |
| Le joueur ne peut pas monter sur la mosquée | Tertre de 200 (> mantle de 180), 0 prise ✓ |
| Pas de raccourci vers la terrasse | Pas de prise sur l'enduit frais. L'acrotère n'est une prise qu'en CorA (latéral) et CorB. Les toits voisins (350) n'ont pas de prise ✓ |
| Pas de sortie du niveau | Îlots de 350 ou plus, sans prise ; `BV_Descente` ; berge atteignable uniquement par chute, donc rattrapage ; `BV_Terrasse` ✓ |
| Ombres | Aucune ombre de la mosquée ni des îlots voisins sur D1, D2, D3 (C07 : 350 cm à 13 m de D1, donc pas d'ombre) ✓ |
| [J] visible depuis l'arène | ✓ (cf. §5) |
| Nombre d'éléments | Environ 140 éléments de décor et structure + 23 prises, soit **environ 163** (dans la fourchette 80-200), plus environ 25 marqueurs de gameplay ✓ |

## 10. Paramètres à exposer (escalade de cette fiche)

`DT_PrisesEscalade` : `DistanceMaxVerticale` = 150, `DistanceMaxLaterale` = 200, `DelaiEffritement` = 1,5 s, `VitesseCorniche` = 60 cm/s, `bMantleAutorise` (par prise), `HauteurSaisieDepuisSol` = 180.
Caméra : durée de l'indice CP2 = 2 s, durée du plan wow = 4 s, FOV du wow = 60.
Lumière : `SoleilYaw` = −160, `SoleilPitch` = −16.

## 11. À valider par `historien-mande`

- ⚠️ **Enduit refait sur une seule façade** d'une maison privée, avant le crépissage collectif : est-ce plausible qu'un propriétaire réenduise sa propre façade de son côté ?
- ⚠️ **Quai à pirogues au pied de la butte**, avec une descente en marches de terre (et non une rampe) ; **déchargement** de ballots et pirogue retournée sur la place.
- ⚠️ **Tour de la mosquée au nord-est** (côté qibla, tour de mihrab) plutôt que sur la façade. Contreforts à pinacles arrondis. Mosquée à l'est de la place.
- ⚠️ **Rôniers et acacia** en ville et sur la berge.
- ⚠️ **Soleil de mai** comme « fin de saison sèche, avant le crépissage ».
