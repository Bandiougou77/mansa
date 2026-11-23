# -*- coding: utf-8 -*-
"""
MANSA — Tâche D3 : pose le tag PriseNormaleYaw_<degrés> sur les prises déjà présentes dans
LV_Djenne_Greybox (générées avant que c_greybox_djenne.py ne le pose), sans régénérer la map.

Pourquoi : UMansaPriseComponent lit la normale (vers l'extérieur du mur) dans le Yaw de l'acteur ;
pour un toron (cylindre couché, Pitch 90°) ce Yaw est ambigu. La normale vient de
design/data/djenne_greybox.json (champ "normale" de chaque prise).

Idempotent : un ancien tag PriseNormaleYaw_ est remplacé. La map est sauvegardée.

Lancement dans l'éditeur (barre « Cmd ») :
  py "C:/Dev/Mansa/tools/unreal/d3_tags_normales_prises.py"
Sans surveillance :
  MANSA_AUTOQUIT=1 UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
"""

import json
import math
import os

import unreal

RACINE = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
CHEMIN_JSON = os.path.join(RACINE, "design", "data", "djenne_greybox.json")


def log(message):
    unreal.log_warning("[MANSA D3] " + message)


def erreur(message):
    unreal.log_error("[MANSA D3] " + message)


def main():
    with open(CHEMIN_JSON, encoding="utf-8") as f:
        donnees = json.load(f)
    chemin_map = donnees["config"]["map"]
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not les.load_level(chemin_map):
        erreur("Impossible d'ouvrir " + chemin_map)
        return

    yaws = {}
    for pr in donnees["prises"]:
        nx, ny, _ = pr["normale"]
        yaws[pr["id"]] = round(math.degrees(math.atan2(ny, nx)), 3)

    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    trouvees = set()
    for acteur in eas.get_all_level_actors():
        tags = [str(t) for t in acteur.tags]
        ids = [t[len("PriseId_"):] for t in tags if t.startswith("PriseId_")]
        if not ids or ids[0] not in yaws:
            continue
        id_prise = ids[0]
        nouveaux = [t for t in tags if not t.startswith("PriseNormaleYaw_")]
        nouveaux.append("PriseNormaleYaw_%g" % yaws[id_prise])
        acteur.set_editor_property("tags", [unreal.Name(t) for t in nouveaux])
        trouvees.add(id_prise)

    manquantes = sorted(set(yaws) - trouvees)
    log("%d prise(s) taguée(s) sur %d" % (len(trouvees), len(yaws)))
    if manquantes:
        erreur("Prises absentes de la map : " + ", ".join(manquantes))
    if not les.save_current_level():
        erreur("Sauvegarde de la map échouée")
    else:
        log("Map sauvegardée : " + chemin_map)


main()

if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
