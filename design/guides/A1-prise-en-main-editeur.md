# Guide A1 — Prendre en main l'éditeur Unreal

> **Tâche** : A1 du plan de la Phase 1 (`/design/plan-phase1.md`), environ 2 h.
> **Projet** : `C:\Dev\Mansa\Mansa\Mansa.uproject` (UE 5.8, C++, template Third Person).
> **Critère de réussite** : tu sais naviguer dans l'éditeur, ouvrir et sauver une map, lancer Play, te servir de l'Outliner, du panneau Details et du Content Browser, puis créer un niveau vide, y placer un cube et le sauver dans `Content/Mansa/Maps/`.
>
> On ne code pas en A1. On apprend juste à se déplacer dans l'outil.

---

## Règles de survie (à garder en tête tout le temps)

1. **Sauve souvent.** `Ctrl+S` sauve le **niveau courant**. `Ctrl+Maj+S` (**Save All**) sauve **tout** ce qui a été modifié (niveau, Blueprints, matériaux…). Un asset modifié et pas encore sauvé porte un **astérisque `*`** sur son onglet ou sa vignette.
2. **Ne touche jamais aux fichiers du projet depuis l'extérieur pendant que l'éditeur est ouvert** : pas de déplacement dans l'Explorateur Windows, pas de `git checkout` / `git pull` / `git stash` sur `Content/`. Ferme l'éditeur d'abord. L'éditeur garde les assets en mémoire : si tu les modifies en douce, au mieux il ne voit rien, au pire il écrase ton travail ou casse des références.
3. **Live Coding ou recompiler ?** (juste pour info, on ne code pas aujourd'hui) Quand tu modifieras du C++ plus tard, **Live Coding** (`Ctrl+Alt+F11`, ou le bouton en bas à droite de l'éditeur) recompile à chaud. C'est bien pour retoucher le corps d'une fonction. Dès que tu modifies un **header** (nouvelle `UPROPERTY`, nouvelle classe, nouvelle `UFUNCTION`), ferme l'éditeur, compile depuis Visual Studio, puis rouvre. On reverra ça en A3.
4. **Unreal est en anglais** dans ce guide. Les noms de menus sont donnés en anglais, avec parfois la traduction entre parenthèses. Selon la version, certains libellés peuvent légèrement changer : je le signale quand j'ai un doute.

---

### Étape 1 — Ouvrir le projet et comprendre l'écran (10 min)

**But** : ouvrir Mansa dans l'éditeur et savoir comment s'appelle chaque zone de l'écran.

**Fais ceci** :
1. Dans l'Explorateur Windows, va dans `C:\Dev\Mansa\Mansa\` et double-clique sur **`Mansa.uproject`**. Autre solution : **Epic Games Launcher** > **Unreal Engine** > onglet **Library** > section **My Projects** > double-clic sur **Mansa**.
2. Si une fenêtre te dit que des modules sont manquants ou doivent être recompilés (« *The following modules are missing or built with a different engine version… Would you like to rebuild them now?* »), réponds **Yes**. Le premier lancement peut prendre plusieurs minutes (compilation des shaders, barre de progression en bas à droite).
3. L'éditeur s'ouvre sur la map du template, **`Lvl_ThirdPerson`**. Repère les zones suivantes :
   - **Menu Bar** (en haut) : `File`, `Edit`, `Window`, `Tools`, `Build`, `Select`, `Actor`, `Help`.
   - **Main Toolbar** (juste en dessous) : bouton **Save**, bouton **Quickly add to the project** (icône cube avec un `+`), et au centre le bouton **Play** (triangle vert).
   - **Viewport** (grande zone centrale) : la vue 3D du niveau.
   - **Outliner** (en haut à droite) : la liste de tous les objets (« Actors ») du niveau.
   - **Details** (en bas à droite) : les propriétés de l'objet sélectionné.
   - **Content Drawer** (bouton en bas à gauche, ou `Ctrl+Espace`) : le navigateur de fichiers du projet. On le verra à l'étape 6.
4. Si tu as dérangé la disposition des panneaux : **Window** > **Load Layout** > **Default Editor Layout**.

**Tu dois voir** : un décor gris-blanc fait de blocs et de rampes (le niveau de démo du template), l'Outliner rempli d'objets à droite, et aucun message d'erreur bloquant.

**Si ça coince** :
- *« Mansa could not be compiled. Try rebuilding from source manually. »* : ouvre `C:\Dev\Mansa\Mansa\Mansa.sln` dans Visual Studio, choisis la configuration **Development Editor** / **Win64**, fais **Build** > **Build Solution**, puis relance le `.uproject`. Si la compilation échoue, copie-moi l'erreur.
- L'éditeur rame énormément au premier lancement : c'est la compilation des shaders, attends qu'elle se termine (compteur en bas à droite).
- Un panneau a disparu : il se rouvre presque toujours via le menu **Window** (par exemple **Window** > **Outliner**, **Window** > **Details**).

