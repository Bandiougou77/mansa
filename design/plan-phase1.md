# Plan de la Phase 1 — Vertical slice « Djenné »

> Source : `design/GDD.md` v1.2. Chaque tâche dure **1 à 3 h**, se termine par un **test** et un commit sur une branche `feature/<id>` fusionnée dans `develop`.
> Ordre conseillé : les blocs se suivent, mais **B (backend griot)** peut avancer en parallèle de **C-E** les jours où tu n'as pas envie d'ouvrir Unreal.
> Légende agents : `dev` = dev-gameplay-unreal, `ia` = ingenieur-ia-pnj, `art` = tech-art-blender, `son` = sound-designer, `dc` = directeur-creatif, `hist` = historien-mande, `qa` = qa-relecteur.
> Total estimé : **~125-145 h** (≈ 3 mois et demi à 10 h/semaine), dont le bloc V (beauty shot) placé **juste après D** pour fixer tôt le niveau visuel.

---

## Bloc A — Fondations techniques (≈ 12 h)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| A1 | Prendre en main l'éditeur : naviguer, ouvrir/sauver une map, Play, Outliner, Details, Content Browser (guide pas à pas sur le template) | 2 h | `dev` | Tu sais créer un niveau vide, y placer un cube et le sauver dans `Content/Mansa/Maps/` |
| A2 | Organiser `Content/Mansa/` (Maps, Characters, Blueprints, Data, UI, Audio, Materials) et décider du sort des variantes du template (garder `Variant_Combat` comme référence de lecture, retirer Platforming/SideScrolling du code) | 1-2 h | `dev` | Le projet compile et la map Third Person se lance toujours |
| A3 | Classe C++ `AMansaCharacter` (dérivée du personnage du template) + `BP_Sira` + GameMode qui l'utilise | 2 h | `dev` | Play : on contrôle `BP_Sira` avec les mêmes contrôles qu'avant |
| A4 | Enhanced Input : `IMC_Exploration`, `IMC_Escalade`, `IMC_Combat` + actions du GDD §4.2 (AZERTY/QWERTY + manette) | 2 h | `dev` | Chaque action affiche un message à l'écran (clavier et manette) |
| A5 | Socle Data Tables : structs `F…Row` + `DT_CharacterMovement`, `DT_CameraProfiles` lus au démarrage (aucune valeur en dur) | 2-3 h | `dev` | Changer la vitesse de course dans la DT change la vitesse en jeu sans recompiler |
| A6 | Relecture du socle (conventions, exposition Blueprint, perf) | 1 h | `qa` | Liste de remarques traitée ou notée |

## Bloc B — Backend griot (Laravel + API Claude) (≈ 16 h, parallélisable)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| B1 | Créer le projet Laravel dans `/backend`, `.env.example`, clé API dans `.env` (jamais commitée), route `POST /api/griot/talk` qui renvoie une réponse factice | 2 h | `ia` | `curl` sur l'endpoint renvoie un JSON ; `git status` ne montre pas `.env` |
| B2 | Fiche de connaissances `griot_djenne.json` (faits autorisés, ton, interdits) | 2 h | `hist` + `ia` | Relue et validée par `hist` ; chaque fait renvoie à `djenne-1324.md` |
| B3 | Prompt système du griot (personnage 1324, 60 mots max, garde-fous GDD §6.3, refus de la mosquée comme parcours, Sira chasseuse) | 2-3 h | `ia` + `dc` | 10 questions de test donnent des réponses en personnage |
| B4 | Appel réel à l'API Claude (modèle et `max_tokens` en config), historique des 6 derniers échanges en session | 2-3 h | `ia` | Conversation de 8 tours cohérente via `curl` ; latence loguée |
| B5 | Robustesse : timeout 6 s, filtre de sortie, limite de requêtes par session, logs de coût | 2 h | `ia` | Coupure réseau simulée → code d'erreur propre ; dépassement de quota → refus |
| B6 | Banc de 20 questions pièges automatisé (`php artisan griot:eval`) | 2 h | `ia` | ≥ 19/20 réponses restent dans le rôle |
| B7 | Revue sécurité (clé API, CORS, entrées utilisateur, injection de prompt) | 1-2 h | `qa` | Aucune fuite de clé, rapport traité |

