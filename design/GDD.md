# MANSA — L'Héritier du Fleuve
## Game Design Document — v1.2 (Phase 0, centré sur la vertical slice « Djenné »)

> Statut : v1, brouillon de travail. Rédigé par `directeur-creatif`.
> Les données chiffrées sont des **valeurs de départ** à régler en jeu : elles vivent dans des Data Tables (`DT_*`) ou des fichiers JSON, **jamais en dur dans le code**.
> Marqueur utilisé : **⚠️ à valider par historien-mande** = tout élément historique ou culturel qui n'est pas un fait solide. Voir aussi `design/lore/glossaire.md` et `design/lore/djenne-1324.md` (rédigés en parallèle). Si ces fichiers contredisent ce GDD, **le lore validé l'emporte**.

---

## 1. Vision

### 1.1 Pitch
**1324.** Mansa Moussa quitte le Mali pour son pèlerinage à La Mecque. Pendant son absence, une relique sacrée est volée. Sira, jeune chasseuse *donso* (une exception, que le griot relève), remonte la piste des voleurs, de ville en ville, jusqu'au mystère de la **flotte disparue d'Aboubakari II**. Ce récit est une **légende** présentée comme telle : al-Umari rapporte que Moussa a parlé d'un prédécesseur parti sur l'océan avec ses navires ; le nom « Aboubakari II » est une identification moderne, et une arrivée en Amérique n'est étayée par aucune preuve (cf. `glossaire.md`, §7).
Des siècles plus tard, un **griot** raconte cette épopée. Il commente, corrige, et l'épopée qu'il chante à la fin dépend de ce que le joueur a fait.

### 1.2 Ton
Épique, respectueux, authentique. Le Mali de 1324 est un empire riche, lettré, commerçant et urbain. La grandeur vient des lieux, des gens et de la parole, pas de l'exotisme.

### 1.3 Public
- Joueurs 16+ d'action-aventure narrative (Uncharted, Assassin's Creed, Ghost of Tsushima).
- Joueurs curieux d'histoire, notamment de l'Afrique de l'Ouest et de sa diaspora.
- Plateforme cible de la slice : PC (clavier/souris + manette Xbox/PlayStation).

### 1.4 Références et ce qui rend MANSA différent
| Référence | Ce qu'on prend | Ce que MANSA fait autrement |
|---|---|---|
| **Uncharted** | Rythme scénarisé, escalade lisible, héros incarné | Escalade sur **architecture de terre** (banco) avec prises naturelles (toron, gouttières) ; héroïne chasseuse *donso*, pas pilleuse de tombes ; refus d'escalader le lieu sacré (la mosquée), assumé comme moment de caractère |
| **Assassin's Creed** | Ville historique explorable, verticalité, point de vue en hauteur | Pas d'« Animus » : le cadre est un **griot** qui raconte et peut se tromper, se reprendre, adapter l'histoire |
| **Ghost of Tsushima** | Respect d'une culture, DA forte | La culture est portée par la **parole et le son** (griot, djembé), pas seulement par l'image |
| **Hi-Fi Rush / Sekiro** | Combat calé sur le rythme, lecture de l'ennemi | Rythme **joué par un djembé diégétique**, bonus sans punition lourde, accessible au débutant |

### 1.5 Ce qu'on refuse
- Les clichés « Afrique générique » : savane avec lions au coucher du soleil, tambours « tribaux » génériques, masques mélangés de cultures différentes, huttes de paille partout, peintures corporelles fantaisistes.
- Le « sauveur extérieur » et le regard colonial : pas de guide européen, pas d'« exploration de terres inconnues ».
- Les langues inventées type charabia : les termes mandingues sont réels et validés, sinon absents.
- La magie spectaculaire décontextualisée : le *nyama* (Phase 2) est traité comme une notion culturelle sérieuse, pas comme du « mana ».
- Le mélange d'époques : pas d'éléments de l'Empire songhaï, de la colonisation ou de l'époque moderne présentés comme de 1324 sans le dire.

---

## 2. Les 5 piliers : présent dans la slice vs reporté

Règle d'arbitrage : la slice doit **prouver** deux piliers (griot, combat rythmique) et **ne pas contredire** les trois autres.

