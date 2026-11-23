"""
MANSA — Tâches A4 + A6 : crée les Input Actions et les contextes Enhanced Input
(conventions AAA, liste complète dans design/controles.md), puis les assigne à BP_Sira.

Ce script est IDEMPOTENT : relancé, il recrée les mappings des contextes à l'identique.
Les remaps du JOUEUR ne sont pas touchés (ils vivent dans ses réglages Enhanced Input).

Crée :
  /Game/Mansa/Input/Actions/IA_*      (19 actions, cf. ACTIONS)
  /Game/Mansa/Input/IMC_Exploration   (priorité 0, toujours actif : touches communes + manette)
  /Game/Mansa/Input/IMC_Clavier_AZERTY (priorité 0, actif si clavier AZERTY : ZQSD, A = parer)
  /Game/Mansa/Input/IMC_Clavier_QWERTY (priorité 0, actif si clavier QWERTY : WASD, Q = parer)
  /Game/Mansa/Input/IMC_Escalade      (priorité 1, pendant l'escalade : C / B = lâcher)
  /Game/Mansa/Input/IMC_Combat        (priorité 2, réservé au bloc E, vide pour l'instant)

AZERTY / QWERTY : Unreal lit la LETTRE de la touche (Z en AZERTY = W en QWERTY, même position).
Les deux profils ne sont jamais actifs ensemble (sinon le A d'AZERTY, « parer », serait aussi
le « gauche » du QWERTY). Sira active le bon profil au démarrage (détection Windows ou réglage).

Conflits voulus et résolus par la priorité : C (s'accroupir) et B/Rond (esquive) deviennent
« lâcher » en escalade, car IMC_Escalade est prioritaire et consomme la touche.

Remappage (menu Options, bloc H) : chaque mapping porte un nom stable (PlayerMappableKeySettings,
ex. « Sauter », « Sauter_Manette », « Avancer_AZERTY ») et une catégorie d'affichage. Les sticks,
la souris et les flèches ne sont pas remappables.

Lancement dans l'éditeur (barre « Cmd ») :
  py "C:/Dev/Mansa/tools/unreal/a4_creer_inputs.py"
Sans surveillance :
  MANSA_AUTOQUIT=1 UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
"""

import os

import unreal

EAL = unreal.EditorAssetLibrary
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

DOSSIER_ACTIONS = "/Game/Mansa/Input/Actions"
DOSSIER_IMC = "/Game/Mansa/Input"
CHEMIN_SIRA = "/Game/Mansa/Characters/Sira/BP_Sira"
CHEMIN_PC = "/Game/Mansa/Blueprints/BP_MansaPlayerController"

BOOL = unreal.InputActionValueType.BOOLEAN
AXE2D = unreal.InputActionValueType.AXIS2D

# Nom de l'action -> (type de valeur, description)
ACTIONS = {
    "IA_Move": (AXE2D, "Se déplacer"),
    "IA_Look": (AXE2D, "Caméra (souris + stick droit)"),
    "IA_Jump": (BOOL, "Sauter / grimper / saut d'escalade"),
    "IA_Sprint": (BOOL, "Courir / parkour rapide (maintien)"),
    "IA_SprintToggle": (BOOL, "Courir (bascule manette L3)"),
    "IA_Walk": (BOOL, "Marcher lentement (maintien)"),
    "IA_Crouch": (BOOL, "S'accroupir (bascule)"),
    "IA_Interact": (BOOL, "Interagir (parler, ramasser, ouvrir)"),
    "IA_AttackLight": (BOOL, "Attaque légère"),
    "IA_AttackHeavy": (BOOL, "Attaque lourde"),
    "IA_Parry": (BOOL, "Parer / bloquer"),
    "IA_Dodge": (BOOL, "Esquive / roulade"),
    "IA_LockOn": (BOOL, "Verrouiller une cible"),
    "IA_HunterSense": (BOOL, "Sens du chasseur"),
    "IA_DropHold": (BOOL, "Lâcher une prise"),
    "IA_Map": (BOOL, "Carte"),
    "IA_Journal": (BOOL, "Journal"),
    "IA_Inventory": (BOOL, "Inventaire"),
    "IA_Pause": (BOOL, "Pause"),
}

# Modificateurs symboliques -> construits dans creer_modificateur()
#  "avant"  : touche -> +Y (swizzle YXZ)
#  "arriere": touche -> -Y (swizzle YXZ + negate)
#  "gauche" : touche -> -X (negate)
#  "droite" : touche -> +X (rien)
#  "zone_morte" : zone morte radiale 0,2 (sticks)
#  "inverser_y" : negate Y seulement (souris, comme le template)
#
# Mapping : (touche, action, modificateurs, nom remappable ou None, libellé affiché, catégorie)
DEP, ACT, CMB, ESC, MEN = "Déplacement", "Actions", "Combat", "Escalade", "Menus"


