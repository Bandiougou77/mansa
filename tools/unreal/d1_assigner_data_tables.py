"""
MANSA — Tâche D1 : branche DT_CharacterMovement et DT_CameraProfiles sur BP_Sira.

Prérequis : a5_importer_data_tables.py a été lancé (les DT existent).
Idempotent.

Lancement dans l'éditeur (barre « Cmd ») :
  py "C:/Dev/Mansa/tools/unreal/d1_assigner_data_tables.py"
Sans surveillance :
  MANSA_AUTOQUIT=1 UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
"""

import os

import unreal

EAL = unreal.EditorAssetLibrary
CHEMIN_SIRA = "/Game/Mansa/Characters/Sira/BP_Sira"
DT_MOUVEMENT = "/Game/Mansa/Data/DT_CharacterMovement"
DT_CAMERA = "/Game/Mansa/Data/DT_CameraProfiles"


def log(message):
    unreal.log_warning("[MANSA D1] " + message)


def main():
    classe = EAL.load_blueprint_class(CHEMIN_SIRA)
    dt_mvt = unreal.load_asset(DT_MOUVEMENT)
    dt_cam = unreal.load_asset(DT_CAMERA)
    if not (classe and dt_mvt and dt_cam):
        unreal.log_error("[MANSA D1] BP_Sira ou les DT sont introuvables (lance a5_importer_data_tables.py).")
        return
    cdo = unreal.get_default_object(classe)
    cdo.set_editor_property("movement_table", dt_mvt)
    cdo.set_editor_property("movement_row_name", "Defaut")
    cam = cdo.get_editor_property("camera_profiles")
    cam.set_editor_property("profile_table", dt_cam)
    cam.set_editor_property("profil_par_defaut", "Exploration")
    EAL.save_asset(CHEMIN_SIRA, only_if_is_dirty=False)
    log("BP_Sira : MovementTable = %s, CameraProfiles.ProfileTable = %s" % (dt_mvt.get_name(), dt_cam.get_name()))


main()

if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
