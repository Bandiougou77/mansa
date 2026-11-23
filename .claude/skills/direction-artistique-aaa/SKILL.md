---
name: direction-artistique-aaa
description: Direction artistique et rendu visuel AAA de MANSA (niveau Assassin's Creed / Uncharted) dans Unreal Engine 5.8. À utiliser pour toute tâche qui touche à l'apparence du jeu - éclairage, ciel, brouillard, post-process, matériaux, paysages, architecture, végétation, PCG, placement d'assets, caméra, beauty shots, revues visuelles et optimisation du rendu.
---

# Direction artistique AAA — MANSA

Objectif : chaque image du jeu doit pouvoir servir de capture promotionnelle. Référence de qualité : Assassin's Creed (Origins, Mirage), Uncharted 4, Ghost of Tsushima, Horizon. L'Empire du Mali en 1324 était l'un des plus riches du monde : **ça doit se voir à l'écran**.

## 1. Les 6 règles d'or

1. **Lumière d'abord.** 70 % de la beauté vient de l'éclairage. On règle la lumière avant d'ajouter des détails.
2. **Silhouettes lisibles.** De loin, chaque lieu doit se reconnaître à sa silhouette (minarets, tours en banco, remparts, baobabs).
3. **Échelle grandiose.** Toujours un élément monumental visible au loin (mosquée, palais, falaise, fleuve) pour donner envie d'y aller.
4. **Densité de détails à hauteur d'yeux.** Poteries, nattes, tissus, étals, graffitis de mains dans le banco, traces d'usure. Rien de « propre et vide ».
5. **Couleurs contrôlées.** Une palette dominante par zone, des accents saturés rares (or, indigo, rouge garance) pour guider l'œil.
6. **Vie.** Poussière, fumée de forge, oiseaux, tissus qui bougent au vent, foule, eau qui coule. Un décor immobile est un décor mort.

## 2. Palette et matières

| Élément | Couleur / rendu |
|---|---|
| Banco (terre crue) | Ocre, beige rosé, brun rouge ; crépi frais lisse et clair vs ancien fissuré et foncé |
| Or | Accents rares : bijoux, tissus royaux, reflets — jamais en aplats |
| Indigo et bogolan | Tissus, voiles, tentures ; contraste froid sur la terre chaude |
| Végétation | Vert sec, jaune paille en saison sèche ; vert profond en saison des pluies |
| Ciel | Bleu poudré, horizon laiteux de poussière (harmattan) ; couchers orange-rose intenses |
| Eau du Niger | Brun-vert, reflets dorés au couchant |

Matériaux : toujours partir d'un **master material** paramétrable (`M_Banco_Master`, `M_Tissu_Master`, `M_Bois_Master`) avec paramètres d'usure, d'humidité, de saleté et de variation par instance. Utiliser du **vertex painting** ou des **decals** pour casser la répétition (traînées d'eau, mains, fissures, saleté au pied des murs).

## 3. Pipeline Unreal 5.8 (réglages de base)

### Éclairage
- **Lumen** : Global Illumination + Reflections activés (Project Settings → Rendering).
- **Directional Light** (soleil) : Atmosphere Sun Light activé, intensité physique, angle source ~0.5 pour des ombres douces ; light shafts activés.
- **Sky Atmosphere** + **Sky Light** (Real Time Capture) + **Volumetric Cloud**.
- **Exponential Height Fog** avec **Volumetric Fog** activé : c'est lui qui crée la poussière et les rayons de lumière.
- Préparer **3 presets d'éclairage** (Data Asset ou niveau de lumière dédié) : *Matin doré*, *Midi écrasant*, *Coucher épique*. Le Coucher épique est le preset des beauty shots.

### Post-process (volume global)
- Exposition : manuelle ou plage resserrée (éviter le pompage).
- Tonemapper filmique par défaut, légère **color grading** chaude dans les ombres moyennes.
- Bloom léger, vignette discrète, **film grain** très faible, aberration chromatique quasi nulle.
- Profondeur de champ uniquement en cinématique.

