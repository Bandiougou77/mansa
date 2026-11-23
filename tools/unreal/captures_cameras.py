"""
MANSA — Captures d'écran automatiques depuis les CameraActor d'une map.

Usage (éditeur complet, sans surveillance) :
  MANSA_AUTOQUIT=1 MANSA_MAP=/Game/Mansa/Maps/LV_Djenne_Greybox \
  UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<ce fichier>" -unattended
Les images vont dans Mansa/Saved/Screenshots/ (non versionné).
On laisse tourner le rendu quelques centaines d'images entre deux captures
pour que Lumen et l'exposition se stabilisent.
"""

import os

import unreal

CHEMIN_MAP = os.environ.get("MANSA_MAP", "/Game/Mansa/Maps/LV_Djenne_Greybox")
LARGEUR, HAUTEUR = 1600, 900
TICKS_AVANT_CAPTURE = 240
TICKS_APRES_CAPTURE = 60


def log(m):
    unreal.log_warning("[MANSA CAPTURE] " + m)


level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ue_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

level_editor.load_level(CHEMIN_MAP)
cameras = [a for a in actors.get_all_level_actors() if isinstance(a, unreal.CameraActor)]
cameras.sort(key=lambda c: c.get_actor_label())
log("%d caméras trouvées dans %s" % (len(cameras), CHEMIN_MAP))

etat = {"index": 0, "ticks": 0, "phase": "attente", "handle": None}


def placer(cam):
    ue_sub.set_level_viewport_camera_info(cam.get_actor_location(), cam.get_actor_rotation())


def tick(_delta):
    if etat["index"] >= len(cameras):
        unreal.unregister_slate_post_tick_callback(etat["handle"])
        log("Terminé.")
        if os.environ.get("MANSA_AUTOQUIT") == "1":
            unreal.SystemLibrary.quit_editor()
        return
    cam = cameras[etat["index"]]
    etat["ticks"] += 1
    if etat["phase"] == "attente":
        if etat["ticks"] == 1:
            placer(cam)
        if etat["ticks"] >= TICKS_AVANT_CAPTURE:
            nom = "mansa_%s.png" % cam.get_actor_label()
            unreal.AutomationLibrary.take_high_res_screenshot(LARGEUR, HAUTEUR, nom, camera=cam)
            log("Capture demandée : " + nom)
            etat["phase"], etat["ticks"] = "ecriture", 0
    elif etat["ticks"] >= TICKS_APRES_CAPTURE:
        etat["index"] += 1
        etat["phase"], etat["ticks"] = "attente", 0


if cameras:
    etat["handle"] = unreal.register_slate_post_tick_callback(tick)
elif os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