def clavier_disposition(suffixe, avant, gauche, parer):
    return [
        (avant, "IA_Move", ["avant"], "Avancer_" + suffixe, "Avancer", DEP),
        ("S", "IA_Move", ["arriere"], "Reculer_" + suffixe, "Reculer", DEP),
        (gauche, "IA_Move", ["gauche"], "Gauche_" + suffixe, "Aller à gauche", DEP),
        ("D", "IA_Move", ["droite"], "Droite_" + suffixe, "Aller à droite", DEP),
        (parer, "IA_Parry", [], "Parer_" + suffixe, "Parer / bloquer", CMB),
    ]


CONTEXTES = {
    "IMC_Exploration": [
        # Flèches, sticks et souris : non remappables
        ("Up", "IA_Move", ["avant"], None, "", DEP),
        ("Down", "IA_Move", ["arriere"], None, "", DEP),
        ("Left", "IA_Move", ["gauche"], None, "", DEP),
        ("Right", "IA_Move", ["droite"], None, "", DEP),
        ("Gamepad_Left2D", "IA_Move", ["zone_morte"], None, "", DEP),
        ("Mouse2D", "IA_Look", ["inverser_y"], None, "", DEP),
        ("Gamepad_Right2D", "IA_Look", ["zone_morte"], None, "", DEP),
        # Clavier / souris (indépendants de la disposition)
        ("SpaceBar", "IA_Jump", [], "Sauter", "Sauter / grimper", DEP),
        ("LeftShift", "IA_Sprint", [], "Courir", "Courir / parkour rapide", DEP),
        ("LeftControl", "IA_Walk", [], "Marcher", "Marcher lentement", DEP),
        ("C", "IA_Crouch", [], "Accroupir", "S'accroupir", DEP),
        ("E", "IA_Interact", [], "Interagir", "Interagir", ACT),
        ("V", "IA_HunterSense", [], "SensChasseur", "Sens du chasseur", ACT),
        ("LeftMouseButton", "IA_AttackLight", [], "AttaqueLegere", "Attaque légère", CMB),
        ("RightMouseButton", "IA_AttackHeavy", [], "AttaqueLourde", "Attaque lourde", CMB),
        ("LeftAlt", "IA_Dodge", [], "Esquiver", "Esquive / roulade", CMB),
        ("MiddleMouseButton", "IA_LockOn", [], "Verrouiller", "Verrouiller la cible", CMB),
        ("M", "IA_Map", [], "Carte", "Carte", MEN),
        ("J", "IA_Journal", [], "Journal", "Journal", MEN),
        ("I", "IA_Inventory", [], "Inventaire", "Inventaire", MEN),
        ("Escape", "IA_Pause", [], "Pause", "Pause", MEN),
        # Manette (Xbox / PlayStation)
        ("Gamepad_FaceButton_Bottom", "IA_Jump", [], "Sauter_Manette", "Sauter / grimper", DEP),
        ("Gamepad_LeftThumbstick", "IA_SprintToggle", [], "Courir_Manette", "Courir", DEP),
        ("Gamepad_DPad_Up", "IA_Interact", [], "Interagir_Manette", "Interagir", ACT),
        ("Gamepad_RightThumbstick", "IA_HunterSense", [], "SensChasseur_Manette", "Sens du chasseur", ACT),
        ("Gamepad_FaceButton_Left", "IA_AttackLight", [], "AttaqueLegere_Manette", "Attaque légère", CMB),
        ("Gamepad_FaceButton_Top", "IA_AttackHeavy", [], "AttaqueLourde_Manette", "Attaque lourde", CMB),
        ("Gamepad_RightShoulder", "IA_Parry", [], "Parer_Manette", "Parer / bloquer", CMB),
        ("Gamepad_FaceButton_Right", "IA_Dodge", [], "Esquiver_Manette", "Esquive / roulade", CMB),
        ("Gamepad_LeftShoulder", "IA_LockOn", [], "Verrouiller_Manette", "Verrouiller la cible", CMB),
        ("Gamepad_Special_Left", "IA_Map", [], "Carte_Manette", "Carte", MEN),
        ("Gamepad_Special_Right", "IA_Pause", [], "Pause_Manette", "Pause", MEN),
    ],
    "IMC_Clavier_AZERTY": clavier_disposition("AZERTY", "Z", "Q", "A"),
    "IMC_Clavier_QWERTY": clavier_disposition("QWERTY", "W", "A", "Q"),
    "IMC_Escalade": [
        ("C", "IA_DropHold", [], "Lacher", "Lâcher la prise", ESC),
        ("Gamepad_FaceButton_Right", "IA_DropHold", [], "Lacher_Manette", "Lâcher la prise", ESC),
    ],
    # Réservé au bloc E (actions propres au combat) : vide pour l'instant.
    "IMC_Combat": [],
}

