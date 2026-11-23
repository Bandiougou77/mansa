"""
MANSA — Tâche A3 : crée BP_Sira et le GameMode Mansa, branchés sur LV_Djenne_Test.

Le template fournit déjà la classe C++ AMansaCharacter (abstraite) et son
Blueprint enfant BP_ThirdPersonCharacter. Ce script (idempotent) :
  1. duplique BP_ThirdPersonCharacter       -> /Game/Mansa/Characters/Sira/BP_Sira
  2. duplique BP_ThirdPersonPlayerController -> /Game/Mansa/Blueprints/BP_MansaPlayerController
  3. duplique BP_ThirdPersonGameMode        -> /Game/Mansa/Blueprints/BP_MansaGameMode
     et y règle Default Pawn = BP_Sira, Player Controller = BP_MansaPlayerController ;
  4. règle le GameMode Override de LV_Djenne_Test sur BP_MansaGameMode et sauvegarde.
Les maps du template ne sont pas modifiées.

Lancement dans l'éditeur (barre « Cmd ») :
  py "C:/Dev/Mansa/tools/unreal/a3_creer_bp_sira.py"
Sans surveillance :
  MANSA_AUTOQUIT=1 UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
"""

import os

import unreal

EAL = unreal.EditorAssetLibrary

SOURCES = {
    "character": "/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter",
    "controller": "/Game/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController",
    "gamemode": "/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode",
}
CIBLES = {
    "character": "/Game/Mansa/Characters/Sira/BP_Sira",
    "controller": "/Game/Mansa/Blueprints/BP_MansaPlayerController",
    "gamemode": "/Game/Mansa/Blueprints/BP_MansaGameMode",
}
CHEMIN_MAP = "/Game/Mansa/Maps/LV_Djenne_Test"


def log(message):
    unreal.log("[MANSA A3] " + message)


def erreur(message):
    unreal.log_error("[MANSA A3] " + message)


def dupliquer(cle):
    source, cible = SOURCES[cle], CIBLES[cle]
    if EAL.does_asset_exist(cible):
        log("Existe déjà : " + cible)
        return True
    dossier = cible.rsplit("/", 1)[0]
    if not EAL.does_directory_exist(dossier):
        EAL.make_directory(dossier)
    if EAL.duplicate_asset(source, cible) is None:
        erreur("Échec de la duplication " + source + " -> " + cible)
        return False
    EAL.save_asset(cible, only_if_is_dirty=False)
    log("Créé : " + cible)
    return True


def classe_bp(cle):
    return EAL.load_blueprint_class(CIBLES[cle])


def regler_gamemode():
    gm = classe_bp("gamemode")
    sira = classe_bp("character")
    pc = classe_bp("controller")
    if not (gm and sira and pc):
        erreur("Impossible de charger les classes Blueprint.")
        return None
    cdo = unreal.get_default_object(gm)
    cdo.set_editor_property("default_pawn_class", sira)
    cdo.set_editor_property("player_controller_class", pc)
    EAL.save_asset(CIBLES["gamemode"], only_if_is_dirty=False)
    log("BP_MansaGameMode : pawn = %s, controller = %s" % (
        cdo.get_editor_property("default_pawn_class").get_name(),
        cdo.get_editor_property("player_controller_class").get_name()))
    return gm


def regler_map(gm):
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_editor.load_level(CHEMIN_MAP):
        erreur("Impossible d'ouvrir " + CHEMIN_MAP + " (lance d'abord a1_creer_map_test.py).")
        return
    monde = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    reglages = monde.get_world_settings()
    reglages.set_editor_property("default_game_mode", gm)
    if level_editor.save_current_level():
        log("LV_Djenne_Test utilise maintenant BP_MansaGameMode. Clique sur Play pour tester.")
    else:
        erreur("Sauvegarde de la map échouée : fais Ctrl+S à la main.")


def main():
    for cle in ("character", "controller", "gamemode"):
        if not dupliquer(cle):
            return
    gm = regler_gamemode()
    if gm:
        regler_map(gm)


main()

if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