## Bloc C — Greybox du niveau (≈ 14 h)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| C1 | Fiche `design/niveaux/djenne-slice.md` : cotes des zones A-E, **trancher Q12** (hauteur de la maison du marchand : piste « bord de butte » 12-14 m ou maisons mitoyennes) | 2 h | `dc` + `hist` | Q12 passe en ✅ dans le GDD |
| C2 | Greybox ruelle A + coin du griot B (cubes / BSP / Modeling Mode) | 2-3 h | `dev` | On marche du spawn au griot sans bloquer ; muret et passage étroit franchissables |
| C3 | Greybox place C + volume de la mosquée (décor, sur tertre) + emplacement [R] et [J] | 2-3 h | `dev` | Le plan large sur la mosquée est lisible depuis l'entrée de la place |
| C4 | Greybox façade du marchand D (D1-D3 aux nouvelles cotes) + terrasse E 15 × 12 m | 2-3 h | `dev` | La géométrie respecte les cotes de C1 (mesure à la règle de l'éditeur) |
| C5 | Lumière d'aube provisoire (Lumen, soleil rasant) pour lire le chemin par la lumière | 1-2 h | `art` | Capture d'écran : le chemin d'escalade se devine sans marqueur |
| C6 | Test de parcours chronométré du greybox complet | 1 h | `qa` | Parcours en marchant ≈ 5-7 min hors dialogue et combat |

## Bloc D — Déplacement et escalade (≈ 18 h)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| D1 | Marche / jogging / course / saut selon `DT_CharacterMovement`, caméra exploration `DT_CameraProfiles` | 2 h | `dev` | Valeurs du GDD §4.3 ressenties ; changement de DT pris en compte |
| D2 | Enjambement et mantle (90-180 cm) avec Motion Warping | 3 h | `dev` | Le muret de la ruelle et les rebords se franchissent proprement |
| D3 | `BP_PriseEscalade` + composant C++ `UMansaPriseComponent` (tag `PriseType`, `DT_PrisesEscalade`) | 2-3 h | `dev` | Les prises posées sur la façade apparaissent en debug avec leur type |
| D4 | Accroche / suspension / déplacement latéral sur prises (`UMansaClimbingComponent`) | 3 h | `dev` | Sira s'accroche à un toron, se déplace, lâche avec C |
| D5 | Saut entre prises (150 cm vertical / 200 cm latéral) + mantle vers la corniche | 3 h | `dev` | D1 se grimpe de bas en haut |
| D6 | Prise `BancoFragile` (1,5 s), volumes de rattrapage + checkpoint (fondu 0,5 s), caméra escalade | 2-3 h | `dev` | Rester sur la prise fragile la fait tomber → retour au checkpoint sans mort |
| D7 | Placement de toutes les prises D1-D3 + mosquée sans aucune prise | 1-2 h | `dev` | Escalade complète D1→terrasse ; aucun prompt « Grimper » sur la mosquée |
| D8 | Relecture escalade (bugs de collision, cas limites) | 1 h | `qa` | 10 montées sans blocage dans la géométrie |

## Bloc V — Beauty shot « Djenné » : la référence visuelle (≈ 14 h, à faire juste après D)

> Une **seule vue**, finie au maximum, qui sert d'étalon visuel pour tout le jeu (procédure : skill `direction-artistique-aaa` §5). Proposition de vue : depuis la terrasse du marchand, la ruelle qui débouche sur la grande mosquée au coucher, le Niger au loin.

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| V1 | Choix du point de vue + composition en greybox (silhouettes, leading lines, monument au loin), planche relue par `hist` | 2 h | `dc` + `art` + `hist` | Capture greybox validée ; `CAM_BeautyShot` posée dans `LV_Djenne_Greybox` |
| V2 | Éclairage « Coucher épique » : Sky Atmosphere, soleil (light shafts), Sky Light temps réel, Volumetric Cloud, Exponential Height Fog + Volumetric Fog, Lumen ; preset réutilisable | 3 h | `art` | Capture avant/après ; rayons et poussière visibles |
| V3 | Master materials `M_Banco_Master` (usure, humidité, crépi frais/ancien, traces de mains), `M_Tissu_Master`, `M_Bois_Master` + sols Megascans/Fab recolorés, dans le champ uniquement | 3 h | `art` | Aucune texture répétitive visible dans le cadre |
| V4 | Densité et vie : PCG pour props au pied des murs et végétation, tissus indigo/bogolan, poteries, nattes, decals ; poussière, fumée, oiseaux (Niagara simple) | 4 h | `art` | ≥ 3 sources de vie dans le cadre |
| V5 | Post-process et grading, `HighResShot 2` dans `design/references/beauty-shots/`, grille de revue (skill §6) notée, liste chiffrée des assets manquants | 2 h | `art` + `qa` | Capture commitée + note /5 par critère + 60 FPS en 1440p mesurés |

---

## Bloc E — Rythme et combat (≈ 26 h)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| E0 | **Vérifier** que le plugin FMOD for Unreal existe pour UE 5.8 ; sinon, plan B (Quartz / MetaSounds d'Unreal) | 1 h | `son` | Décision notée dans `CLAUDE.md` |
| E1 | Installer FMOD Studio + plugin, projet `/audio`, événement djembé 96 BPM avec marqueurs de temps | 2-3 h | `son` | Le djembé joue en jeu ; un message s'affiche à chaque temps |
| E2 | `UMansaRhythmComponent` : horloge pilotée par les callbacks FMOD, qualité Parfait / En rythme / Hors rythme (`DT_RythmeCombat`) + compensation de latence | 3 h | `dev` + `son` | Appuyer en rythme affiche « Parfait » ; dérive < 10 ms après 3 min (log) |
| E3 | Fiche `design/mecaniques/combat-rythmique.md` (détails du GDD §5) | 1-2 h | `dc` | Relue, valeurs cohérentes avec les DT |
| E4 | Attaques légère / lourde / esquive (`DT_AttaquesJoueur`), animations provisoires du template (en s'inspirant de `Variant_Combat`) | 3 h | `dev` | Les 3 actions marchent, avec timings du GDD §5.3 |
| E5 | Multiplicateurs de rythme, compteur de combo, paliers, décroissance (GDD §5.4) | 2-3 h | `dev` | Log des dégâts conforme au tableau §5.4 |
| E6 | Ennemi de base C++ + `BP_Ennemi_Voleur` (IA StateTree, attaques calées sur les temps, `DT_Ennemis`) | 3 h | `dev` | Le voleur poursuit Sira et attaque sur les temps |
| E7 | `BP_Ennemi_Brute` (garde frontale, charge) + jeton d'attaque (un seul attaquant à la fois) | 3 h | `dev` | La brute entre à 50 % des PV du voleur ; jamais 2 attaques simultanées |
| E8 | PV de Sira, mort → checkpoint, déclenchement du combat au mantle sur la terrasse | 2 h | `dev` | Perdre ramène au début du combat ; gagner débloque la suite |
| E9 | Feedback combat (hit-stop, anneau de pulsation, sons, couche FMOD `Combo` 0-3) + options « Aide au rythme » / « Rythme désactivé » | 3 h | `dev` + `son` | Combat gagnable en « Rythme désactivé » |
| E10 | Relecture combat + mesure de dérive | 1-2 h | `qa` | Rapport + log de dérive joint |

## Bloc F — Griot dans le jeu (≈ 14 h)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| F1 | Client HTTP Unreal (module HTTP) vers `POST /api/griot/talk`, URL du backend en config, timeout 6 s | 2-3 h | `dev` + `ia` | Une question envoyée depuis le jeu reçoit la réponse du backend (log) |
| F2 | Widget de dialogue UMG : 3 questions suggérées + champ libre + indicateur « le griot réfléchit » | 3 h | `dev` | Conversation complète à la souris et à la manette |
| F3 | `DT_GriotFallback` (≥ 15 répliques validées) + bascule automatique en erreur | 2 h | `dev` + `hist` | Backend arrêté → réponses de fallback en ≤ 6 s, aucun message d'erreur |
| F4 | Narrateur : `DT_NarrateurLignes` + volumes de déclenchement + sous-titres | 2-3 h | `dev` | Les lignes se déclenchent aux bons endroits, une seule fois |
| F5 | Scène du refus `TV_RefusMosquee` (réplique de Sira + narrateur + caméra vers le marchand) et plan « wow » | 2 h | `dev` | Se déclenche à chaque partie, skippable |
| F6 | Choix `bVoleurEpargne` + fin à 2 variantes + corrections du narrateur (saut de toit, attente au marché) | 2 h | `dev` | Les 2 fins s'affichent selon le choix |

## Bloc G — Art, audio, contenu (≈ 20 h)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| G1 | Installer Blender (LTS) + pipeline d'export FBX vers UE (script Python, échelle, axes, nommage `SM_`) | 2 h | `art` | Un cube exporté arrive à la bonne échelle dans UE |
| G2 | Matériaux `M_Banco` (érosion par vertex paint), `M_Toron`, `M_SolTerreBattue` (Megascans + Substance) | 3 h | `art` | Les murs du greybox prennent l'aspect banco |
| G3 | Kit modulaire banco (murs, contreforts, acrotère, pinacles, toron, gouttières) | 3 h | `art` | La ruelle est habillée avec le kit |
| G4 | Façade de la maison du marchand (escaladable, prises alignées sur le mesh) | 3 h | `art` + `dev` | L'escalade fonctionne toujours après habillage |
| G5 | Silhouette de la mosquée (décor, détail moyen, 🔶 « médiévale imaginée ») — planche relue par `hist` avant modélisation | 3 h | `art` + `hist` | Validation `hist` ; lisible depuis la place et la terrasse |
| G6 | Personnages : Sira, griot, voleur, brute, passants (MetaHuman ou équivalent + tenues) — planches relues par `hist` | 3 h × 2 | `art` + `hist` | Les 4 personnages principaux sont en jeu |
| G7 | Ambiance FMOD : marché, appel à la prière lointain, pilons, oiseaux, fleuve ; nappe d'escalade | 2-3 h | `son` | Ambiance qui change selon la zone |

## Bloc H — Finition et validation (≈ 12 h)

| ID | Tâche | Durée | Agent | Comment tester |
|---|---|---|---|---|
| H1 | Menu pause + options (sensibilité, rythme, latence, sous-titres, liseré des prises) | 2-3 h | `dev` | Chaque option a un effet visible |
| H2 | Profilage : 60 FPS en 1080p « Élevé » (Nanite, Lumen, LOD) | 2-3 h | `art` + `qa` | `stat fps` ≥ 60 sur tout le parcours |
| H3 | Build packagé Windows + config de l'URL du backend + vérification « clé API absente du build » | 2 h | `dev` + `qa` | Le build se lance sur une 2e machine ; recherche de chaîne négative |
| H4 | Tests utilisateurs (3 personnes) : durée, compréhension, rythme | 2-3 h | `qa` + `dc` | Grille de la Definition of Done (GDD §11) remplie |
| H5 | Corrections issues des tests + relecture culturelle finale (lecteur musulman malien, connaisseur des donso si possible) | 3 h | `dev` + `hist` | Tous les critères de la §11 cochés |

---

## Jalons

1. **Fin A + C** : on se promène dans un Djenné en cubes. → commit taggé `slice-greybox`
2. **Fin D** : on grimpe la maison du marchand. → `slice-escalade`
2 bis. **Fin V** : le beauty shot de Djenné existe et sert de référence visuelle. → `slice-beauty-shot`
3. **Fin E** : le combat au djembé est jouable en greybox. → `slice-combat`
4. **Fin B + F** : le griot parle dans le jeu. → `slice-griot`
5. **Fin G + H** : la slice est présentable. → `v0.1-slice` fusionné dans `main`

## Risques identifiés

- **FMOD + UE 5.8** : plugin peut-être pas encore disponible pour la 5.8 (vérifié en E0, plan B : Quartz/MetaSounds).
- **Escalade** : c'est le système le plus long pour un débutant ; garder les prises discrètes, pas d'escalade libre.
- **Animations** : le template fournit les bases ; les animations d'escalade et de lance devront venir de Mixamo, du Marketplace (Fab) ou d'un pack, à choisir en D4/E4.
- **Coût API** : borné en B5 (limite par session + logs).
- **Variantes du template** : code de `Variant_Combat` utile comme modèle, mais à ne pas garder tel quel (nommage, Data Tables).