**Analogie dev** : l'éditeur, c'est ton IDE. Le niveau (la *map*), c'est le fichier ouvert. L'Outliner, c'est l'arbre DOM de la page. Details, c'est l'inspecteur des DevTools.

---

### Étape 2 — Régler le clavier AZERTY et naviguer dans le viewport (15 min)

**But** : te déplacer librement dans la scène 3D, avec des touches qui tombent bien sous les doigts sur un clavier AZERTY.

**Le piège AZERTY, en clair** : sous Windows, Unreal lit les touches selon le **caractère** qu'elles produisent, pas selon leur position physique. Le vol de caméra est réglé par défaut sur **W/A/S/D** (avancer, gauche, reculer, droite) et **Q/E** (descendre, monter). Sur ton AZERTY, il faut donc appuyer sur la touche marquée **W** (en bas à gauche) pour avancer et sur la touche marquée **A** (en haut à gauche) pour aller à gauche. C'est injouable. On va remapper. Le comportement exact peut varier selon la version : commence par le test du point 1.

**Fais ceci** :
1. **Test** : clique dans le viewport, maintiens le **clic droit** et appuie sur la touche **Z**. Si la caméra avance, ta version gère déjà l'AZERTY : saute au point 4. Si rien ne se passe et que c'est **W** qui fait avancer, continue.
2. Ouvre **Edit** > **Editor Preferences…** > section **General** > **Keyboard Shortcuts**. Dans la barre de recherche, tape `Viewport Navigation`, ou cherche directement `Forward`, `Left`, etc. (le nom de la catégorie peut varier légèrement).
3. Modifie les raccourcis **dans cet ordre** pour éviter les conflits (le `Q` est déjà utilisé par « Down ») :
   - **Down** : `A`
   - **Left** : `Q`
   - **Forward** : `Z`
   - **Backward** : `S` (déjà bon)
   - **Right** : `D` (déjà bon)
   - **Up** : `E` (déjà bon)

   Pour changer une touche, clique dans le champ du raccourci, appuie sur la nouvelle touche, puis valide. Si l'éditeur signale un conflit, lis le nom de la commande en conflit et dis-moi si c'est autre chose que la navigation. Les réglages sont sauvés automatiquement : ferme la fenêtre.
4. **Navigation** (toujours avec la souris dans le viewport) :
   - **Clic droit maintenu + souris** : regarder autour de toi.
   - **Clic droit maintenu + Z/Q/S/D** : voler (avancer, gauche, reculer, droite). **E/A** : monter et descendre.
   - **Clic droit maintenu + molette** : changer la **vitesse de vol** pendant que tu voles.
   - **Molette seule** : avancer ou reculer par crans.
   - **Clic du milieu maintenu** : déplacer la vue latéralement (*pan*).
   - **Alt + clic gauche** : tourner en orbite autour du point visé. **Alt + clic droit** : zoomer.
