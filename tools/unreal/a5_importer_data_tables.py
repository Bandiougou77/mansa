"""
MANSA — Tâche A5 : importe (ou réimporte) les Data Tables du socle depuis les CSV de /design/data.

  design/data/DT_CharacterMovement.csv -> /Game/Mansa/Data/DT_CharacterMovement (FMansaMovementRow)
  design/data/DT_CameraProfiles.csv    -> /Game/Mansa/Data/DT_CameraProfiles    (FMansaCameraProfileRow)
  design/data/DT_PrisesEscalade.csv    -> /Game/Mansa/Data/DT_PrisesEscalade    (FMansaPriseRow, D3)
  design/data/DT_LibellesTouches.csv   -> /Game/Mansa/Data/DT_LibellesTouches   (FMansaLibelleToucheRow, A6)

Pourquoi /design/data ? Les CSV sont des données de DESIGN (lisibles, comparables dans Git,
modifiables sans Unreal). Les .uasset générés dans Content/ sont aussi commités pour que le jeu
marche sans relancer l'import.

Flux de travail :
  1. modifier le CSV (Excel / LibreOffice / éditeur texte, séparateur virgule) ;
  2. relancer ce script (ou clic droit sur la DT > Reimport : la source est mémorisée) ;
  3. Play : le personnage relit la DT au démarrage.
Attention : une modification faite directement dans l'éditeur de DT est écrasée au prochain import.

Lancement dans l'éditeur (barre « Cmd ») :
  py "C:/Dev/Mansa/tools/unreal/a5_importer_data_tables.py"
Sans surveillance :
  MANSA_AUTOQUIT=1 UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
"""

import os

import unreal

DOSSIER_CSV = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "design", "data"))
DOSSIER_DT = "/Game/Mansa/Data"

TABLES = [
    ("DT_CharacterMovement", "/Script/Mansa.MansaMovementRow"),
    ("DT_CameraProfiles", "/Script/Mansa.MansaCameraProfileRow"),
    ("DT_PrisesEscalade", "/Script/Mansa.MansaPriseRow"),
    ("DT_LibellesTouches", "/Script/Mansa.MansaLibelleToucheRow"),
]


def log(message):
    unreal.log_warning("[MANSA A5] " + message)


def erreur(message):
    unreal.log_error("[MANSA A5] " + message)


def importer(nom, chemin_struct):
    fichier = os.path.join(DOSSIER_CSV, nom + ".csv")
    if not os.path.isfile(fichier):
        erreur("CSV introuvable : " + fichier)
        return False
    struct = unreal.load_object(None, chemin_struct)
    if struct is None:
        erreur("Struct introuvable (le module Mansa est-il compilé ?) : " + chemin_struct)
        return False

    fabrique = unreal.CSVImportFactory()
    reglages = fabrique.get_editor_property("automated_import_settings")
    reglages.set_editor_property("import_type", unreal.CSVImportType.ECSV_DATA_TABLE)
    reglages.set_editor_property("import_row_struct", struct)
    fabrique.set_editor_property("automated_import_settings", reglages)

    tache = unreal.AssetImportTask()
    tache.set_editor_property("filename", fichier)
    tache.set_editor_property("destination_path", DOSSIER_DT)
    tache.set_editor_property("destination_name", nom)
    tache.set_editor_property("replace_existing", True)
    tache.set_editor_property("automated", True)
    tache.set_editor_property("save", True)
    tache.set_editor_property("factory", fabrique)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([tache])

    dt = unreal.load_asset(DOSSIER_DT + "/" + nom)
    if dt is None:
        erreur("Import échoué : " + nom)
        return False
    lignes = [str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)]
    log("%s importée depuis %s : lignes %s" % (nom, fichier, lignes))
    return True


def main():
    if not unreal.EditorAssetLibrary.does_directory_exist(DOSSIER_DT):
        unreal.EditorAssetLibrary.make_directory(DOSSIER_DT)
    for nom, struct in TABLES:
        importer(nom, struct)


main()

if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
