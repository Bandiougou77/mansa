"""
MANSA — Tâche D2 : ajoute 3 obstacles de test (60, 120, 170 cm) dans LV_Djenne_Test.

  OBS_Muret_060  : muret fin (30 cm d'épaisseur)   -> enjambement
  OBS_Rebord_120 : bloc profond (200 cm)           -> mantle
  OBS_Rebord_170 : bloc profond (200 cm)           -> mantle (proche de la limite de 180 cm)

Les obstacles sont alignés le long de Y, à gauche du Player Start, leur face d'approche
tournée vers -X (Sira arrive depuis le Player Start en regardant vers +X).
Ils portent le tag « MansaObstacleTest » utilisé par la commande console MansaTestFranchissement.
Idempotent : les obstacles existants (même label) sont replacés, pas dupliqués.

Lancement dans l'éditeur (barre « Cmd ») :
  py "C:/Dev/Mansa/tools/unreal/d2_obstacles_test.py"
Sans surveillance :
  MANSA_AUTOQUIT=1 UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
"""

import os

import unreal

CHEMIN_MAP = "/Game/Mansa/Maps/LV_Djenne_Test"
MESH_CUBE = "/Engine/BasicShapes/Cube"   # cube de 100 cm, pivot au centre
TAG = "MansaObstacleTest"

# label : (centre X, centre Y, épaisseur X, largeur Y, hauteur Z) en cm, posé sur le sol (Z = 0)
OBSTACLES = {
    "OBS_Muret_060": (600.0, -500.0, 30.0, 300.0, 60.0),
    "OBS_Rebord_120": (685.0, -900.0, 200.0, 300.0, 120.0),
    "OBS_Rebord_170": (685.0, -1300.0, 200.0, 300.0, 170.0),
}


def log(message):
    unreal.log_warning("[MANSA D2] " + message)


def main():
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not level_editor.load_level(CHEMIN_MAP):
        unreal.log_error("[MANSA D2] Impossible d'ouvrir " + CHEMIN_MAP)
        return

    existants = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
    mesh = unreal.load_asset(MESH_CUBE)

    for label, (x, y, ep, larg, haut) in OBSTACLES.items():
        acteur = existants.get(label)
        if acteur is None:
            acteur = actors.spawn_actor_from_object(mesh, unreal.Vector(x, y, haut / 2.0))
            if acteur is None:
                unreal.log_error("[MANSA D2] Spawn impossible : " + label)
                continue
            acteur.set_actor_label(label)
        acteur.set_actor_location(unreal.Vector(x, y, haut / 2.0), False, False)
        acteur.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)
        acteur.set_actor_scale3d(unreal.Vector(ep / 100.0, larg / 100.0, haut / 100.0))
        acteur.set_editor_property("tags", [TAG])
        acteur.set_folder_path("Tests/Franchissement")
        log("%s : %.0f cm de haut, %.0f cm d'épaisseur, en (%.0f, %.0f)" % (label, haut, ep, x, y))

    if level_editor.save_current_level():
        log("LV_Djenne_Test sauvegardée.")
    else:
        unreal.log_error("[MANSA D2] Sauvegarde échouée : fais Ctrl+S à la main.")


main()

if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
