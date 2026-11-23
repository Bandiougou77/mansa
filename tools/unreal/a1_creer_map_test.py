"""
MANSA — Tâche A1 : crée la map de test LV_Djenne_Test.

Ce que fait le script (idempotent : on peut le relancer sans risque) :
  1. crée le dossier /Game/Mansa/Maps ;
  2. crée LV_Djenne_Test à partir du template « Basic » (sol, ciel, lumière),
     ou l'ouvre si elle existe déjà ;
  3. ajoute un cube (SM_CubeTest) et un Player Start s'ils manquent ;
  4. sauvegarde la map.

Lancement dans l'éditeur Unreal (barre « Cmd » en bas de l'écran) :
  py "C:/Dev/Mansa/tools/unreal/a1_creer_map_test.py"
Sans surveillance (éditeur complet lancé puis fermé automatiquement) :
  MANSA_AUTOQUIT=1 UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
Prérequis : plugins « Python Editor Script Plugin » et « Editor Scripting Utilities » activés (déclarés dans Mansa.uproject).
"""

import os

import unreal

DOSSIER_MAPS = "/Game/Mansa/Maps"
CHEMIN_MAP = DOSSIER_MAPS + "/LV_Djenne_Test"
TEMPLATE_BASIC = "/Engine/Maps/Templates/Template_Default"
MESH_CUBE = "/Engine/BasicShapes/Cube"

LABEL_CUBE = "SM_CubeTest"
POSITION_CUBE = unreal.Vector(400.0, 0.0, 50.0)       # cm ; le cube de base fait 100 cm
POSITION_PLAYER_START = unreal.Vector(0.0, 0.0, 120.0)


def log(message):
    unreal.log("[MANSA A1] " + message)


def preparer_map(level_editor):
    """Crée la map depuis le template Basic, ou l'ouvre si elle existe."""
    if not unreal.EditorAssetLibrary.does_directory_exist(DOSSIER_MAPS):
        unreal.EditorAssetLibrary.make_directory(DOSSIER_MAPS)
        log("Dossier créé : " + DOSSIER_MAPS)

    if unreal.EditorAssetLibrary.does_asset_exist(CHEMIN_MAP):
        log("La map existe déjà, ouverture : " + CHEMIN_MAP)
        return level_editor.load_level(CHEMIN_MAP)

    if level_editor.new_level_from_template(CHEMIN_MAP, TEMPLATE_BASIC):
        log("Map créée depuis le template Basic : " + CHEMIN_MAP)
        return True

    unreal.log_error("[MANSA A1] Échec de création depuis le template " + TEMPLATE_BASIC)
    return False


def acteurs_du_niveau(actor_subsystem):
    return actor_subsystem.get_all_level_actors()


def ajouter_cube(actor_subsystem):
    for acteur in acteurs_du_niveau(actor_subsystem):
        if acteur.get_actor_label() == LABEL_CUBE:
            log("Cube déjà présent, rien à faire.")
            return
    mesh = unreal.EditorAssetLibrary.load_asset(MESH_CUBE)
    cube = actor_subsystem.spawn_actor_from_object(mesh, POSITION_CUBE)
    if cube is None:
        unreal.log_error("[MANSA A1] Impossible d'ajouter le cube (lance le script depuis l'éditeur ouvert).")
        return
    cube.set_actor_label(LABEL_CUBE)
    log("Cube ajouté : " + LABEL_CUBE)


def ajouter_player_start(actor_subsystem):
    for acteur in acteurs_du_niveau(actor_subsystem):
        if isinstance(acteur, unreal.PlayerStart):
            log("Player Start déjà présent, rien à faire.")
            return
    if actor_subsystem.spawn_actor_from_class(unreal.PlayerStart, POSITION_PLAYER_START) is None:
        unreal.log_error("[MANSA A1] Impossible d'ajouter le Player Start.")
        return
    log("Player Start ajouté.")


def main():
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if not preparer_map(level_editor):
        return

    ajouter_cube(actor_subsystem)
    ajouter_player_start(actor_subsystem)

    if level_editor.save_current_level():
        log("Map sauvegardée. Clique sur Play (Jouer) pour tester.")
    else:
        unreal.log_error("[MANSA A1] La sauvegarde a échoué : fais Ctrl+S à la main.")


main()

# Lancement automatique sans surveillance : fermer l'éditeur à la fin.
if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
