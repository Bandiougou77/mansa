"""
MANSA — Tâche A4 : crée les Input Actions et les contextes Enhanced Input (GDD §4.2),
puis les assigne à BP_Sira.

Ce script est IDEMPOTENT : relancé, il recrée les mappings des contextes à l'identique
(les remaps faits à la main dans les IMC seront donc écrasés : modifie plutôt ce script).

Crée :
  /Game/Mansa/Input/Actions/IA_*   (Move, Look, Jump, Sprint, SprintToggle, Walk, Interact,
                                    AttackLight, AttackHeavy, Dodge, LockOn, DropHold, Pause)
  /Game/Mansa/Input/IMC_Exploration (priorité 0, toujours actif)
  /Game/Mansa/Input/IMC_Escalade    (priorité 1, ajouté pendant l'escalade)
  /Game/Mansa/Input/IMC_Combat      (priorité 2, ajouté pendant un combat)

Clavier : AZERTY (ZQSD) ET QWERTY (WASD) sont mappés tous les deux (Unreal lit la
lettre imprimée sur la touche : Z en AZERTY, W en QWERTY ; aucune des deux ne gêne l'autre).
Manette : prioritaire. « Interagir » (exploration) et « Attaque légère » (combat)
partagent X/Carré : IMC_Combat a une priorité plus haute et consomme la touche.

Assigne aussi les actions/contextes sur le CDO de BP_Sira et vide les IMC du template
dans BP_MansaPlayerController (c'est maintenant le personnage qui gère ses contextes).

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
    "IA_Sprint": (BOOL, "Courir (maintien clavier)"),
    "IA_SprintToggle": (BOOL, "Courir (bascule manette L3)"),
    "IA_Walk": (BOOL, "Marcher lentement (maintien)"),
    "IA_Interact": (BOOL, "Interagir / parler"),
    "IA_AttackLight": (BOOL, "Attaque légère"),
    "IA_AttackHeavy": (BOOL, "Attaque lourde"),
    "IA_Dodge": (BOOL, "Esquive"),
    "IA_LockOn": (BOOL, "Verrouiller une cible"),
    "IA_DropHold": (BOOL, "Lâcher une prise"),
    "IA_Pause": (BOOL, "Pause"),
}

# Modificateurs symboliques -> construits dans creer_modificateur()
#  "avant"  : touche -> +Y (swizzle YXZ)
#  "arriere": touche -> -Y (swizzle YXZ + negate)
#  "gauche" : touche -> -X (negate)
#  "droite" : touche -> +X (rien)
#  "zone_morte" : zone morte radiale 0,2 (sticks)
#  "inverser_y" : negate Y seulement (souris, comme le template)
DEPLACEMENT_CLAVIER = [
    # AZERTY
    ("Z", "IA_Move", ["avant"]), ("S", "IA_Move", ["arriere"]),
    ("Q", "IA_Move", ["gauche"]), ("D", "IA_Move", ["droite"]),
    # QWERTY (S et D sont communs)
    ("W", "IA_Move", ["avant"]), ("A", "IA_Move", ["gauche"]),
    # Flèches
    ("Up", "IA_Move", ["avant"]), ("Down", "IA_Move", ["arriere"]),
    ("Left", "IA_Move", ["gauche"]), ("Right", "IA_Move", ["droite"]),
]

CONTEXTES = {
    "IMC_Exploration": DEPLACEMENT_CLAVIER + [
        ("Gamepad_Left2D", "IA_Move", ["zone_morte"]),
        ("Mouse2D", "IA_Look", ["inverser_y"]),
        ("Gamepad_Right2D", "IA_Look", ["zone_morte"]),
        ("SpaceBar", "IA_Jump", []),
        ("Gamepad_FaceButton_Bottom", "IA_Jump", []),
        ("LeftShift", "IA_Sprint", []),
        ("Gamepad_LeftThumbstick", "IA_SprintToggle", []),
        ("LeftAlt", "IA_Walk", []),
        ("E", "IA_Interact", []),
        ("Gamepad_FaceButton_Left", "IA_Interact", []),
        ("Escape", "IA_Pause", []),
        ("Gamepad_Special_Right", "IA_Pause", []),
    ],
    "IMC_Escalade": [
        ("C", "IA_DropHold", []),
        ("Gamepad_FaceButton_Right", "IA_DropHold", []),
    ],
    "IMC_Combat": [
        ("LeftMouseButton", "IA_AttackLight", []),
        ("Gamepad_FaceButton_Left", "IA_AttackLight", []),
        ("RightMouseButton", "IA_AttackHeavy", []),
        ("Gamepad_FaceButton_Top", "IA_AttackHeavy", []),
        ("LeftControl", "IA_Dodge", []),
        ("LeftShift", "IA_Dodge", []),
        ("Gamepad_FaceButton_Right", "IA_Dodge", []),
        ("MiddleMouseButton", "IA_LockOn", []),
        ("Gamepad_RightThumbstick", "IA_LockOn", []),
    ],
}

DESCRIPTIONS_IMC = {
    "IMC_Exploration": "Exploration (toujours actif)",
    "IMC_Escalade": "Escalade (par-dessus l'exploration)",
    "IMC_Combat": "Combat (par-dessus l'exploration)",
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


def creer_contextes(actions):
    for nom_imc, liste in CONTEXTES.items():
        imc = obtenir_ou_creer(nom_imc, DOSSIER_IMC, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
        if imc is None:
            return False
        mappings = []
        for nom_touche, nom_action, codes in liste:
            m = unreal.EnhancedActionKeyMapping()
            touche = unreal.Key()
            touche.set_editor_property("key_name", nom_touche)
            m.set_editor_property("key", touche)
            m.set_editor_property("action", actions[nom_action])
            mods = []
            for code in codes:
                mods += creer_modificateur(code, imc)
            m.set_editor_property("modifiers", mods)
            mappings.append(m)
        donnees = imc.get_editor_property("default_key_mappings")
        donnees.set_editor_property("mappings", mappings)
        imc.set_editor_property("default_key_mappings", donnees)
        imc.set_editor_property("context_description", DESCRIPTIONS_IMC[nom_imc])
        EAL.save_loaded_asset(imc, only_if_is_dirty=False)
        relu = imc.get_editor_property("default_key_mappings").get_editor_property("mappings")
        log("%s : %d mappings" % (nom_imc, len(relu)))
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