| # | Pilier | Dans la slice (Phase 1) | Reporté (Phase 2+) |
|---|---|---|---|
| 1 | **Griot narrateur** | **OUI, cœur de la slice.** Voix off scriptée (intro, escalade, combat, fin) avec au moins **1 correction** (« non, ce n'est pas comme ça… ») + **1 PNJ griot interactif** via API Claude | Épopée finale dépendante des choix, mémoire des choix sur toute la partie, voix TTS généralisée, STT (Whisper) |
| 2 | **Nyama** | **NON.** Tout au plus un clin d'œil textuel : le griot mentionne le mot dans une réplique. Aucune jauge, aucun système | Jauge de nyama, gain en combat/chasse, corruption |
| 3 | **Combat rythmique** | **OUI.** 1 combat, 2 ennemis, djembé à BPM fixe, coups en rythme, combo | Variations de tempo, rythmes par type d'ennemi, ensembles de percussions, boss |
| 4 | **Monde saisonnier** | **NON.** Saison fixe (saison sèche). Le crépi frais/ancien n'existe pas encore comme système, mais les prises d'escalade sont taguées pour pouvoir l'ajouter | Crue/décrue du Niger, crépissage annuel modifiant les prises |
| 5 | **Mémoire orale (fasa)** | **NON** comme système. Clin d'œil optionnel : le griot PNJ peut réciter un court passage si on le lui demande (texte validé dans `DT_GriotFallback`) | Fasa comme clés de dialogues, portes, alliances |

Décision tranchée : **aucun code de nyama, fasa ou saisons dans la Phase 1.** Seul l'architecture de données anticipe (tag `PriseType` sur les prises, cf. §4.3).

---

## 3. Boucle de jeu de la slice

### 3.1 Boucle principale (résumé)
Explorer la ruelle → écouter / questionner le griot → traverser la place au pied de la mosquée (Sira refuse de l'escalader) → escalader la façade de la grande maison de marchand → combattre 2 voleurs sur sa terrasse, au rythme du djembé → récupérer un indice → le griot clôt le chapitre, la mosquée en toile de fond.

### 3.2 Déroulé minute par minute (cible 12 min, plage 10-15)

| Temps | Séquence | Ce que fait le joueur | Ce que fait le jeu | Tension |
|---|---|---|---|---|
| 0:00-1:00 | **Intro** | Regarde (skippable) | Écran noir, voix off du griot narrateur (scriptée). Fondu sur la ruelle à l'aube | Repos |
| 1:00-3:00 | **Ruelle : tutoriel déplacement** | Marche, court, saute un muret, se faufile entre des passants | Prompts contextuels discrets. Marché qui s'éveille | Repos |
| 3:00-5:30 | **Rencontre du griot (PNJ)** | Parle au griot : 3 questions suggérées + saisie libre | Réponses via API Claude (cf. §6). Le griot donne l'objectif : « l'homme au turban indigo a filé vers la place ; on l'a vu monter chez le marchand, la grande maison aux gouttières, à côté de la mosquée » 🔶 inspiré de (assumé) : le turban des musulmans et le voile de visage des Sahariens sont attestés (Ibn Battuta) ; une teinture indigo à Djenné en 1324 est plausible mais pas documentée (cf. `glossaire.md`, §6 « indigo ») | Repos / curiosité |
| 5:30-6:30 | **Place et pied de la mosquée** | Traverse la place du marché, passe au pied de la mosquée | Plan large sur la mosquée (moment « wow »). Voix off : première correction du narrateur. Au pied de la mosquée, dont les toron offrent le chemin le plus court vers les toits, **Sira s'arrête et refuse de l'escalader** (réplique de Sira + commentaire du narrateur, cf. §6.2), puis se tourne vers la maison du marchand voisine | Montée |
| 6:30-9:30 | **Escalade de la maison du marchand** | Grimpe la façade en banco via les toron, les gouttières et les reliefs, 1 saut entre contreforts, 1 passage en corniche | Tutoriel escalade, 1 prise qui s'effrite (frisson scripté, sans mort). La mosquée reste visible derrière Sira pendant la montée | Montée |
| 9:30-12:00 | **Combat sur la terrasse du marchand** | Affronte 2 ennemis (1 voleur agile, 1 brute) au rythme du djembé | Le djembé démarre (diégétique : joué en contrebas, cf. §5.1). Tutoriel rythme sur le 1er ennemi | Pic |
| 12:00-13:00 | **Indice** | Fouille le voleur vaincu, ramasse un objet-indice | Gros plan sur l'indice (ex. fragment de bois gravé d'une pirogue) 🔶 inspiré de (assumé) : la pirogue de bois et la sculpture sur bois sont plausibles en 1324 ; l'objet lui-même est inventé. Le lien avec la flotte d'Aboubakari II doit rester au stade de la rumeur (cf. `djenne-1324.md`, §11) | Retombée |
| 13:00-14:00 | **Fin** | Regarde le fleuve depuis la terrasse du marchand, la mosquée à contre-jour | Voix off conclusive, variante selon 1 « choix » mineur (épargner ou non le voleur, cf. §6.4). Carton « À suivre » | Repos |

---

## 4. Personnage : Sira

### 4.1 Identité (slice)
- Jeune **chasseuse** *donso* (confrérie de chasseurs du Mandé), prénom **Sira**. ✅ décidé (§12, Q4) : les confréries donso sont traditionnellement masculines, Sira est donc une **exception**, que le griot relève et commente avec gravité, sans jamais la présenter comme banale ni se moquer de la tradition (les donso sont une tradition vivante). Écho narratif : la **femme-buffle du Do** 🔶 inspiré de (assumé) : épisode de l'épopée de Soundiata (tradition orale, versions multiples). Do Kamissa, femme de pouvoir dont le double-buffle ravage le pays de Do, finit par livrer elle-même son secret à deux chasseurs, qui la vainquent et emmènent Sogolon, future mère de Soundiata. C'est donc une figure de **puissance redoutable**, qui est l'adversaire des chasseurs avant d'être leur alliée, et non une « chasseuse ». Le griot s'en sert comme écho poétique, pas comme filiation ni comme fait historique (cf. `glossaire.md`, §2). Répliques exemples en §6.2. Tenue : tunique de coton en bandes, teinte ocre ou brune, amulettes de cuir, cornes, cauris, bonnet de chasseur. Éléments historiques : la tunique couverte d'amulettes est documentée à l'époque moderne (hypothèse pour 1324, sans miroirs ni verroterie abondante) ; couteau de chasse et lance légère sont plausibles. **« Sira »** est surtout un prénom féminin au Mali, ce qui convient au personnage ⚠️ (à confirmer par un locuteur) (cf. `djenne-1324.md`, §8 ; `glossaire.md`, §3).
- Arme de la slice : **un couteau de chasse + une lance courte** ✅ décidé (lance légère plausible, cf. `djenne-1324.md`, §8), pas d'arc (hors scope).

### 4.2 Contrôles

| Action | Clavier / souris | Manette |
|---|---|---|
| Se déplacer | ZQSD (AZERTY) / WASD (QWERTY) | Stick gauche |
| Caméra | Souris | Stick droit |
| Marcher (lent) | Alt (maintenir) | Stick gauche partiel |
| Courir | Maj gauche (maintenir) | L3 (cliquer) ou stick à fond |
| Sauter / grimper / saut d'escalade | Espace | A / Croix |
| Lâcher une prise | C | B / Rond |
| Interagir / parler | E | X / Carré |
| Attaque légère | Clic gauche | X / Carré (en combat) |
| Attaque lourde | Clic droit | Y / Triangle |
| Esquive | Ctrl gauche / Maj (en combat) | B / Rond (en combat) |
| Verrouiller une cible | Molette clic | R3 |
| Pause | Échap | Start / Options |

Toutes les touches remappables (Enhanced Input, `IMC_Exploration`, `IMC_Escalade`, `IMC_Combat`). Note : « Interagir » et « Attaque légère » partagent X/Carré sur manette, le contexte (`IMC_Combat` actif ou non) tranche.

### 4.3 Déplacement (valeurs de départ, `DT_CharacterMovement`)
| Paramètre | Valeur de départ |
|---|---|
| Vitesse marche | 200 cm/s |
| Vitesse jogging (défaut) | 400 cm/s |
| Vitesse course | 650 cm/s |
| Hauteur de saut | 120 cm |
| Enjambement auto (muret) | ≤ 90 cm |
| Escalade auto (« mantle ») | 90-180 cm |

Base : `CharacterMovementComponent` standard d'UE5 + Motion Warping pour le mantle. Pas de parkour libre dans la slice.

### 4.4 Escalade
**Intention** : sentir que la grande maison du marchand est un bâtiment *fait à la main*, et que ses prises (toron, gouttières, reliefs du banco) sont naturelles, pas des marqueurs jaunes. Sentir aussi, par contraste, que la mosquée voisine est un lieu qu'on ne gravit pas : elle n'a **aucune prise active** (aucun `BP_PriseEscalade`), Sira refuse de la grimper (§3.2, §7.3), et elle sert de repère visuel pendant toute la montée.

**Principe technique (simple, adapté à un débutant)** : escalade **à prises discrètes** (comme Uncharted), pas d'escalade libre sur toute surface. Chaque prise est un acteur ou composant placé à la main (`BP_PriseEscalade`) avec un tag de type.

**Types de prises (`DT_PrisesEscalade`)** :
| Type (`PriseType`) | Description | Règle |
|---|---|---|
| `Toron` | Poutre de bois de palmier rônier qui dépasse du mur 🔶 inspiré de (assumé) : la fonction des toron (échafaudage permanent pour le crépissage) est attestée ; on ignore depuis quand ils existent. Ils sont plus fréquents sur la mosquée et les grandes façades que sur les petites maisons, ce qui rend plausible leur présence sur une grande maison de marchand. On les garde, de façon irrégulière (cf. `djenne-1324.md`, §6). Les **gouttières** saillantes en demi-tronc utilisent aussi ce type | Prise sûre, solide, on peut s'y suspendre indéfiniment |
| `BancoRelief` | Arête, saillie de contrefort, rebord de fenêtre en banco | Prise standard |
| `BancoFragile` | Banco érodé | S'effrite après **1,5 s** de suspension, la prise disparaît (scripté, 1 seule dans la slice) |
| `Corniche` | Rebord horizontal (terrasse, sommet de mur) | Déplacement latéral + mantle vers le haut |
| `Reserve_CrepiFrais` | Réservé Phase 2 (saisons) | Non utilisé dans la slice |

**Règles** :
- Distance max de saut entre prises : **verticale 150 cm, latérale 200 cm** (`DT_PrisesEscalade`).
- Pas d'endurance dans la slice (simplification). Exception : `BancoFragile`.
- Pas de mort par chute dans la zone d'escalade : volumes de « rattrapage » qui ramènent au dernier checkpoint (fondu noir **0,5 s**) au lieu de tuer.
- Visibilité des prises : lisibles par la forme (bois sombre sur banco clair) + léger liseré plus clair **uniquement** en mode « aide » (option d'accessibilité, désactivable).

### 4.5 Caméra
| Contexte | Distance (bras) | FOV | Notes |
|---|---|---|---|
| Exploration | 300 cm | 85° | Légèrement décalée épaule droite |
| Escalade | 400 cm | 90° | Recul pour lire le mur ; la caméra s'oriente vers la prochaine prise suggérée |
| Combat | 450 cm | 90° | Cadre les 2 ennemis ; verrouillage optionnel |
| Plan « wow » mosquée | Caméra scriptée | — | 4 s max, skippable |

Valeurs dans `DT_CameraProfiles`, transitions interpolées sur **0,4 s**.

---

## 5. Combat rythmique (slice)

> Fiche détaillée à produire : `design/mecaniques/combat-rythmique.md`.

### 5.1 Intention
Le joueur doit sentir qu'il **danse avec le djembé** : frapper en rythme est gratifiant, rater le rythme n'est jamais humiliant.
Diégétique : un joueur de djembé, en contrebas sur la place, au pied de la maison du marchand, accompagne le combat. ✅ décidé (§12, Q1) : on **garde le djembé**, assumé par le narrateur, qui « raconte avec les tambours de son temps » ; il le dit lui-même à l'écran (réplique en §6.2). L'ensemble tambour de place + pilons + balafon tenant le tempo est **à étudier en Phase 2**. Fait établi : le djembé **n'est attesté par aucune source médiévale** (hypothèse pour 1324) ; en revanche, les tambours sont attestés au Mali (Ibn Battuta, 1352 : tambours et trompettes de cour), ainsi que le balafon. Un tambourinaire sur une place de marché est plausible (cf. `glossaire.md`, §4).

### 5.2 Tempo et fenêtre (valeurs de départ, `DT_RythmeCombat`)
| Paramètre | Valeur de départ | Plage de réglage |
|---|---|---|
| Tempo | **96 BPM** (1 temps = 625 ms) | 80-120 |
| Mesure | 4 temps, temps fort sur le 1 et le 3 | — |
| Fenêtre « Parfait » | **±60 ms** autour du temps | 40-100 |
| Fenêtre « En rythme » | **±120 ms** autour du temps | 80-180 |
| Hors fenêtre | « Hors rythme » | — |
| Compensation latence audio/entrée | 0 ms par défaut, réglable joueur | -150 à +150 |

Source de vérité du temps : **la timeline de l'événement FMOD** (callbacks de beat), pas un timer de gameplay séparé, pour éviter la dérive son/jeu.

### 5.3 Actions
| Action | Dégâts de base | Durée (startup / actif / récupération) | Notes |
|---|---|---|---|
| Attaque légère | 10 | 150 / 100 / 250 ms | Enchaînable jusqu'à 3 |
| Attaque lourde | 25 | 400 / 150 / 450 ms | Brise la garde de la brute |
| Esquive | — | 350 ms dont **200 ms d'invulnérabilité** | Recul de 250 cm |

Armes : **couteau de chasse** pour les attaques légères, **lance courte** pour l'attaque lourde (coup d'estoc ou balayage à deux mains, avec de l'allonge) ; les animations de combat et le finisher sont faits pour cette paire d'armes (pas de bâton). Valeurs dans `DT_AttaquesJoueur`. Le moment de l'**impact** (fin du startup) est celui qui est comparé au temps, pas l'appui de la touche : l'appui est anticipé par le système d'une durée = startup. Choix à valider en prototype : si c'est trop complexe, comparer l'appui (plus simple, voir Questions ouvertes).

### 5.4 Ce que donne un coup en rythme
| Qualité | Multiplicateur dégâts | Effet combo | Autre |
|---|---|---|---|
| Parfait | ×1,5 | +2 au compteur | Micro-ralenti 60 ms (hit-stop) |
| En rythme | ×1,25 | +1 au compteur | Hit-stop 30 ms |
| Hors rythme | ×1,0 | Le compteur **baisse de 1** (ne retombe pas à 0) | Aucun |

- **Combo** : à 4, 8 et 12 points, palier qui ajoute ×1,1 / ×1,2 / ×1,3 aux dégâts (cumulé avec la qualité). Plafond ×1,95 (×1,5 × ×1,3).
- **Finisher rythmique** : au palier 8, une attaque lourde « Parfait » déclenche une animation de finisher (fin du combat contre le voleur si PV ≤ 30 %).
- **Esquive en rythme** : esquive « Parfait » = fenêtre d'invulnérabilité portée à 300 ms + contre-attaque légère disponible pendant 400 ms.
- Le compteur décroît de 1 toutes les **2 mesures** sans action.
- Prendre un coup : compteur −2.

### 5.5 Feedback si on rate le rythme (doux)
- Aucun bruit d'erreur strident. Le coup part normalement, avec un son de frappe « sec » sans la résonance de djembé.
- Pendant le tutoriel (1er ennemi), un indicateur visuel apparaît après **3 coups hors rythme consécutifs** : l'anneau de pulsation du HUD grossit pendant 4 mesures.
- Option d'accessibilité : « Aide au rythme » (fenêtre élargie à ±180 ms) et « Rythme désactivé » (tous les coups comptent « En rythme »). Le combat doit rester gagnable sans jamais frapper en rythme.

### 5.6 Ennemis de la slice (`DT_Ennemis`, valeurs de départ)
| Paramètre | **Voleur agile** (`BP_Ennemi_Voleur`) | **Brute** (`BP_Ennemi_Brute`) |
|---|---|---|
| PV | 80 | 160 |
| Dégâts par coup | 8 | 18 |
| Vitesse déplacement | 450 cm/s | 280 cm/s |
| Attaques | Estafilade rapide (télégraphe 400 ms), double coup | Coup lourd (télégraphe 700 ms), charge (télégraphe 900 ms) |
| Garde | Aucune, esquive 1 attaque sur 3 | Garde frontale : bloque les légères, brisée par 1 lourde |
| Cadence d'attaque | 1 attaque / 2 s | 1 attaque / 3,5 s |
| Rythme | Ses attaques **tombent sur les temps** (lisible, apprend le rythme au joueur) | Ses attaques tombent sur le temps fort **1** |
| Récompense | Laisse tomber l'indice | — |

- Ordre : la brute observe pendant que le voleur attaque ; elle entre en jeu à 50 % des PV du voleur ou après 45 s. Un seul ennemi attaque à la fois (« jeton d'attaque »).
- PV de Sira : **100**, aucune régénération en combat, checkpoint au début du combat.
- Identité des ennemis (mercenaires ? hommes de main d'un commanditaire ?) et leurs tenues : 🔶 inspiré de (assumé). Pas d'obstacle historique, quel que soit le choix narratif. Tenues : tunique usée, turban ou voile de visage ; armes : couteau, gourdin, lance courte, bouclier de peau pour la brute, éventuellement une épée droite (lame importée). Pas de cotte de mailles, pas d'arme à feu (cf. `djenne-1324.md`, §8).

### 5.7 Feedback combat
| Événement | Visuel | Son | Caméra |
|---|---|---|---|
| Temps du djembé | Anneau de pulsation discret sous Sira | Frappe djembé (FMOD) | — |
| Coup Parfait | Éclat de poussière ocre, trace de mouvement | Frappe joueur alignée sur le djembé (couche « tone » ou « slap ») | Hit-stop 60 ms, micro-secousse |
| Coup En rythme | Éclat plus petit | Frappe + résonance courte | Hit-stop 30 ms |
| Hors rythme | Impact standard | Frappe sèche | — |
| Palier de combo | Chiffre du combo qui pulse | Le djembé ajoute une variation (couche musicale supplémentaire) | Léger zoom (−5° FOV, 0,3 s) |
| Télégraphe ennemi | Éclat sur l'arme ennemie | Sifflement court | — |

---

## 6. Le griot dans la slice

> Fiche détaillée à produire : `design/mecaniques/griot.md`. Implémentation : `ingenieur-ia-pnj` (backend Laravel).

### 6.1 Deux rôles distincts
| | **Griot narrateur** (voix off) | **Griot PNJ** (dans la ruelle, 1324) |
|---|---|---|
| Époque | Des siècles plus tard | 1324, un griot de Djenné 🔶 inspiré de (assumé) : Djenné n'est pas une ville mandingue (Nono/soninké, Bozo, Songhay) ; les jeli y sont plausibles **au service des marchands mandingues ou de l'émissaire du Mali**. Les Soninké et les Songhay ont leurs propres bardes (*gesere*, *jesere*). Le griot du jeu est un jeli mandingue venu avec ces réseaux (cf. `djenne-1324.md`, §2) |
| Technologie | **100 % scripté**, lignes dans `DT_NarrateurLignes`, voix pré-générées (ElevenLabs) ou texte seul | **API Claude** via backend Laravel, réponse texte affichée en sous-titre |
| Déclencheurs | Volumes de déclenchement + événements (entrée de zone, combat, fin) | Le joueur engage la conversation |
| Hors-ligne | Fonctionne toujours | Fallback scripté (§6.3) |

Statut des griots (*jeli*, caste de la parole, généalogistes, conseillers) : fait solide dans les grandes lignes, quant au ton de leur relation à une jeune chasseuse, 🔶 inspiré de (assumé) : le jeli appartient aux *nyamakala*, il est à la fois respecté et craint ; il peut parler librement, taquiner, rappeler la lignée de son interlocuteur et attendre un présent en échange de sa parole. Les donso ont aussi leurs propres bardes (*sora*). Il ne faut pas parler de « caste » au sens indien, ni décrire un serviteur (cf. `glossaire.md`, §1 et §4).

### 6.2 Exemples de lignes du narrateur (à réécrire après validation)
- Intro : « Écoute. Ce que je vais te dire, mon père le tenait de son père. C'était l'année où le Mansa partit vers la ville sainte… »
- Sira chasseuse (intro ou première apparition) : « Les anciens disent qu'aucune femme n'entrait chez les donso. Ils le disent encore, et ils ont raison de le dire. Mais Sira y est entrée, et c'est pour cela qu'on chante encore son nom. » 🔶 inspiré de (assumé) : la réplique respecte la tradition, qui est énoncée comme règle et non moquée, et présente Sira comme une exception de l'épopée. Les femmes ne sont pas absentes du monde des chasseurs : Sanènè, patronne de la chasse, est une figure féminine (cf. `glossaire.md`, §3).
- Sira chasseuse, écho à la femme-buffle du Do 🔶 inspiré de (assumé) (épopée de Soundiata, tradition orale ; cf. `glossaire.md`, §2). Version corrigée par historien-mande : « Souviens-toi de la femme-buffle du Do : les chasseurs ne l'ont vaincue que parce qu'elle leur a livré son secret. Depuis, ceux qui ont des oreilles savent ce que vaut le *nyama* d'une femme. Sira, elle, portait une lance. » Pourquoi cette correction : l'ancienne formule (« les chasseurs savent qu'une femme peut porter plus de *nyama* que dix hommes ») attribuait aux donso un dicton inventé, et effaçait le fait que dans l'épopée, la femme-buffle est l'**adversaire** des chasseurs. Variante chez le griot PNJ, qui taquine sans moquer : « Une chasseuse ? Les donso de ce pays auraient froncé les sourcils. Moi, je chante ce qui est rare. » (corrigé : « les donso de mon père » ne convenait pas, car le père d'un jeli est un jeli). ⚠️ À faire relire par un connaisseur des donso avant enregistrement.
- Correction (si le joueur saute d'un toit au lieu de grimper) : « Non, non… ce n'est pas comme ça que ça s'est passé. Sira n'était pas un oiseau. Elle a grimpé, prise après prise. » → le jeu ramène au checkpoint (habillage narratif du « rattrapage »).
- Correction (si le joueur reste longtemps au marché) : « Certains disent qu'elle s'attarda au marché. Ceux-là n'ont jamais connu Sira. »
- Place (plan « wow ») : « La mosquée que vous connaissez fut rebâtie bien plus tard. Celle-ci, je la raconte comme mes pères me l'ont racontée. »
- Refus au pied de la mosquée (scripté, sur le chemin critique) : Sira, à voix basse : « Pas par là. Pas sur la maison de Dieu. » Narrateur : « Le chemin le plus court passait par les toron de la mosquée. Sira ne le prit pas. Les maçons y montent chaque année pour la soigner ; on n'y monte pas pour la course d'un voleur, ni pour verser le sang. Elle prit la maison du marchand. » 🔶 inspiré de (assumé), corrigé par historien-mande : à Djenné, monter sur la mosquée **n'est pas tabou en soi**, puisque les maçons et les habitants la gravissent pour le crépissage (pratique attestée aujourd'hui ; un entretien régulier est certain pour 1324). Le refus doit donc porter sur l'**usage** (poursuite, violence), et non sur le simple fait de grimper. Cela évite aussi de contredire le pilier saisonnier du crépissage (cf. `djenne-1324.md`, §7). ⚠️ À faire relire par un lecteur musulman, idéalement malien.
- Combat : « Et le tambour de la place se mit à parler, et Sira l'écouta. »
- Combat, djembé assumé (une fois, au premier temps du djembé) : « Un djembé ? En ce temps-là ? Peut-être pas… Mais je raconte avec les tambours de mon temps, et celui-là sait parler. Écoute-le. »
- Fin (voleur épargné / non épargné) : deux variantes de 1 phrase.

### 6.3 Griot PNJ : règles de conversation
- **Interface** : 3 questions suggérées (cliquables, garanties de marcher hors-ligne) + saisie de texte libre (clavier). Pas de voix du joueur (Whisper hors slice).
- **Réponses** : 1 à 3 phrases, **max 60 mots** (limite côté backend : `max_tokens` + consigne de prompt), en français.
- **Contexte envoyé au backend** : identifiant du PNJ, étape de la quête, historique des 6 derniers échanges. **Jamais la clé API côté client** : le jeu appelle uniquement l'endpoint Laravel.
- **Garde-fous (prompt système + filtres backend)** :
  - Reste en personnage : un griot de 1324, ne connaît pas le monde moderne, ne parle pas de « jeu », d'« IA » ni de « Claude ».
  - Ramène toujours vers l'objectif de la slice (le voleur, la maison du marchand) après 2 questions hors sujet.
  - Si le joueur parle d'escalader la mosquée, le griot l'en dissuade, dans son rôle et avec respect (« On ne monte sur la maison de Dieu que pour la soigner, au temps du crépissage. Pour courir après un voleur, le marchand a des murs solides. » 🔶 corrigé par historien-mande, cf. §6.2). Il ne présente jamais la mosquée comme un parcours.
  - Il sait que Sira est une chasseuse, chose rare chez les donso : il peut le relever (curiosité, respect, taquinerie légère de jeli), jamais le moquer ni le banaliser.
  - Refuse poliment les sujets inappropriés, en restant dans le rôle (« Ces paroles-là ne se disent pas sur la place du marché. »).
  - N'invente pas de faits historiques précis : la fiche de connaissances du griot (`backend/…/griot_djenne.json`, contenu validé par l'historien) est la seule source de faits.
- **Latence** : cible **< 2,5 s** jusqu'au premier texte affiché (streaming si possible), délai max **6 s** puis fallback.
- **Masquage de la latence** : animation « le griot réfléchit » : **il accorde son ngoni** (luth à cordes pincées ; ✅ décidé, §12, Q11 ; forme exacte en 1324 inconnue, cf. `glossaire.md`, §4) + son d'ambiance. La **kora est exclue pour 1324** (anachronisme très probable) ; elle reste éventuellement l'instrument du **narrateur** des siècles plus tard.
- **Fallback hors-ligne** : si erreur réseau, timeout ou réponse filtrée → réplique tirée de `DT_GriotFallback` (au moins 15 répliques : 3 par question suggérée + 6 génériques). Le joueur ne voit jamais de message d'erreur technique.

### 6.4 Le seul « choix » de la slice
Après le combat, l'action Interagir sur le voleur à terre propose : **Épargner** / **Prendre l'indice et le laisser**. La variable `bVoleurEpargne` est enregistrée et ne change que la phrase finale du narrateur. Objectif : prouver techniquement que le narrateur réagit aux choix, sans arbre narratif.

---

## 7. Level design : ruelle, place et maison du marchand

> Fiche détaillée à produire : `design/niveaux/djenne-slice.md`.

### 7.1 Zones
| Zone | Taille approx. | Fonction |
|---|---|---|
| **A. Ruelle** | 60 × 4-6 m, sinueuse | Tutoriel déplacement, ambiance, 1 muret, 1 passage étroit |
| **B. Coin du griot** | Placette 10 × 10 m | Conversation, repos |
| **C. Place du marché** | 40 × 30 m | Plan « wow » sur la mosquée, foule légère (5-8 PNJ animés en boucle), point de refus au pied de la mosquée **[R]**. **Mosquée = décor, non escaladable** (aucune prise active), visible depuis C et depuis E. 🔶 inspiré de (assumé) : on ignore les dimensions de l'édifice de 1324. La mosquée du jeu **ne reprend pas** la silhouette de 1907 (trois tours, œufs d'autruche) ; elle est plus massive, avec une tour principale (~20 m comme hauteur maximale de la tour, pas comme celle d'un mur continu), sur un tertre (cf. `djenne-1324.md`, §5) |
| **D. Façade de la maison du marchand** | 3 sections D1-D3 (voir §7.3). Grande maison à étage en banco et briques rondes modelées (*djenné-ferey*), toron, gouttières saillantes, acrotère à pinacles. Façade décorée **sobrement** 🔶 inspiré de (assumé) : pilastres simples, porte basse en bois épais, petites ouvertures ; pas de façade « toucouleur » ou « marocaine » détaillée, style documenté aux XIXe-XXe s. (cf. `djenne-1324.md`, §6) | Escalade, 3 sections |
| **E. Terrasse du marchand** | 15 × 12 m | Arène de combat, puis vue sur le fleuve et sur la mosquée en toile de fond |

### 7.2 Croquis (vue de dessus, non à l'échelle)
```
                         N (fleuve au loin, visible depuis E)
  +--------------------------+   +-----------------------------------+
  | MOSQUÉE (toile de fond)  |   | E. TERRASSE DU MARCHAND (arène)   |
  |  [T] tour principale     |   |   [x]      S1          [x]        |
  |  contreforts, toron      |   |        S2 (voleur, brute)         |
  |  AUCUNE prise active     |   |   [x]                     [x]     |
  |                          |   | ====== corniche / acrotère ====== |
  |  (vue depuis C et E)     |   | D3: toron + gouttière | D3': saut |
  |                          |   | D2: banco relief + 1 BancoFragile |
  |        tertre            |   | D1: toron bas, mantle sur socle   |
  +-----------[R]------------+   +=====FAÇADE DU MARCHAND (D)========+
  |                                            ^                     |
  |   [R] refus de Sira    C. PLACE DU MARCHÉ  |    [J] djembé       |
  |   [étal] [étal]  (plan wow vers la mosquée)      [étal]          |
  |                          ^                                       |
  +---------------+          |           +---------------------------+
                  |   B. GRIOT [G]       |
                  +-------+    +---------+
                          |    |  <- muret (enjambement)
                          | A. |
                          |RUE-|   <- passage étroit (faufiler)
                          |LLE |
                          |    |
                          [SPAWN]  S
```
Légende : [T] tour, [x] obstacle bas, [R] point de refus (trigger scripté), [J] joueur de djembé (visible depuis E), [G] griot PNJ, S1/S2 positions de départ des ennemis.

### 7.3 Chemin critique et escalade
1. **Spawn** (bas de la ruelle) → muret → passage étroit → **[G] griot** (dialogue requis pour déclencher l'objectif, 1 min max même si le joueur ne pose aucune question).
2. Place du marché → déclenchement du plan « wow » sur la mosquée à l'entrée de C.
3. **[R] Pied de la mosquée** : le chemin critique longe la mosquée, dont les toron semblent le chemin le plus court vers les toits. Volume de déclenchement (`TV_RefusMosquee`, une seule fois) : courte séquence scriptée, skippable, avec la réplique de Sira et le commentaire du narrateur (§6.2), puis la caméra s'oriente vers la façade du marchand. La mosquée n'a **aucun** `BP_PriseEscalade` et aucun prompt « Grimper ».
4. **D1** (5 m) : toron régulièrement espacés tous les 100-120 cm, apprentissage. Checkpoint au pied de la maison du marchand.
5. **D2** (7 m) : alternance banco relief / toron / gouttière, 1 `BancoFragile` au milieu, avec un toron de secours latéral à 150 cm. Checkpoint sur la corniche intermédiaire.
6. **D3** (8 m) : choix entre la colonne de toron et de gouttières (sûre) et le saut entre contreforts (plus rapide, plus spectaculaire). Les deux mènent à l'acrotère.
7. **Mantle** sur la terrasse du marchand → déclenchement du combat (portes de sortie fermées par la mise en scène : les ennemis bloquent l'accès).

Règle : le chemin à suivre doit se lire **par la lumière** (soleil rasant de l'aube sur les toron) et par la forme, pas par de la peinture jaune.

⚠️ **Point ouvert de level design** : les hauteurs D1-D3 (5 + 7 + 8 = 20 m) ont été pensées pour la mosquée. Une maison de Djenné fait 1 à 2 étages et « seule la mosquée domine » (`djenne-1324.md`, §6). À trancher en greybox : soit une maison plus haute que la normale (exception assumée, mais elle concurrencerait la mosquée), soit un parcours rallongé à l'horizontale (traversées, annexe plus basse, maison voisine) pour une hauteur réelle d'environ 8-10 m. L'avis historique (dont la piste « maison au bord de la butte ») est en §12, Q12.

✅ décidé (§12, Q2) : **hybride**. Escalader une mosquée est un acte sensible. On en fait donc un moment de caractère : la poursuite passe au pied de la mosquée, Sira **refuse** de la gravir, et le narrateur souligne ce choix ; l'escalade et le combat ont lieu sur la grande maison du marchand voisine (banco, toron, gouttières : même vocabulaire d'escalade). La mosquée garde son rôle de repère et de plan « wow », depuis la place et depuis la terrasse. Recommandation maintenue : faire relire la scène du refus par au moins un lecteur musulman, idéalement malien.

### 7.4 Arène (zone E, terrasse du marchand)
- 15 × 12 m, sol plat de terre damée, 3-4 obstacles bas (jarres, bois, nattes roulées) pour casser les lignes sans bloquer la caméra.
- Bordures : parapets / acrotère de 1 m (pas de chute possible).
- Le joueur de djembé est **visible** en contrebas sur la place (lien diégétique son/image).
- La mosquée est visible en toile de fond depuis l'arène : la cadrer dans le plan d'ouverture du combat et dans le plan de fin.

---

## 8. Direction artistique et audio (slice)

### 8.1 DA
- **Palette** : ocre et brun du banco, bleu indigo et blanc des tissus, ombres bleutées de l'aube, or chaud du soleil rasant. 🔶 inspiré de (assumé) : le coton tissé en bandes étroites est attesté (textiles tellem, XIe s.) ; l'indigo est plausible ; le bogolan relève de l'hypothèse pour 1324, à n'utiliser qu'avec parcimonie (cf. `glossaire.md`, §6).
- **Matériaux clés** (Substance / Megascans) : `M_Banco` (murs, avec variation d'érosion par vertex paint), `M_Toron` (bois de palmier, toron et gouttières), `M_SolTerreBattue`.
- **Bâtiments clés** : la **maison du marchand** (escaladable, détail élevé, lisible de près) et la **mosquée** (décor non escaladable, vue de loin depuis la place et la terrasse : silhouette forte, détail moyen, 🔶 silhouette « médiévale imaginée », cf. §7.1).
- **Lumière** : aube, Lumen, soleil bas pour dessiner les reliefs de la façade du marchand et découper la silhouette de la mosquée.
- **Personnages** : Sira (chasseuse donso, armée d'un couteau de chasse et d'une lance courte), griot (avec son ngoni), voleur, brute, 5-8 passants (variantes de couleurs d'un même modèle). Base possible : MetaHuman, avec tenues refaites dans Blender. 🔶 inspiré de (assumé) : le cadre des costumes est fixé dans `djenne-1324.md`, §8 et §10 ; chaque planche de costume est à faire relire par historien-mande avant modélisation.
- Style : réaliste stylisé, pas photoréaliste (plus atteignable pour un développeur seul).

### 8.2 Audio (FMOD)
- **Ambiance** : marché qui s'éveille, appel à la prière au loin à l'aube ✅ validé : islam établi à Djenné (mosquée fondée selon la tradition au XIIIe s.) ; voix seule, sans amplification, en arabe. Ajouter les récitations des écoles coraniques et les pilons (cf. `djenne-1324.md`, §4 et §9), oiseaux, fleuve.
- **Musique** : quasi absente en exploration (le son de la ville suffit) ; en escalade, nappe discrète ; en combat, **djembé** (assumé par le narrateur, cf. §5.1) à 96 BPM + couches qui s'ajoutent selon le combo (paramètre FMOD `Combo` 0-3).
- **Voix** : narrateur pré-enregistré ou TTS pré-généré (jamais en temps réel dans la slice) ; griot PNJ en texte seul, TTS optionnel si le temps le permet.
- Le paramètre FMOD `Tempo` est exposé mais fixe dans la slice.

---

## 9. UI / HUD minimal
| Élément | Quand | Description |
|---|---|---|
| Prompts contextuels | Exploration, escalade | Icône de touche + verbe (« Grimper »), apparaît à 2 m |
| Barre de vie de Sira | Combat seulement | Fine, en bas à gauche, se masque 3 s après le combat |
| Anneau de pulsation | Combat seulement | Sous les pieds de Sira, pulse sur chaque temps ; désactivable |
| Compteur de combo | Combat, dès 2 | Chiffre + qualité du dernier coup (« Parfait », « En rythme ») |
| Sous-titres | Toujours | Narrateur et griot PNJ, activés par défaut |
| Fenêtre de dialogue griot | Conversation | 3 questions suggérées + champ texte + indicateur « le griot réfléchit » |
| Menu pause | — | Reprendre, Options (sensibilité, aide au rythme, compensation latence, sous-titres, liseré des prises), Quitter |

Pas de mini-carte, pas de boussole, pas de marqueur d'objectif permanent : le griot et la lumière guident.

---

## 10. Hors scope de la slice (liste explicite)
Tout ce qui suit va dans `design/idees-plus-tard.md` si quelqu'un le propose pendant la Phase 1 :
- Nyama (jauge, gain, corruption) — Phase 2.
- Fasa / chants comme clés — Phase 2.
- Saisons, crue du Niger, crépissage dynamique — Phase 2.
- Arc, chasse, animaux.
- Infiltration, discrétion, assassinat.
- Monde ouvert, parkour libre sur toits, escalade sur n'importe quelle surface.
- Endurance d'escalade.
- Plus de 2 ennemis, boss, IA de groupe avancée.
- Arbre de compétences, inventaire, équipement, économie.
- Variation de tempo du djembé en combat.
- Ensemble tambour de place + pilons + balafon tenant le tempo du combat — à étudier en Phase 2.
- Escalade de la mosquée (exclue par design, pas seulement par scope : cf. §7.3).
- Reconnaissance vocale (Whisper), TTS en temps réel.
- Mémoire persistante du griot entre sessions.
- Cinématiques complexes (au-delà de caméras scriptées simples).
- Niani, Tombouctou, caravane, Bambouk, océan — Phases 3-4.
- Localisation autre que le français (structure prête : textes en Data Tables / String Tables).
- Consoles.

---

## 11. Definition of Done de la slice
La slice est terminée quand **toutes** ces conditions sont vraies :

**Contenu**
- [ ] Les 5 zones (A-E) sont jouables de bout en bout sans blocage.
- [ ] La scène du refus au pied de la mosquée se déclenche à chaque partie sur le chemin critique, et la mosquée n'a aucune prise active (0 `BP_PriseEscalade`, aucun prompt « Grimper »).
- [ ] La mosquée est visible depuis la place (plan « wow ») et depuis la terrasse du marchand (combat et fin).
- [ ] Durée mesurée sur 3 testeurs n'ayant jamais joué : entre **10 et 15 min**.

**Déplacement / escalade**
- [ ] Le chemin critique d'escalade sur la façade du marchand (D1-D3) se termine sans aide extérieure par 3 testeurs sur 3.
- [ ] 0 cas de personnage coincé dans la géométrie sur 10 parties.

**Combat**
- [ ] Combat contre 2 ennemis gagnable avec l'option « Rythme désactivé ».
- [ ] Dérive entre beat FMOD et détection de rythme < **10 ms** après 3 min (mesurée par log).
- [ ] Au moins 2 testeurs sur 3 disent spontanément ou sur question qu'ils « jouaient avec le tambour ».
- [ ] Taux de coups « en rythme » des testeurs > 40 % en fin de combat (log).

**Griot**
- [ ] Le PNJ griot répond via le backend Laravel en < 2,5 s dans 80 % des cas (log, réseau domestique).
- [ ] Coupure réseau simulée : le fallback se déclenche en ≤ 6 s, aucun message d'erreur affiché.
- [ ] Sur 20 questions pièges (monde moderne, IA, sujets inappropriés), le griot reste dans son rôle dans au moins 19 cas.
- [ ] Au moins 1 correction du narrateur et la variante de fin (`bVoleurEpargne`) fonctionnent.
- [ ] La clé API n'apparaît nulle part dans le build client (vérification par recherche de chaîne).

**Technique**
- [ ] 60 FPS stables en 1080p sur la machine de développement (paramètres « Élevé »).
- [ ] Toutes les valeurs de gameplay listées dans ce GDD sont dans des Data Tables / JSON, aucune en dur.
- [ ] Build packagé Windows lancé sur une 2e machine.

**Culture**
- [ ] Tous les points ⚠️ de ce document sont soit validés par `historien-mande`, soit remplacés par une option « inspirée de » assumée et notée.
- [ ] La scène du refus de la mosquée et les répliques sur Sira chasseuse ont été relues par `historien-mande` et, si possible, par un lecteur musulman malien et un connaisseur des donso.

---

## 12. Questions ouvertes
1. **Djembé en 1324** : ✅ décidé (2026-11-23) : on garde le djembé, assumé par le narrateur qui « raconte avec les tambours de son temps » (réplique en §6.2) ; l'ensemble tambour de place + pilons + balafon est à étudier en Phase 2.
2. **Mosquée et lieu de l'escalade** : ✅ décidé (2026-11-23) : hybride : escalade et combat sur la grande maison de marchand voisine de la place ; la poursuite passe au pied de la mosquée, que Sira refuse de gravir ; mosquée en toile de fond (plan « wow »), 🔶 inspiré de (assumé) : silhouette de 1907 non reprise (cf. `djenne-1324.md`, §5).
3. **Statut politique de Djenné en 1324** : ✅ validé (cadre) : les sources divergent, et la réponse est assumée. Djenné est une **cité autonome**, avec son *koy* et ses notables marchands, dans l'orbite du Mali et sous sa pression ; un émissaire du mansa y réside ; Moussa est absent (pèlerinage). Le griot PNJ peut dire « on dit que Djenné ne s'est jamais soumise » (tradition, Tarikh es-Sudan). Voir `djenne-1324.md`, §1.
4. **Sira** : ✅ décidé (2026-11-23) : chasseuse, prénom Sira, exception commentée par le griot (écho à la femme-buffle du Do, 🔶 inspiré de (assumé), épopée de Soundiata), couteau de chasse + lance courte.
5. **Détection du rythme** : comparer le moment de l'impact (plus juste) ou celui de l'appui (plus simple) ? À trancher en prototype, valeur par défaut : l'appui si le dev manque de temps.
6. **Voix du narrateur** : comédien enregistré ou TTS ElevenLabs pour la slice ? Question de budget et d'authenticité (accent, langue).
7. **Langue des dialogues** : tout en français ou quelques mots en mandingue (salutations) sous-titrés ? Nécessite un validateur locuteur.
8. **Relique volée** : quelle nature ? Doit être validée culturellement avant d'apparaître même par allusion dans la slice. Note historien-mande : éviter un objet sacré de l'islam réel (Coran précis, relique du Prophète) ou un *boli* réel ; préférer un objet inventé, lié au pèlerinage (présent destiné au Caire ou à La Mecque).
9. **Indice de fin de slice** (fragment de pirogue gravé ?) : 🔶 inspiré de (assumé) : cohérent avec l'artisanat de l'époque (pirogues et bois sculpté plausibles). Les « navires » d'Aboubakari II restent un récit rapporté par al-Umari : l'indice doit suggérer une rumeur, pas une preuve.
10. **Coûts API** : budget mensuel de tests du griot PNJ, et limite de requêtes par session côté Laravel.
11. **Instrument du griot PNJ** : ✅ décidé (2026-11-23) : luth de type ngoni (il l'accorde pendant l'animation « le griot réfléchit ») ; kora exclue pour 1324, éventuellement réservée au narrateur des siècles plus tard.
12. **Hauteur de la maison du marchand** : ⚠️ ouvert (level design). Les sections D1-D3 totalisent 20 m, alors qu'une maison de Djenné fait 1 à 2 étages et que seule la mosquée domine. À trancher en greybox (cf. §7.3).
   - **Avis historique (historien-mande, sans trancher)** :
     - Aucune maison de 1324 n'a été conservée ni décrite. Les références sont les maisons de Djenné des XIXe-XXe s. et la logique du banco (hypothèse).
     - Une grande maison de marchand compte un rez-de-chaussée et un étage, parfois une pièce sur la terrasse. Façade avec acrotère et pinacles : environ **7 à 10 m** (hypothèse) ; un étage de banco porteur fait environ 2,5 à 3,5 m.
     - 3 niveaux pleins : exception peu plausible pour le XIVe s. 20 m : invention lourde, qui ferait de la maison un rival de la mosquée et contredirait le récit du refus.
     - **Piste légitime** : Djenné est bâtie sur une butte au-dessus de la plaine inondable. Une maison au **bord de la butte**, avec une façade arrière qui donne sur un mur de soutènement en contrebas (berge, quai à pirogues), peut offrir **12-14 m de verticalité** sans être anormalement haute : 4-5 m de soutènement + 8-9 m de maison (🔶 inspiré de, assumé). Cela donnerait aussi une vue sur le fleuve.
     - Autres pistes : enchaîner des maisons mitoyennes (on passe de terrasse en terrasse), ou une annexe plus basse (D1) avant le corps principal.

---

## Historique des versions
| Version | Date | Auteur | Changements |
|---|---|---|---|
| v1 | 2026-11-23 | directeur-creatif | Première version, centrée vertical slice Djenné |
| v1.1 | 2026-11-23 | historien-mande | Relecture historique : points ⚠️ validés ou passés en « inspiré de » ; arbitrages de design laissés ouverts avec options (§12, Q1, Q2, Q4, Q11) ; Aboubakari II présenté comme légende ; kora exclue pour 1324 |
| v1.2 | 2026-11-23 | directeur-creatif | Décisions du développeur (§12) : Q1 djembé gardé et assumé par le narrateur (ensemble tambour + pilons + balafon reporté en Phase 2) ; Q2 hybride : escalade et combat sur une grande maison de marchand, refus de Sira au pied de la mosquée, mosquée en toile de fond (§3, §4.4, §6, §7, §8, §10, §11 mis à jour) ; Q4 Sira chasseuse, exception commentée par le griot en écho à la femme-buffle du Do (🔶), armes couteau de chasse + lance courte (bâton supprimé), texte accordé au féminin ; Q11 ngoni pour le griot PNJ ; nouvelle Q12 (hauteur de la maison du marchand) |
| v1.2-r | 2026-11-23 | historien-mande | Relecture v1.2 : femme-buffle du Do recadrée (adversaire puis alliée des chasseurs, pas « chasseuse ») et réplique corrigée ; refus de la mosquée reformulé (les maçons y montent pour le crépissage : le refus porte sur la poursuite et la violence) ; avis historique sur Q12 ; restent ⚠️ uniquement les relectures extérieures (locuteur, connaisseur des donso, lecteur musulman malien) et Q12 |