5. **Focus** : clique sur un objet dans le viewport, puis appuie sur **F**. La caméra se centre sur l'objet. C'est le geste que tu utiliseras le plus pour ne pas te perdre.
6. **Vitesse de caméra** : en haut à droite du viewport, une icône de caméra accompagnée d'un chiffre (de 1 à 8) règle la vitesse de base. Le libellé et la position peuvent varier selon la version : passe la souris sur les icônes de la barre du viewport pour trouver l'infobulle *Camera Speed*.
7. Bonus : **G** bascule en **Game View** (cache les icônes d'éditeur). Rappuie sur **G** pour revenir.

**Tu dois voir** : tu voles dans le niveau avec clic droit + ZQSD, tu sais te recentrer avec **F** sur n'importe quel objet, et tu sais ralentir ou accélérer.

**Si ça coince** :
- Aucune touche ne fait voler la caméra : le **clic droit doit rester enfoncé** pendant que tu appuies sur les touches, et la souris doit être dans le viewport.
- Tu es perdu dans le vide : sélectionne un objet dans l'**Outliner**, survole le viewport et appuie sur **F**.
- Solution de secours sans remapper : `Win+Espace` bascule Windows en clavier anglais (US). Attention, tout ce que tu tapes ensuite sera en QWERTY, y compris les noms d'assets. Le remapping est plus confortable.

**Analogie dev** : comme rebinder les raccourcis de ton IDE (Keymap dans IntelliJ, keybindings dans VS Code).

---

### Étape 3 — Play, Simulate, arrêter, et le piège de la souris capturée (10 min)

**But** : lancer le jeu dans l'éditeur, en sortir proprement, et récupérer ta souris quand le jeu la « capture ».

**Fais ceci** :
1. Clique sur **Play** (triangle vert de la Main Toolbar) ou appuie sur **Alt+P**. Le jeu démarre dans le viewport : c'est le **PIE** (*Play In Editor*).
2. Contrôle le personnage. **Attention, AZERTY** : les contrôles du jeu du template sont réglés sur les touches **W/A/S/D**, lues comme caractères, comme pour la caméra. Sur ton clavier, il faut donc utiliser les touches marquées W, A, S, D, ou une **manette** si tu en as une. C'est normal à ce stade : on règle les contrôles AZERTY et QWERTY en tâche **A4**. Espace pour sauter, souris pour la caméra.
3. **Le piège** : en Play, la souris est capturée par le jeu, tu ne peux plus cliquer dans les menus. Appuie sur **Maj+F1** pour libérer le curseur. Pour revenir dans le jeu, reclique dans le viewport.
4. **Arrêter** : appuie sur **Échap** (*Esc*) ou clique sur le bouton **Stop** (carré rouge qui remplace Play pendant le jeu).
5. **Les autres modes** : clique sur les **trois points** (`⋮`) à côté de Play. Tu y trouves notamment :
   - **Selected Viewport** : le mode par défaut, le jeu tourne dans le viewport.
   - **New Editor Window (PIE)** : le jeu dans une fenêtre séparée.
   - **Standalone Game** : un processus séparé, plus proche du jeu final, plus lent à lancer.
   - **Simulate** (**Alt+S**) : le monde tourne (physique, IA), mais **sans** personnage joueur. Tu gardes la caméra libre de l'éditeur.
6. Pendant un Play, appuie sur **F8** (*Eject*) : tu quittes le personnage et retrouves la caméra libre pour inspecter la scène pendant que le jeu tourne. Rappuie sur **F8** pour reprendre le personnage.
7. **Règle d'or** : tout ce que tu modifies **pendant** le Play (déplacer un objet, changer une valeur) est **perdu** quand tu arrêtes. Arrête toujours le jeu avant d'éditer.

**Tu dois voir** : le personnage du template qui court et saute. Après **Maj+F1**, ton curseur est libre. Après **Échap**, retour à l'éditeur normal.

**Si ça coince** :
- Échap ne fait rien : la souris n'est peut-être plus dans le jeu. Fais **Maj+F1**, puis clique sur **Stop** dans la toolbar.
- Le personnage ne bouge pas : vérifie que tu appuies sur les touches *marquées* W/A/S/D (voir point 2), et que le viewport a bien le focus (clique dedans).
- Le jeu se lance mais tu n'as pas de personnage : tu es peut-être en **Simulate**. Repasse en **Selected Viewport** via les trois points.

**Analogie dev** : Play, c'est `ng serve` / `php artisan serve` avec hot state. Mais tout ce qui change pendant l'exécution n'est jamais réécrit dans les sources, comme modifier le DOM dans les DevTools.

---

### Étape 4 — Outliner et Details : sélectionner, renommer, modifier (15 min)

**But** : trouver un objet du niveau, le renommer et changer une de ses propriétés.

**Fais ceci** :
1. Dans l'**Outliner**, clique sur un objet. Il est sélectionné dans le viewport, entouré d'un contour orange. L'inverse marche aussi : un clic dans le viewport sélectionne la ligne dans l'Outliner.
2. Utilise la **barre de recherche** de l'Outliner : tape `Light` pour filtrer les lumières. Efface ensuite la recherche.
3. Repère les **dossiers** de l'Outliner (icônes de dossier). Ils servent à ranger, ils n'ont aucun effet en jeu. Clique sur l'**œil** à gauche d'une ligne pour cacher ou montrer l'objet dans l'éditeur.
4. Sélectionne un bloc du décor (un cube ou une rampe). Appuie sur **F2** (ou clic droit > **Edit** > **Rename**, le libellé peut varier) et renomme-le `MonPremierBloc`. Valide avec Entrée.
5. Regarde le panneau **Details** :
   - En haut, la **barre de recherche** des propriétés. C'est ton meilleur ami : il y a des centaines de propriétés.
   - La section **Transform** : **Location**, **Rotation**, **Scale**.
   - Plus bas, les **composants** de l'objet (par exemple *StaticMeshComponent*) et leurs propriétés : **Static Mesh**, **Materials**, etc.
6. Dans **Transform** > **Location**, change la valeur **Z** (la hauteur) : ajoute `200`. Le bloc monte de 2 mètres. L'unité d'Unreal est le **centimètre**.
7. Clique sur la **flèche jaune « reset »** à droite d'une propriété modifiée pour revenir à la valeur par défaut. Fais **Ctrl+Z** pour annuler le renommage.
8. **Ne sauve pas** ce niveau du template : on ne veut pas le modifier. Si tu as un doute, on rouvrira le niveau sans sauver à l'étape 8.

**Tu dois voir** : l'objet change de nom dans l'Outliner, monte quand tu modifies Location Z, et tout revient après Ctrl+Z ou reset.

**Si ça coince** :
- Le panneau Details est vide : rien n'est sélectionné. Clique un objet.
- La propriété est grisée : elle n'est pas éditable dans ce contexte, par exemple pendant le Play ou sur un objet verrouillé. Arrête le jeu.
- Tu ne retrouves plus l'objet : cherche-le dans l'Outliner, puis survole le viewport et appuie sur **F**.

**Analogie dev** : Outliner ≈ arbre DOM, Details ≈ onglet *Elements/Styles* de l'inspecteur. Un **Actor**, c'est un élément de la page. Ses **Components**, ce sont ses comportements attachés : en gros, de la composition plutôt que de l'héritage.

---

### Étape 5 — Transformer : W/E/R, snapping, dupliquer (10 min)

**But** : déplacer, tourner et redimensionner un objet à la souris, proprement aligné sur la grille.

**Fais ceci** :
1. Sélectionne un bloc. Les modes du *gizmo* (les flèches de manipulation) se changent avec ces touches, **lues comme caractères** :
   - **W** = Translate (déplacer). Sur AZERTY, c'est la touche marquée **W**, en bas à gauche.
   - **E** = Rotate (tourner).
   - **R** = Scale (mettre à l'échelle).
   - **Espace** fait défiler les trois modes. Si les touches te gênent, les mêmes boutons existent en haut à droite du viewport.
2. Tire une flèche du gizmo (rouge = X, vert = Y, bleu = Z) pour déplacer l'objet sur un axe. Tire un carré entre deux flèches pour le déplacer sur un plan.
3. **Snapping** : en haut à droite du viewport, tu trouves trois icônes avec une valeur : grille de déplacement (par exemple `10`), angle de rotation (par exemple `10°`), échelle (par exemple `0.25`). Clique sur l'icône pour activer ou désactiver le snap, et sur la valeur pour la changer. Mets le déplacement à `50` et déplace un bloc : il saute de 50 cm en 50 cm.
4. **Coller au sol** : sélectionne un objet en l'air et appuie sur **Fin** (*End*). Il tombe sur la surface en dessous.
5. **Dupliquer** : **Ctrl+D**, ou maintiens **Alt** pendant que tu tires une flèche du gizmo : une copie part avec la flèche.
6. **World / Local** : l'icône en forme de globe ou de cube, en haut à droite du viewport, bascule les axes du gizmo entre repère du monde et repère de l'objet. Utile après une rotation.
7. Annule tout avec plusieurs **Ctrl+Z**, ou ferme le niveau sans sauver plus tard.

**Tu dois voir** : le bloc se déplace par crans réguliers quand le snap est actif. Fin le pose au sol, Alt+drag crée une copie.

**Si ça coince** :
- W ne passe pas en mode déplacement : sur AZERTY, appuie sur la touche *marquée* W, pas sur Z. Et vérifie que la souris survole le viewport.
- L'objet ne bouge pas du tout : il est peut-être verrouillé (clic droit > **Transform** > **Lock Actor Movement**, libellé variable), ou ce n'est pas lui qui est sélectionné.
- Le snap fait des sauts bizarres : vérifie la valeur de la grille, et si tu es en World ou en Local.

---

### Étape 6 — Content Browser : explorer les assets du template (10 min)

**But** : savoir où sont les fichiers du projet et les retrouver vite.

**Fais ceci** :
1. Ouvre le **Content Drawer** : bouton **Content Drawer** en bas à gauche, ou **Ctrl+Espace**. Il se replie dès que tu cliques ailleurs. Pour qu'il reste affiché, clique sur **Dock in Layout** en haut à droite du tiroir. Autre solution : **Window** > **Content Browser** > **Content Browser 1**.
2. À gauche, l'**arborescence des dossiers**. La racine **All** > **Content** correspond au dossier disque `C:\Dev\Mansa\Mansa\Content\`. Dans Unreal, son chemin interne est `/Game/`.
3. Explore les dossiers du template :
   - `ThirdPerson/` : la map `Lvl_ThirdPerson` et ses Blueprints (personnage, GameMode).
   - `Characters/` : les mannequins (meshes, animations).
   - `Input/` : les actions et mappings Enhanced Input (préfixes `IA_` et `IMC_`).
   - `LevelPrototyping/` : les blocs et matériaux de prototypage.
   - `Variant_Combat/`, `Variant_Platforming/`, `Variant_SideScrolling/` : les variantes du template. On décide de leur sort en A2, **n'y touche pas** aujourd'hui.
   - `Developers/`, `Collections/` : des dossiers techniques de l'éditeur, ignore-les.
4. Dans la zone de droite, **double-clique** sur un asset pour l'ouvrir dans son éditeur dédié (essaie un *Static Mesh* dans `LevelPrototyping/Meshes/`). Ferme-le ensuite.
5. **Filtres** : le bouton **Filters** (ou l'icône d'entonnoir, à gauche de la recherche) filtre par type : *Static Mesh*, *Material*, *Blueprint Class*, *Level*… Choisis **Level** : tu ne vois plus que les maps. Retire le filtre.
6. **Recherche** : la barre de recherche fonctionne dans le dossier courant et ses sous-dossiers. Clique sur `Content` puis tape `Lvl_` pour retrouver les maps.
7. **Raccourci global** : **Ctrl+P** ouvre une recherche d'asset rapide dans tout le projet.
8. Survole un asset sans cliquer : l'infobulle donne son type, son chemin et sa taille.

**Tu dois voir** : tu sais retrouver `Lvl_ThirdPerson` en moins de 10 secondes, par l'arborescence, par le filtre Level ou par Ctrl+P.

**Si ça coince** :
- Le Content Drawer se ferme tout seul : c'est voulu. Clique sur **Dock in Layout** pour le garder.
- Le dossier `__ExternalActors__` visible dans l'Explorateur Windows n'apparaît pas dans l'éditeur : normal, l'éditeur le cache (voir étape 11).
- Tu ne vois pas certains dossiers : regarde les options d'affichage du Content Browser (**Settings**, icône d'engrenage). Des options comme *Show Engine Content* ou *Show Plugin Content* contrôlent ce qui est visible. N'active pas le contenu moteur pour l'instant.

**Analogie dev** : Content Browser ≈ l'explorateur de fichiers de ton IDE, mais **avec une base de références** derrière. Chaque `.uasset` sait qui il référence et qui le référence. Pense à un ORM avec ses clés étrangères, pas à de simples fichiers.

---

### Étape 7 — Redirecteurs, et création de `Content/Mansa/Maps` (10 min)

**But** : comprendre pourquoi on ne touche jamais aux assets depuis Windows, puis créer notre premier dossier.

**Pourquoi tout passe par l'éditeur** : les assets se référencent par **chemin** (par exemple `/Game/ThirdPerson/Lvl_ThirdPerson`). Si tu renommes ou déplaces un asset **dans l'éditeur**, il met à jour les références. Pour les fichiers qu'il ne peut pas réécrire tout de suite, il laisse un **redirecteur** à l'ancien emplacement : un petit fichier `.uasset` qui dit « je suis maintenant là-bas ». Si tu déplaces le fichier **dans l'Explorateur Windows**, personne n'est prévenu : les références cassent, et tu te retrouves avec des matériaux manquants ou des Blueprints qui ne chargent plus.

**Fais ceci** :
1. Dans le Content Browser, clique sur **Content** à la racine.
2. Clic droit dans une zone vide > **New Folder**. Nomme-le `Mansa`.
3. Double-clique sur `Mansa`, puis clic droit > **New Folder** > `Maps`.
4. Résultat : `Content/Mansa/Maps/`, c'est-à-dire `/Game/Mansa/Maps/` dans Unreal.
5. **Pour plus tard, à retenir** : après avoir déplacé ou renommé des assets, fais un clic droit sur le dossier concerné > **Update Redirector References**. Ce nettoyage supprime les redirecteurs devenus inutiles. Dans les anciennes versions, la commande s'appelait **Fix Up Redirectors in Folder**, et le libellé peut encore varier. Pour voir les redirecteurs, utilise **Filters** > **Other Filters** > **Show Redirectors** (l'emplacement peut varier).