DESCRIPTIONS_IMC = {
    "IMC_Exploration": "Exploration : touches communes clavier/souris + manette (toujours actif)",
    "IMC_Clavier_AZERTY": "Clavier AZERTY : ZQSD + A = parer (actif si AZERTY)",
    "IMC_Clavier_QWERTY": "Clavier QWERTY : WASD + Q = parer (actif si QWERTY)",
    "IMC_Escalade": "Escalade : C / B = lâcher (prioritaire sur l'exploration)",
    "IMC_Combat": "Combat (bloc E)",
}

# Propriété C++ d'AMansaCharacter -> asset
PROPRIETES_SIRA = {
    "move_action": DOSSIER_ACTIONS + "/IA_Move",
    "look_action": DOSSIER_ACTIONS + "/IA_Look",
    "jump_action": DOSSIER_ACTIONS + "/IA_Jump",
    "sprint_action": DOSSIER_ACTIONS + "/IA_Sprint",
    "sprint_toggle_action": DOSSIER_ACTIONS + "/IA_SprintToggle",
    "walk_action": DOSSIER_ACTIONS + "/IA_Walk",
    "interact_action": DOSSIER_ACTIONS + "/IA_Interact",
    "attack_light_action": DOSSIER_ACTIONS + "/IA_AttackLight",
    "attack_heavy_action": DOSSIER_ACTIONS + "/IA_AttackHeavy",
    "dodge_action": DOSSIER_ACTIONS + "/IA_Dodge",
    "lock_on_action": DOSSIER_ACTIONS + "/IA_LockOn",
    "drop_hold_action": DOSSIER_ACTIONS + "/IA_DropHold",
    "pause_action": DOSSIER_ACTIONS + "/IA_Pause",
    "crouch_action": DOSSIER_ACTIONS + "/IA_Crouch",
    "parry_action": DOSSIER_ACTIONS + "/IA_Parry",
    "hunter_sense_action": DOSSIER_ACTIONS + "/IA_HunterSense",
    "map_action": DOSSIER_ACTIONS + "/IA_Map",
    "journal_action": DOSSIER_ACTIONS + "/IA_Journal",
    "inventory_action": DOSSIER_ACTIONS + "/IA_Inventory",
    "clavier_azerty_context": DOSSIER_IMC + "/IMC_Clavier_AZERTY",
    "clavier_qwerty_context": DOSSIER_IMC + "/IMC_Clavier_QWERTY",
    "exploration_context": DOSSIER_IMC + "/IMC_Exploration",
    "escalade_context": DOSSIER_IMC + "/IMC_Escalade",
    "combat_context": DOSSIER_IMC + "/IMC_Combat",
}


def log(message):
    unreal.log_warning("[MANSA A4] " + message)


def erreur(message):
    unreal.log_error("[MANSA A4] " + message)


def obtenir_ou_creer(nom, dossier, classe, fabrique):
    chemin = dossier + "/" + nom
    if EAL.does_asset_exist(chemin):
        return unreal.load_asset(chemin)
    if not EAL.does_directory_exist(dossier):
        EAL.make_directory(dossier)
    asset = ASSET_TOOLS.create_asset(nom, dossier, classe, fabrique)
    if asset is None:
        erreur("Création impossible : " + chemin)
    else:
        log("Créé : " + chemin)
    return asset


def creer_actions():
    actions = {}
    for nom, (type_valeur, description) in ACTIONS.items():
        ia = obtenir_ou_creer(nom, DOSSIER_ACTIONS, unreal.InputAction, unreal.InputAction_Factory())
        if ia is None:
            return None
        ia.set_editor_property("value_type", type_valeur)
        ia.set_editor_property("action_description", description)
        EAL.save_loaded_asset(ia, only_if_is_dirty=False)
        actions[nom] = ia
    return actions


