"""
MANSA — Vérification en lecture seule du socle (A4, A5, D1, D2).

Ne modifie aucun asset. Affiche OK / ÉCHEC pour chaque point et un bilan final.
Rapide, en commandlet (pas besoin d'ouvrir l'éditeur) :
  UnrealEditor-Cmd.exe Mansa.uproject -run=pythonscript -script="C:/Dev/Mansa/tools/unreal/verifier_socle.py" -unattended -nop4 -nosplash -stdout
Les lignes utiles commencent par « [MANSA VERIF] ».
"""

import unreal

EAL = unreal.EditorAssetLibrary
ECHECS = []


def log(message):
    unreal.log_warning("[MANSA VERIF] " + message)


def verifier(condition, message):
    log(("OK     " if condition else "ÉCHEC  ") + message)
    if not condition:
        ECHECS.append(message)
    return condition


# ---------------------------------------------------------------- A4
ACTIONS = ["IA_Move", "IA_Look", "IA_Jump", "IA_Sprint", "IA_SprintToggle", "IA_Walk", "IA_Interact",
           "IA_AttackLight", "IA_AttackHeavy", "IA_Dodge", "IA_LockOn", "IA_DropHold", "IA_Pause"]
CONTEXTES = {"IMC_Exploration": 22, "IMC_Escalade": 2, "IMC_Combat": 9}
PROPS_SIRA = {
    "move_action": "IA_Move", "look_action": "IA_Look", "jump_action": "IA_Jump",
    "sprint_action": "IA_Sprint", "sprint_toggle_action": "IA_SprintToggle", "walk_action": "IA_Walk",
    "interact_action": "IA_Interact", "attack_light_action": "IA_AttackLight",
    "attack_heavy_action": "IA_AttackHeavy", "dodge_action": "IA_Dodge", "lock_on_action": "IA_LockOn",
    "drop_hold_action": "IA_DropHold", "pause_action": "IA_Pause",
    "exploration_context": "IMC_Exploration", "escalade_context": "IMC_Escalade", "combat_context": "IMC_Combat",
}


def verifier_a4():
    for nom in ACTIONS:
        verifier(EAL.does_asset_exist("/Game/Mansa/Input/Actions/" + nom), "A4 action " + nom)
    for nom, attendu in CONTEXTES.items():
        imc = unreal.load_asset("/Game/Mansa/Input/" + nom)
        if not verifier(imc is not None, "A4 contexte " + nom):
            continue
        mappings = imc.get_editor_property("default_key_mappings").get_editor_property("mappings")
        touches = [m.get_editor_property("key").get_editor_property("key_name") for m in mappings]
        verifier(len(mappings) == attendu, "A4 %s : %d mappings (attendu %d)" % (nom, len(mappings), attendu))
        if nom == "IMC_Exploration":
            for t in ("Z", "Q", "W", "A", "S", "D", "Gamepad_Left2D", "Gamepad_Right2D", "Mouse2D"):
                verifier(str(t) in [str(x) for x in touches], "A4 IMC_Exploration contient " + t)
            # Le recul (S) doit avoir swizzle + negate : vérifie que les modificateurs ont été sauvés
            for m in mappings:
                if str(m.get_editor_property("key").get_editor_property("key_name")) == "S":
                    noms = [x.get_class().get_name() for x in m.get_editor_property("modifiers")]
                    verifier(noms == ["InputModifierSwizzleAxis", "InputModifierNegate"], "A4 modificateurs de S : %s" % noms)

    classe = EAL.load_blueprint_class("/Game/Mansa/Characters/Sira/BP_Sira")
    if not verifier(classe is not None, "BP_Sira chargé"):
        return None
    cdo = unreal.get_default_object(classe)
    for prop, nom in PROPS_SIRA.items():
        valeur = cdo.get_editor_property(prop)
        verifier(valeur is not None and valeur.get_name() == nom, "A4 BP_Sira.%s = %s" % (prop, valeur.get_name() if valeur else None))
    return cdo


# ---------------------------------------------------------------- A5 / D1
def lignes_dt(chemin):
    dt = unreal.load_asset(chemin)
    if dt is None:
        return None, []
    return dt, [str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)]