**Tu dois voir** : le dossier `Mansa` sous `Content`, et `Maps` à l'intérieur. Note : un dossier vide n'existe que dans l'éditeur tant qu'il ne contient pas d'asset. Il apparaîtra vraiment sur le disque à l'étape 8.

**Si ça coince** :
- Tu ne trouves pas **New Folder** : vérifie que tu fais le clic droit dans la zone de droite (la liste des assets), ou sur un dossier de l'arborescence.
- Faute de frappe dans le nom : clic droit sur le dossier > **Rename** (ou **F2**).

**Analogie dev** : un redirecteur, c'est une **redirection 301** : l'ancienne URL renvoie vers la nouvelle tant que tous les liens n'ont pas été mis à jour. Déplacer un fichier dans l'Explorateur, c'est supprimer une page sans redirection : 404 partout.

---

### Étape 8 — Créer un nouveau niveau et le sauver sous `LV_Djenne_Test` (10 min)

**But** : créer ta propre map et l'enregistrer au bon endroit, avec le bon nom.

**Fais ceci** :
1. **File** > **New Level…** (**Ctrl+N**). Si l'éditeur demande de sauver `Lvl_ThirdPerson` parce que tu l'as modifié aux étapes 4 et 5, réponds **Don't Save** : on ne modifie pas le template.
2. La fenêtre propose plusieurs modèles (la liste exacte peut varier selon la version) :
   - **Open World** : un grand terrain avec **World Partition** (le monde est découpé en cellules chargées à la demande).
   - **Empty Open World** : World Partition, mais vide.
   - **Basic** : un sol, une lumière de soleil, un ciel, du brouillard. Pas de World Partition.
   - **Empty Level** : vraiment rien, noir complet.