### Géométrie
- **Nanite** sur tous les meshes statiques compatibles (bâtiments, rochers, props).
- **Megascans / Fab** pour roches, sols, végétation, props génériques. Toujours retoucher la couleur pour coller à la palette.
- **PCG (Procedural Content Generation)** pour : végétation, éboulis, props au pied des murs, foules de détails dans les ruelles, quartiers entiers à partir de règles.
- **World Partition** activé dès le départ pour préparer le monde ouvert.

### Paysages
- **Landscape** avec matériau auto (pente → roche, plat → terre/herbe, bord d'eau → boue).
- **Water plugin** pour le Niger (rivière avec flux, rives humides).
- Horizon : toujours des éléments lointains (falaises de Bandiagara, collines, fumées de villages) via des meshes simplifiés ou des impostors.

## 4. Composition et caméra (le style Uncharted / AC)

- **Leading lines** : ruelles, poutres *toron*, rives du fleuve guident vers l'objectif.
- **Lumière = chemin** : les zones escaladables et le chemin principal sont plus éclairés ou marqués par des tissus clairs (pas de peinture jaune façon autres jeux : ici des **tissus indigo/blancs** et des **traces de mains blanches** sur le banco).
- **Points de vue panoramiques** (équivalent des points de synchronisation) : sommets avec une vue soignée sur la ville et le fleuve, et une courte cinématique de caméra.
- Caméra 3e personne : FOV ~80-90, légère inertie, plus large en exploration, plus serrée en combat.

## 5. Procédure « beauty shot »

À faire tôt pour chaque nouvelle zone. C'est la référence visuelle qu'on ne doit jamais faire baisser.

1. Choisir **un seul point de vue** marquant (ex. ruelle de Djenné débouchant sur la mosquée au loin, au coucher).
2. Greybox rapide → valider la composition (silhouettes, leading lines, monument au loin).
3. Éclairage *Coucher épique* + fog volumétrique.
4. Matériaux finaux dans le champ uniquement.
5. Densifier : props, tissus, végétation, decals, foule.
6. Vie : particules (poussière, fumée), oiseaux, tissus animés.
7. Post-process et grading.
8. Capture en haute résolution (`HighResShot 2`) dans `/design/references/beauty-shots/`.
9. Lister ce qui manque pour atteindre le niveau AAA (assets à acheter ou à faire par un artiste).

## 6. Grille de revue visuelle

Avant de dire qu'une zone est « finie », vérifier chaque point et donner une note sur 5 :

- [ ] Silhouette reconnaissable de loin
- [ ] Élément monumental visible au loin
- [ ] Lumière qui guide le joueur
- [ ] Aucune texture répétitive visible
- [ ] Bas des murs et coins sales et détaillés (pas de jonctions nettes sol/mur)
- [ ] Au moins 3 sources de vie (particules, animaux, tissus, PNJ)
- [ ] Palette cohérente avec la zone
- [ ] Authenticité validée par `historien-mande` (architecture, tissus, objets)
- [ ] Performance : 60 FPS en 1440p sur la machine de dev, sinon lister les coupables (`stat unit`, `stat gpu`, Nanite et Lumen visualizers)

## 7. Honnêteté sur les limites

Claude peut réaliser : code, matériaux, éclairage, post-process, PCG, scripts Blender/Unreal, placement procédural, caméra, particules simples, organisation des assets.

Demandent des ressources externes :
- **Personnages principaux** (Sira, griot, Mansa) de qualité AAA : MetaHuman + vêtements sur mesure, ou artiste 3D.
- **Bâtiments uniques** (grande mosquée, palais de Niani) : kit modulaire possible par script, mais la version héroïque demande un artiste ou un asset de qualité.
- **Animations** de qualité (escalade, combat) : motion capture ou packs du marché.

Quand une tâche atteint ces limites, le dire clairement et proposer : (a) un placeholder de bonne qualité gratuit sur Fab, (b) un asset payant avec estimation de prix, (c) un brief pour un artiste.

## 8. Règles de travail

- Toujours **montrer une capture avant/après** (demander à l'utilisateur d'en faire une si besoin) pour toute modification visuelle.
- Donner les étapes éditeur **une par une**, avec les noms des menus en français.
- Ne jamais sacrifier la lisibilité du gameplay pour la beauté.
- Garder les budgets : textures en 2K par défaut, 4K seulement pour les éléments héroïques.