def colonne(dt, nom_colonne):
    return [str(v) for v in unreal.DataTableFunctionLibrary.get_data_table_column_as_string(dt, nom_colonne)]


def verifier_a5(cdo):
    dt, lignes = lignes_dt("/Game/Mansa/Data/DT_CharacterMovement")
    if verifier(dt is not None, "A5 DT_CharacterMovement existe"):
        struct = unreal.DataTableFunctionLibrary.get_data_table_row_struct(dt)
        verifier(struct is not None and struct.get_name() == "MansaMovementRow", "A5 DT_CharacterMovement struct = %s" % (struct.get_name() if struct else None))
        verifier("Defaut" in lignes, "A5 DT_CharacterMovement a la ligne Defaut (%s)" % lignes)
        noms_col = [str(c) for c in unreal.DataTableFunctionLibrary.get_data_table_column_names(dt)]
        i = lignes.index("Defaut") if "Defaut" in lignes else 0
        attendus = {"VitesseMarche": 200.0, "VitesseJogging": 400.0, "VitesseCourse": 650.0, "HauteurSaut": 120.0,
                    "HauteurEnjambementMax": 90.0, "HauteurMantleMax": 180.0}
        for col, val in attendus.items():
            if verifier(col in noms_col, "A5 colonne " + col):
                lu = float(colonne(dt, col)[i])
                verifier(abs(lu - val) < 0.01, "A5 Defaut.%s = %s (attendu %s)" % (col, lu, val))

    dt, lignes = lignes_dt("/Game/Mansa/Data/DT_CameraProfiles")
    if verifier(dt is not None, "A5 DT_CameraProfiles existe"):
        for ligne in ("Exploration", "Escalade", "Combat"):
            verifier(ligne in lignes, "A5 DT_CameraProfiles a la ligne " + ligne)
        attendus = {"Exploration": (300, 85), "Escalade": (400, 90), "Combat": (450, 90)}
        bras = colonne(dt, "LongueurBras")
        fov = colonne(dt, "FOV")
        for ligne, (b, f) in attendus.items():
            if ligne in lignes:
                i = lignes.index(ligne)
                verifier(abs(float(bras[i]) - b) < 0.01 and abs(float(fov[i]) - f) < 0.01,
                         "A5 %s : bras %s FOV %s (attendu %s / %s)" % (ligne, bras[i], fov[i], b, f))

    if cdo is not None:
        dt_mvt = cdo.get_editor_property("movement_table")
        verifier(dt_mvt is not None and dt_mvt.get_name() == "DT_CharacterMovement", "D1 BP_Sira.movement_table assignée")
        cam = cdo.get_editor_property("camera_profiles")
        if verifier(cam is not None, "D1 composant CameraProfiles présent"):
            dt_cam = cam.get_editor_property("profile_table")
            verifier(dt_cam is not None and dt_cam.get_name() == "DT_CameraProfiles", "D1 CameraProfiles.profile_table assignée")
        try:
            trav = cdo.get_editor_property("traversal")
        except Exception:
            trav = None
        verifier(trav is not None, "D2 composant Traversal présent")


# ---------------------------------------------------------------- D2
def verifier_d2():
    # Ouvre la map (lecture seule : rien n'est sauvegardé) et liste les obstacles tagués.
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not verifier(level_editor.load_level("/Game/Mansa/Maps/LV_Djenne_Test"), "D2 LV_Djenne_Test ouverte"):
        return
    acteurs = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    obstacles = [a for a in acteurs if "MansaObstacleTest" in [str(t) for t in a.tags]]
    labels = sorted(a.get_actor_label() for a in obstacles)
    verifier(labels == ["OBS_Muret_060", "OBS_Rebord_120", "OBS_Rebord_170"], "D2 obstacles de test : %s" % labels)
    for a in obstacles:
        origine, etendue = a.get_actor_bounds(False)
        log("       %s : hauteur %.0f cm" % (a.get_actor_label(), etendue.z * 2))


def main():
    cdo = verifier_a4()
    verifier_a5(cdo)
    verifier_d2()
    if ECHECS:
        log("BILAN : %d échec(s)" % len(ECHECS))
    else:
        log("BILAN : tout est OK")


main()