3. Choisis **Basic**, puis **Create**. Pourquoi Basic :
   - la ruelle de Djenné de la vertical slice est un **petit espace fermé**. World Partition est fait pour les mondes ouverts de plusieurs kilomètres, et il ajoute une complexité inutile ici (cellules, Data Layers, streaming) ;
   - tu as tout de suite de la **lumière et un sol**, donc tu vois ce que tu fais (un Empty Level est noir) ;
   - on pourra toujours convertir plus tard (**Tools** > **Convert Level**, libellé variable) si la Phase 3 l'exige.
4. Sauve tout de suite : **File** > **Save Current Level As…** (**Ctrl+Alt+S**).
5. Dans la fenêtre, navigue jusqu'à **Content** > **Mansa** > **Maps**, et tape le nom **`LV_Djenne_Test`**. Clique sur **Save**.
6. **Convention de nommage** : on adopte le préfixe **`LV_`** pour les maps du jeu. Le template utilise `Lvl_` : on ne renomme pas ses maps, elles disparaîtront ou resteront comme référence (décision en A2). **À faire** : ajouter dans le `CLAUDE.md`, section *Conventions* > *Assets*, la ligne `LV_ level (map)`. Je te propose de le faire à la fin de la tâche (étape 12).

**Tu dois voir** : un sol gris sous un ciel bleu. L'onglet du viewport, ou le titre de la fenêtre, affiche `LV_Djenne_Test`. Le fichier apparaît dans `Content/Mansa/Maps/` dans le Content Browser.