def creer_modificateur(code, outer):
    """Crée les modificateurs instanciés (outer = l'IMC pour qu'ils soient sauvegardés avec)."""
    if code in ("avant", "arriere"):
        mods = [unreal.new_object(unreal.InputModifierSwizzleAxis, outer)]
        mods[0].set_editor_property("order", unreal.InputAxisSwizzle.YXZ)
        if code == "arriere":
            mods.append(unreal.new_object(unreal.InputModifierNegate, outer))
        return mods
    if code == "gauche":
        return [unreal.new_object(unreal.InputModifierNegate, outer)]
    if code == "droite":
        return []
    if code == "zone_morte":
        dz = unreal.new_object(unreal.InputModifierDeadZone, outer)
        dz.set_editor_property("lower_threshold", 0.2)
        dz.set_editor_property("upper_threshold", 1.0)
        dz.set_editor_property("type", unreal.DeadZoneType.RADIAL)
        return [dz]
    if code == "inverser_y":
        neg = unreal.new_object(unreal.InputModifierNegate, outer)
        neg.set_editor_property("x", False)
        neg.set_editor_property("y", True)
        neg.set_editor_property("z", False)
        return [neg]
    raise ValueError("Modificateur inconnu : " + code)


def configurer_remap(mapping, imc, nom, libelle, categorie, noms_vus):
    """Nom stable + libellé pour le menu de remappage ; None = non remappable."""
    comportements = unreal.PlayerMappableKeySettingBehaviors
    if nom is None:
        mapping.set_editor_property("setting_behavior", comportements.IGNORE_SETTINGS)
        return
    if nom in noms_vus:
        raise ValueError("Nom remappable en double : " + nom)
    noms_vus.add(nom)
    reglages = unreal.new_object(unreal.PlayerMappableKeySettings, imc)
    reglages.set_editor_property("name", nom)
    reglages.set_editor_property("display_name", libelle)
    reglages.set_editor_property("display_category", categorie)
    mapping.set_editor_property("setting_behavior", comportements.OVERRIDE_SETTINGS)
    mapping.set_editor_property("player_mappable_key_settings", reglages)


def creer_contextes(actions):
    for nom_imc, liste in CONTEXTES.items():
        imc = obtenir_ou_creer(nom_imc, DOSSIER_IMC, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
        if imc is None:
            return False
        mappings = []
        noms_vus = set()
        for nom_touche, nom_action, codes, nom_remap, libelle, categorie in liste:
            m = unreal.EnhancedActionKeyMapping()
            touche = unreal.Key()
            touche.set_editor_property("key_name", nom_touche)
            m.set_editor_property("key", touche)
            m.set_editor_property("action", actions[nom_action])
            mods = []
            for code in codes:
                mods += creer_modificateur(code, imc)
            m.set_editor_property("modifiers", mods)
            configurer_remap(m, imc, nom_remap, libelle, categorie, noms_vus)
            mappings.append(m)
        donnees = imc.get_editor_property("default_key_mappings")
        donnees.set_editor_property("mappings", mappings)
        imc.set_editor_property("default_key_mappings", donnees)
        imc.set_editor_property("context_description", DESCRIPTIONS_IMC[nom_imc])
        EAL.save_loaded_asset(imc, only_if_is_dirty=False)
        relu = imc.get_editor_property("default_key_mappings").get_editor_property("mappings")
        remappables = [r for r in relu if r.get_editor_property("player_mappable_key_settings") is not None]
        log("%s : %d mappings dont %d remappables" % (nom_imc, len(relu), len(remappables)))
    return True


def assigner_sira():
    classe = EAL.load_blueprint_class(CHEMIN_SIRA)
    if classe is None:
        erreur("BP_Sira introuvable : " + CHEMIN_SIRA)
        return False
    cdo = unreal.get_default_object(classe)
    for prop, chemin in PROPRIETES_SIRA.items():
        cdo.set_editor_property(prop, unreal.load_asset(chemin))
    # IA_Look gère la souris : l'action souris séparée du template n'est plus utilisée.
    cdo.set_editor_property("mouse_look_action", None)
    EAL.save_asset(CHEMIN_SIRA, only_if_is_dirty=False)
    log("BP_Sira : %d actions/contextes assignés" % len(PROPRIETES_SIRA))
    return True


def vider_imc_controller():
    classe = EAL.load_blueprint_class(CHEMIN_PC)
    if classe is None:
        erreur("BP_MansaPlayerController introuvable : " + CHEMIN_PC)
        return
    cdo = unreal.get_default_object(classe)
    cdo.set_editor_property("default_mapping_contexts", [])
    cdo.set_editor_property("mobile_excluded_mapping_contexts", [])
    EAL.save_asset(CHEMIN_PC, only_if_is_dirty=False)
    log("BP_MansaPlayerController : IMC du template retirés (Sira gère ses contextes)")


def main():
    actions = creer_actions()
    if not actions:
        return
    if not creer_contextes(actions):
        return
    if assigner_sira():
        vider_imc_controller()
    log("Terminé.")


main()

if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