**Si ça coince** :
- Tu as cliqué sur **Save** au lieu de **Save As** et le niveau s'est appelé `Untitled` ou s'est rangé ailleurs : retrouve-le dans le Content Browser, clic droit > **Rename** pour le nom, et **glisser-déposer dans l'éditeur** vers `Mansa/Maps` > **Move Here** pour l'emplacement. Jamais dans l'Explorateur Windows.
- Une fenêtre demande quels fichiers sauver (avec des cases à cocher) : c'est le dialogue **Save Content**. Coche ce qui concerne ta map et clique sur **Save Selected**.
- L'écran est noir : tu as pris **Empty Level**. Recommence avec **Basic**.

---

### Étape 9 — Placer un cube et un Player Start, tester en Play (15 min)

**But** : peupler ton niveau, y jouer, et sauver. C'est le cœur du critère de test de A1.

**Fais ceci** :
1. Dans la Main Toolbar, clique sur le bouton **Quickly add to the project** (icône cube avec un `+`) > **Shapes** > **Cube**. Le cube apparaît devant la caméra. Autre solution : **Window** > **Place Actors**, catégorie **Shapes**, puis glisse **Cube** dans le viewport.
2. Le cube flotte ou s'enfonce : sélectionne-le et appuie sur **Fin** pour le poser sur le sol.
3. Dans **Details**, renomme-le (**F2**) `Cube_Test`. Change **Scale** à `2, 2, 2` pour qu'il fasse 2 m de côté (le cube de base mesure 1 m).
4. Ajoute un point d'apparition du joueur : **Quickly add to the project** > **Basic** > **Player Start** (ou **Place Actors** > **Basic** > **Player Start**). Place-le à quelques mètres du cube, au sol (**Fin**), et oriente-le vers le cube avec **E** (rotation). Sa **flèche bleue** indique la direction dans laquelle le joueur regardera.
5. Si le Player Start affiche **BAD size** : il touche un objet (le sol ou le cube). Monte-le légèrement ou éloigne-le du cube jusqu'à ce que le message disparaisse.
6. Clique sur **Play** (**Alt+P**). Le personnage du template apparaît sur le Player Start, parce que le **GameMode par défaut du projet** est celui du Third Person (voir étape 10). Cours jusqu'au cube et saute dessus.
7. **Échap** pour arrêter.
8. **Ctrl+S** pour sauver le niveau. Vérifie qu'il n'y a plus d'astérisque sur le nom du niveau.
9. Contre-épreuve : **File** > **Open Level…** (**Ctrl+O**), ouvre `Lvl_ThirdPerson`, puis rouvre `LV_Djenne_Test` (double-clic dans `Content/Mansa/Maps/`). Le cube et le Player Start sont toujours là.

**Tu dois voir** : le personnage apparaît face au cube, peut monter dessus, et après réouverture le niveau contient bien ton cube.

**Si ça coince** :
- En Play, tu es une caméra volante sans personnage : le GameMode utilisé n'est pas celui du template. Ouvre **Window** > **World Settings** > **Game Mode** > **GameMode Override** et vérifie que la valeur est **None** (le niveau prend alors le GameMode du projet). Si le problème persiste, dis-le-moi.
- Le personnage tombe à travers le sol ou apparaît ailleurs : le Player Start est sous le sol ou mal placé. Remonte-le, puis appuie sur **Fin**.
- Tu ne retrouves pas **Shapes** dans le menu : utilise **Window** > **Place Actors** et sa barre de recherche (`Cube`).

**Analogie dev** : un Player Start, c'est le *point d'entrée* (`main` / la route par défaut) du niveau. Le GameMode, c'est le fichier de config qui dit quelle classe de joueur instancier.

---

### Étape 10 — Project Settings : où se règle la map par défaut (5 min)

**But** : savoir où le projet décide quelle map s'ouvre et quel GameMode est utilisé, **sans rien changer**.

**Fais ceci** :
1. **Edit** > **Project Settings…**
2. Dans la colonne de gauche, section **Project** > **Maps & Modes**.
3. Regarde, **sans modifier** :
   - **Default Modes** > **Default GameMode** : le GameMode du template (un Blueprint `BP_ThirdPersonGameMode` ou équivalent). C'est lui qui a fait apparaître ton personnage à l'étape 9.
   - **Default Maps** > **Editor Startup Map** : la map ouverte au lancement de l'éditeur (`Lvl_ThirdPerson`).
   - **Default Maps** > **Game Default Map** : la map chargée au lancement du jeu packagé (`Lvl_ThirdPerson`).
4. **Ne change rien maintenant.** On basculera vers les maps Mansa quand la vertical slice aura sa vraie map, et le GameMode sera remplacé en A3 par le nôtre.
5. Info utile : ces réglages sont écrits dans `C:\Dev\Mansa\Mansa\Config\DefaultEngine.ini` (clés `EditorStartupMap` et `GameDefaultMap`) et `DefaultGame.ini`. Ce sont des fichiers texte versionnés, donc chaque changement ici se verra dans Git.
6. Ferme la fenêtre Project Settings.

**Tu dois voir** : les trois réglages ci-dessus, avec les valeurs du template.

**Si ça coince** :
- La recherche est plus rapide que la navigation : tape `Default Map` dans la barre de recherche en haut de Project Settings.
- Tu as modifié une valeur par erreur : clique sur la flèche jaune **reset** à droite du champ.

**Analogie dev** : Project Settings ≈ `config/*.php` de Laravel, ou `application.yml` de Spring. Les `.ini` du dossier `Config/` jouent le rôle du `.env`, mais **sans secret** et versionnés.

---

### Étape 11 — Ce que Git voit après ton travail (10 min)

**But** : comprendre quels fichiers ton travail a créés sur le disque, et comment Git les traite.

**Fais ceci** :
1. **Ferme l'éditeur** (ou au minimum ne fais rien dans l'éditeur pendant que tu regardes).
2. Dans l'Explorateur Windows, ouvre `C:\Dev\Mansa\Mansa\Content\Mansa\Maps\`. Tu y trouves **`LV_Djenne_Test.umap`**.
3. Le niveau **Basic** n'utilise normalement pas le système **One File Per Actor** (OFPA), donc tout tient dans le `.umap`. Pour vérifier, rouvre l'éditeur sur ta map, va dans **Window** > **World Settings** et cherche `External Actors`. La case **Use External Actors** (libellé variable) est-elle cochée ?
   - **Non coché** (cas attendu) : un seul fichier `.umap` par niveau. Deux personnes qui modifient le niveau en même temps entrent en conflit, mais c'est simple.
   - **Coché** (c'est le cas de `Lvl_ThirdPerson`, qui utilise World Partition) : chaque Actor est sauvé dans son propre petit fichier `.uasset` sous `Content\__ExternalActors__\Mansa\Maps\LV_Djenne_Test\…`, avec des noms illisibles. Ouvre `C:\Dev\Mansa\Mansa\Content\__ExternalActors__\ThirdPerson\` pour voir à quoi ça ressemble. L'avantage : on peut travailler à plusieurs sur un même niveau. L'inconvénient : beaucoup de fichiers. **Ne les supprime ou ne les renomme jamais à la main.**
4. Côté Git (le dépôt `C:\Dev\Mansa` est initialisé, branche `feature/A1-prise-en-main-editeur`) :
   - le `.gitattributes` du dépôt envoie déjà **`*.umap`** et **`*.uasset`** dans **Git LFS** : ce sont des fichiers binaires, non fusionnables ;
   - `git status` montrera `Mansa/Content/Mansa/Maps/LV_Djenne_Test.umap` (et éventuellement le dossier `__ExternalActors__/...`) ;
   - `git lfs status` (après un `git add`) doit lister le `.umap` comme fichier LFS ;
   - les dossiers **`Saved/`, `Intermediate/`, `Binaries/`, `DerivedDataCache/`** sont ignorés par le `.gitignore` : ce sont des caches et des builds, régénérés automatiquement ;
   - si tu as modifié les raccourcis clavier (étape 2), ils sont stockés dans `Saved/` : c'est un réglage **personnel**, non versionné, et c'est voulu.
5. **Ne commite pas encore** si on ne te l'a pas demandé : le commit de fin de tâche se fera ensemble. Le message serait du type `feat(maps): ajoute la map de test LV_Djenne_Test`.

**Tu dois voir** : le fichier `LV_Djenne_Test.umap` sur le disque, et tu sais dire si ton niveau utilise OFPA ou non.

**Si ça coince** :
- Pas de `.umap` sur le disque : le niveau n'a pas été sauvé. Rouvre l'éditeur, **Ctrl+S**, puis vérifie le chemin.
- `git status` affiche des centaines de fichiers dans `Saved/` ou `Intermediate/` : le `.gitignore` n'est pas pris en compte (il doit être à la racine du dépôt). Signale-le-moi avant de commiter quoi que ce soit.
- Un `.umap` apparaît en « modifié » alors que tu n'as rien changé : l'éditeur re-sauve parfois les maps à l'ouverture. Si tu n'as rien voulu changer, `git restore` **éditeur fermé**.

**Analogie dev** : OFPA, c'est passer d'un gros fichier `routes.php` à un fichier par contrôleur : moins de conflits de merge, plus de fichiers. LFS, c'est stocker les blobs sur un serveur de fichiers, avec juste un pointeur dans le dépôt.

---

### Étape 12 — Récapitulatif et test final de A1 (10 min)

**But** : vérifier en une seule passe que tu maîtrises tout ce qui est attendu.

**Fais ceci** (sans relire le guide, si possible) :
1. Lance l'éditeur via `Mansa.uproject`.
2. Crée un nouveau niveau **Basic** (**File** > **New Level**).
3. Sauve-le dans `Content/Mansa/Maps/` sous le nom `LV_Test_A1` (**Ctrl+Alt+S**).
4. Place **deux cubes** (un grand, un petit posé dessus) et un **Player Start**, en utilisant **W/E/R**, le snapping et **Fin**.
5. Renomme les cubes dans l'**Outliner** et change la **Location** de l'un d'eux au clavier dans **Details**.
6. **Play** : monte sur les cubes. **Maj+F1** pour libérer la souris, **Échap** pour arrêter.
7. **Ctrl+S**. Ouvre `Lvl_ThirdPerson` avec **Ctrl+P**, puis rouvre `LV_Test_A1` : tout est là.
8. Supprime `LV_Test_A1` **dans le Content Browser** (clic droit > **Delete**). Garde `LV_Djenne_Test`.
9. Ajoute la convention de préfixe dans `C:\Dev\Mansa\CLAUDE.md`, section *Conventions*, ligne *Assets*, en ajoutant `LV_` level (map). Ou demande-le-moi et je le fais.

**Tu dois voir** : tout se fait en moins de 10 minutes sans chercher les menus. Le critère de A1 est rempli : **tu sais créer un niveau vide, y placer un cube et le sauver dans `Content/Mansa/Maps/`**.

**Si ça coince** :
- Une étape te bloque encore : refais l'étape correspondante du guide (1 à 11), c'est fait pour ça.
- La suppression affiche un avertissement sur des références : lis la liste. Pour une map de test, tu peux confirmer avec **Force Delete**, mais jamais sur un asset du template sans en parler d'abord.

**Récapitulatif des raccourcis du jour** :

| Action | Raccourci |
|---|---|
| Voler dans le viewport | Clic droit maintenu + ZQSD (après remap) / E-A monter-descendre |
| Vitesse de vol | Clic droit maintenu + molette |
| Centrer sur la sélection | F |
| Déplacer / Tourner / Échelle | W / E / R (touches *marquées*, Espace pour alterner) |
| Poser au sol | Fin |
| Dupliquer | Ctrl+D ou Alt + glisser |
| Renommer | F2 |
| Play / Simulate | Alt+P / Alt+S |
| Libérer la souris en jeu | Maj+F1 |
| Quitter le personnage en jeu | F8 |
| Arrêter le jeu | Échap |
| Content Drawer | Ctrl+Espace |
| Ouvrir un asset (recherche) | Ctrl+P |
| Nouveau niveau / Ouvrir niveau | Ctrl+N / Ctrl+O |
| Sauver le niveau / Sauver sous / Tout sauver | Ctrl+S / Ctrl+Alt+S / Ctrl+Maj+S |
| Game View (cacher les icônes) | G |

**Prochaine tâche** : A2, organiser `Content/Mansa/` et décider du sort des variantes du template.
