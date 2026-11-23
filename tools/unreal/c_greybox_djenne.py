# -*- coding: utf-8 -*-
"""
MANSA - Générateur de greybox de la vertical slice « Djenné ».

À lancer dans l'éditeur Unreal COMPLET (pas en commandlet) :
    Outils > Execute Python Script... > c_greybox_djenne.py
ou en ligne de commande :
    UnrealEditor.exe Mansa.uproject -ExecutePythonScript="<chemin>/c_greybox_djenne.py"
    (avec MANSA_AUTOQUIT=1 pour fermer l'éditeur à la fin)

Toutes les données (géométrie, prises, marqueurs, lumière, palette, chemins d'assets)
viennent de djenne_greybox.json. Le script ne contient aucune donnée de niveau.

Idempotent : si la map existe, elle est rouverte et seuls les acteurs portant le tag
`MansaGreybox` (configurable dans le JSON) sont supprimés puis recréés. Les acteurs
ajoutés à la main (sans ce tag) sont conservés.
"""

import json
import math
import os
import traceback

import unreal

# ---------------------------------------------------------------------------
# Emplacement du JSON
# ---------------------------------------------------------------------------
NOM_FICHIER_JSON = "djenne_greybox.json"

try:
    _DOSSIER_SCRIPT = os.path.dirname(os.path.abspath(__file__))
except NameError:  # exécution par exec() sans __file__
    _DOSSIER_SCRIPT = os.getcwd()

# Candidats, dans l'ordre : le premier qui existe gagne.
CHEMIN_JSON = [
    # 1. à côté du script (scratchpad, ou si on range le JSON avec le script)
    os.path.join(_DOSSIER_SCRIPT, NOM_FICHIER_JSON),
    # 2. emplacement final : <repo>/tools/unreal/ -> <repo>/design/data/
    os.path.normpath(os.path.join(_DOSSIER_SCRIPT, "..", "..", "design", "data", NOM_FICHIER_JSON)),
]

PREFIXE = "[MANSA GREYBOX]"


# ---------------------------------------------------------------------------
# Journalisation
# ---------------------------------------------------------------------------
def log(msg):
    unreal.log_warning(f"{PREFIXE} {msg}")


def log_info(msg):
    unreal.log(f"{PREFIXE} {msg}")


def log_erreur(msg):
    unreal.log_error(f"{PREFIXE} {msg}")


# ---------------------------------------------------------------------------
# Outils mathématiques (aucune dépendance aux méthodes de struct d'Unreal)
# ---------------------------------------------------------------------------
def srgb_vers_lineaire(c8):
    """Composante sRGB 0-255 -> linéaire 0-1 (les Constant3Vector sont en linéaire)."""
    c = c8 / 255.0
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def couleur_lineaire(rgb, a=1.0):
    return unreal.LinearColor(srgb_vers_lineaire(rgb[0]), srgb_vers_lineaire(rgb[1]),
                              srgb_vers_lineaire(rgb[2]), a)


def tourner_vecteur(v, pitch, yaw, roll):
    """Équivalent de FRotationMatrix(FRotator(P, Y, R)).TransformVector(v), en degrés."""
    p, y, r = math.radians(pitch), math.radians(yaw), math.radians(roll)
    sp, cp = math.sin(p), math.cos(p)
    sy, cy = math.sin(y), math.cos(y)
    sr, cr = math.sin(r), math.cos(r)
    ax = (cp * cy, cp * sy, sp)
    ay = (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp)
    az = (-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp)
    return tuple(v[0] * ax[i] + v[1] * ay[i] + v[2] * az[i] for i in range(3))


def rotateur(rot):
    """dict {pitch, yaw, roll} -> unreal.Rotator (mots-clés : l'ordre positionnel est roll, pitch, yaw)."""
    rot = rot or {}
    return unreal.Rotator(roll=float(rot.get("roll", 0)), pitch=float(rot.get("pitch", 0)),
                          yaw=float(rot.get("yaw", 0)))


def vecteur(v):
    return unreal.Vector(float(v[0]), float(v[1]), float(v[2]))


# ---------------------------------------------------------------------------
# Générateur
# ---------------------------------------------------------------------------
class GenerateurGreybox:

    def __init__(self, donnees):
        self.d = donnees
        self.cfg = donnees["config"]
        self.tag = self.cfg["tag_greybox"]
        self.eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        self.les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        self.ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        self.eal = unreal.EditorAssetLibrary
        self.compteurs = {}
        self.erreurs = 0
        self.avertissements = 0
        self.materiaux = {}
        self.meshes = {}         # forme -> (mesh, taille_native(x,y,z), origine_bounds(x,y,z))
        self.correction_bounds_signalee = set()

    # -- utilitaires -------------------------------------------------------
    def compter(self, cat, n=1):
        self.compteurs[cat] = self.compteurs.get(cat, 0) + n

    def erreur(self, msg):
        self.erreurs += 1
        log_erreur(msg)

    def avertir(self, msg):
        self.avertissements += 1
        unreal.log_warning(f"{PREFIXE} (avertissement) {msg}")

    def dossier(self, sous_dossier):
        return f"{self.cfg['racine_outliner']}/{sous_dossier}"

    def essayer_prop(self, obj, nom, valeur):
        """set_editor_property protégé : un nom de propriété absent de cette version ne casse rien."""
        try:
            obj.set_editor_property(nom, valeur)
            return True
        except Exception as e:  # noqa: BLE001
            self.avertir(f"propriété '{nom}' non réglée sur {obj.get_class().get_name()} : {e}")
            return False

    def finaliser_acteur(self, acteur, label, dossier, tags):
        acteur.set_actor_label(label)
        acteur.set_folder_path(dossier)
        tous = [self.tag] + [t for t in tags if t]
        acteur.set_editor_property("tags", [unreal.Name(t) for t in tous])

    # -- 1. map --------------------------------------------------------------
    def ouvrir_ou_creer_map(self):
        chemin = self.cfg["map"]
        if self.eal.does_asset_exist(chemin):
            log(f"Map existante, ouverture : {chemin}")
            if not self.les.load_level(chemin):
                raise RuntimeError(f"Impossible d'ouvrir {chemin}")
            self.supprimer_acteurs_generes()
        else:
            log(f"Création d'un niveau vide : {chemin}")
            try:
                ok = self.les.new_level(chemin, False)  # (asset_path, is_partitioned_world)
            except TypeError:
                ok = self.les.new_level(chemin)
            if not ok:
                raise RuntimeError(f"new_level a échoué pour {chemin}")
        monde = self.ues.get_editor_world()
        nom_monde = monde.get_path_name() if monde else "?"
        if chemin.split("/")[-1] not in nom_monde:
            raise RuntimeError(f"Le monde ouvert n'est pas la map attendue ({nom_monde}) : arrêt pour ne rien abîmer")
        return monde

    def supprimer_acteurs_generes(self):
        a_supprimer = [a for a in self.eas.get_all_level_actors() if a and a.actor_has_tag(self.tag)]
        if not a_supprimer:
            return
        try:
            self.eas.destroy_actors(a_supprimer)
        except Exception:  # noqa: BLE001 - repli acteur par acteur
            for a in a_supprimer:
                self.eas.destroy_actor(a)
        self.compter("supprimes (ancien greybox)", len(a_supprimer))
        log(f"{len(a_supprimer)} acteurs '{self.tag}' supprimés")

    # -- 2. matériaux ---------------------------------------------------------
    def preparer_materiaux(self):
        dossier = self.cfg["dossier_materiaux"]
        mel = unreal.MaterialEditingLibrary
        outils = unreal.AssetToolsHelpers.get_asset_tools()
        for nom, spec in self.d["materiaux"].items():
            try:
                chemin = f"{dossier}/{nom}"
                if self.eal.does_asset_exist(chemin):
                    self.materiaux[nom] = self.eal.load_asset(chemin)
                    self.compter("materiaux reutilises")
                    continue
                rgb = self.d["palette"][spec["couleur"]]["rgb"]
                mat = outils.create_asset(nom, dossier, unreal.Material, unreal.MaterialFactoryNew())
                if mat is None:
                    self.erreur(f"Création du matériau {nom} impossible")
                    continue
                couleur = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
                couleur.set_editor_property("constant", couleur_lineaire(rgb))
                if spec.get("translucide"):
                    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
                    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
                    mel.connect_material_property(couleur, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
                    opa = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 200)
                    opa.set_editor_property("r", float(spec.get("opacite", 0.2)))
                    mel.connect_material_property(opa, "", unreal.MaterialProperty.MP_OPACITY)
                else:
                    mel.connect_material_property(couleur, "", unreal.MaterialProperty.MP_BASE_COLOR)
                    rough = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 200)
                    rough.set_editor_property("r", float(spec.get("roughness", 0.85)))
                    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
                    if spec.get("specular") is not None:
                        spe = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -400, 300)
                        spe.set_editor_property("r", float(spec["specular"]))
                        mel.connect_material_property(spe, "", unreal.MaterialProperty.MP_SPECULAR)
                mel.recompile_material(mat)
                self.eal.save_loaded_asset(mat)
                self.materiaux[nom] = mat
                self.compter("materiaux crees")
            except Exception:  # noqa: BLE001
                self.erreur(f"Matériau {nom} : {traceback.format_exc()}")

    def materiau(self, nom):
        m = self.materiaux.get(nom)
        if m is None:
            self.avertir(f"matériau inconnu ou non créé : {nom}")
        return m

    # -- 3. meshes de base : vérification du pivot par les bounds --------------
    def preparer_meshes(self):
        for forme, chemin in self.cfg["meshes"].items():
            mesh = self.eal.load_asset(chemin)
            if mesh is None:
                self.erreur(f"Mesh de base introuvable : {chemin}")
                continue
            b = mesh.get_bounds()
            o, e = b.origin, b.box_extent
            taille = (2 * e.x, 2 * e.y, 2 * e.z)
            origine = (o.x, o.y, o.z)
            self.meshes[forme] = (mesh, taille, origine)
            centre = max(abs(c) for c in origine) < 0.5
            log_info(f"Mesh {forme} ({chemin}) : taille {taille}, centre des bounds {origine}"
                     f" -> pivot {'au centre' if centre else 'DÉCALÉ, correction appliquée'}")
            if not centre:
                log(f"Pivot décalé détecté sur {chemin} : offset {origine} compensé automatiquement")

    def transform_mesh(self, forme, centre, dims_locales, axe_local, yaw):
        """Calcule (location, rotator, scale) pour qu'un mesh de base remplisse dims autour de centre."""
        mesh, taille, origine = self.meshes[forme]
        dx, dy, dz = dims_locales
        pitch = roll = 0.0
        if forme == "cylindre" and axe_local == "X":
            pitch, voulu = 90.0, (dz, dy, dx)    # l'axe Z du mesh passe sur X
        elif forme == "cylindre" and axe_local == "Y":
            roll, voulu = 90.0, (dx, dz, dy)     # l'axe Z du mesh passe sur Y
        else:
            voulu = (dx, dy, dz)
        echelle = tuple((voulu[i] / taille[i]) if taille[i] > 1e-3 else 1.0 for i in range(3))
        decal_local = tuple(origine[i] * echelle[i] for i in range(3))
        decal_monde = tourner_vecteur(decal_local, pitch, yaw, roll)
        loc = tuple(centre[i] - decal_monde[i] for i in range(3))
        rot = unreal.Rotator(roll=roll, pitch=pitch, yaw=float(yaw))
        return vecteur(loc), rot, vecteur(echelle)

    def spawner_mesh(self, forme, centre, dims_locales, axe_local, yaw):
        if forme not in self.meshes:
            raise ValueError(f"forme inconnue : {forme}")
        loc, rot, echelle = self.transform_mesh(forme, centre, dims_locales, axe_local, yaw)
        acteur = self.eas.spawn_actor_from_object(self.meshes[forme][0], loc, rot)
        if acteur is None:
            return None
        acteur.set_actor_scale3d(echelle)
        # Contrôle a posteriori : le centre des bounds réels doit tomber sur le centre demandé.
        try:
            origine, _ = acteur.get_actor_bounds(False)
            delta = (centre[0] - origine.x, centre[1] - origine.y, centre[2] - origine.z)
            if max(abs(c) for c in delta) > 1.0:
                acteur.set_actor_location(vecteur((loc.x + delta[0], loc.y + delta[1], loc.z + delta[2])), False, False)
                if forme not in self.correction_bounds_signalee:
                    self.correction_bounds_signalee.add(forme)
                    log(f"Correction de centre appliquée sur un {forme} (delta {tuple(round(c, 1) for c in delta)})")
        except Exception as e:  # noqa: BLE001
            self.avertir(f"contrôle des bounds impossible : {e}")
        return acteur

    def appliquer_materiau(self, acteur, nom_mat):
        mat = self.materiau(nom_mat)
        if mat is not None:
            acteur.static_mesh_component.set_material(0, mat)

    # -- 4. éléments ---------------------------------------------------------
    def generer_elements(self):
        for el in self.d["elements"]:
            try:
                rot = el.get("rotation") or {}
                acteur = self.spawner_mesh(el["forme"], el["centre"], el["dims"], el.get("axe", "Z"),
                                           rot.get("yaw", 0))
                if acteur is None:
                    self.erreur(f"Spawn nul pour l'élément {el['id']}")
                    continue
                self.appliquer_materiau(acteur, el["materiau"])
                comp = acteur.static_mesh_component
                if not el.get("collision", True):
                    comp.set_collision_profile_name("NoCollision")
                    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
                self.finaliser_acteur(acteur, el["label"], self.dossier(el["dossier"]),
                                      [el["id"], "Role_" + _ascii(el["role"]), "Greybox_Element"])
                self.compter(f"elements/{el['dossier']}")
            except Exception:  # noqa: BLE001
                self.erreur(f"Élément {el.get('id', '?')} : {traceback.format_exc()}")

    # -- 5. prises ------------------------------------------------------------
    def generer_prises(self):
        dossier = self.dossier(self.cfg["dossier_prises"])
        for pr in self.d["prises"]:
            try:
                nx, ny, _ = pr["normale"]
                yaw = round(math.degrees(math.atan2(ny, nx)), 3)   # +X -> 0, +Y -> 90, -Y -> -90
                dx, dy, dz = pr["dims"]
                tourne = abs(math.sin(math.radians(yaw))) > 0.5
                dims_locales = (dy, dx, dz) if tourne else (dx, dy, dz)
                axe = pr.get("axe", "Z")
                if axe in ("X", "Y") and tourne:
                    axe = "Y" if axe == "X" else "X"
                acteur = self.spawner_mesh(pr["forme"], pr["centre"], dims_locales, axe, yaw)
                if acteur is None:
                    self.erreur(f"Spawn nul pour la prise {pr['id']}")
                    continue
                self.appliquer_materiau(acteur, pr["materiau"])
                tags = ["Prise", f"PriseType_{pr['type']}", f"PriseId_{pr['id']}", f"Prise:{pr['type']}",
                        f"Section_{pr['section']}", pr["id"]]
                if pr.get("sous_type"):
                    tags.append(f"PriseSousType_{pr['sous_type']}")
                if pr.get("mantle_autorise") is True:
                    tags.append("MantleAutorise")
                elif pr.get("mantle_autorise") is False:
                    tags.append("MantleInterdit")
                if pr.get("fragile"):
                    tags.append("Fragile")
                self.finaliser_acteur(acteur, pr["label"], dossier, tags)
                self.compter(f"prises/{pr['type']}")
            except Exception:  # noqa: BLE001
                self.erreur(f"Prise {pr.get('id', '?')} : {traceback.format_exc()}")

    # -- 6. marqueurs de gameplay ---------------------------------------------
    def generer_gameplay(self):
        dossier = self.dossier(self.cfg["dossier_gameplay"])
        for mk in self.d["gameplay"]:
            try:
                acteur = self._spawner_marqueur(mk)
                if acteur is None:
                    self.erreur(f"Spawn nul pour le marqueur {mk['id']}")
                    continue
                tags = [mk["id"], f"Gameplay_{mk['categorie']}", f"Type_{mk['type']}"]
                if mk.get("actif_au_depart") is False:
                    tags.append("ActiverAuCombat")
                self.finaliser_acteur(acteur, mk["id"], dossier, tags)
                self.compter(f"gameplay/{mk['type']}")
            except Exception:  # noqa: BLE001
                self.erreur(f"Marqueur {mk.get('id', '?')} : {traceback.format_exc()}")

    def _spawner_marqueur(self, mk):
        t = mk["type"]
        pos = list(mk["position"])
        rot = rotateur(mk.get("rotation"))
        if t == "PlayerStart":
            pos[2] += float(mk.get("decalage_z", 0))
            return self.eas.spawn_actor_from_class(unreal.PlayerStart, vecteur(pos), rot)
        if t == "TargetPoint":
            return self.eas.spawn_actor_from_class(unreal.TargetPoint, vecteur(pos), rot)
        if t == "TriggerBox":
            a = self.eas.spawn_actor_from_class(unreal.TriggerBox, vecteur(pos), rot)
            if a is not None:
                d = mk["dims"]
                boite = a.get_component_by_class(unreal.BoxComponent)
                boite.set_box_extent(unreal.Vector(d[0] / 2, d[1] / 2, d[2] / 2), True)
            return a
        if t == "TriggerSphere":
            a = self.eas.spawn_actor_from_class(unreal.TriggerSphere, vecteur(pos), rot)
            if a is not None:
                a.get_component_by_class(unreal.SphereComponent).set_sphere_radius(float(mk["rayon"]), True)
            return a
        if t == "CameraActor":
            a = self.eas.spawn_actor_from_class(unreal.CameraActor, vecteur(pos), rot)
            if a is not None and mk.get("fov"):
                cam = a.get_component_by_class(unreal.CameraComponent)
                cam.set_field_of_view(float(mk["fov"]))
            return a
        if t == "BlockingVolume":
            # Un ABlockingVolume demande un brush, peu pilotable en Python : on utilise un cube
            # invisible en jeu, profil de collision « mur invisible », matériau de volume.
            a = self.spawner_mesh("boite", pos, mk["dims"], "Z", mk.get("rotation", {}).get("yaw", 0))
            if a is not None:
                comp = a.static_mesh_component
                self.appliquer_materiau(a, "M_GB_Volume")
                comp.set_collision_profile_name(self.cfg.get("profil_collision_blocking", "InvisibleWall"))
                comp.set_cast_shadow(False)
                if mk.get("actif_au_depart") is False:
                    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
                a.set_actor_hidden_in_game(True)
            return a
        raise ValueError(f"type de marqueur inconnu : {t}")

    # -- 7. lumière d'aube ----------------------------------------------------
    def generer_lumiere(self):
        L = self.d["lumiere"]
        dossier = self.dossier(self.cfg["dossier_lumiere"])
        etapes = [("soleil", self._soleil), ("ciel_atmosphere", self._atmosphere), ("skylight", self._skylight),
                  ("brouillard", self._brouillard), ("nuages", self._nuages), ("post_process", self._post_process)]
        for cle, fn in etapes:
            spec = L.get(cle)
            if not spec:
                continue
            try:
                acteur = fn(spec)
                if acteur is None:
                    continue
                self.finaliser_acteur(acteur, spec["label"], dossier, ["Lumiere", spec["label"]])
                self.compter(f"lumiere/{cle}")
            except Exception:  # noqa: BLE001
                self.erreur(f"Lumière {cle} : {traceback.format_exc()}")

    def _spawn_classe(self, classe, spec):
        a = self.eas.spawn_actor_from_class(classe, vecteur(spec.get("position", [0, 0, 0])),
                                            rotateur(spec.get("rotation")))
        if a is None:
            self.erreur(f"Spawn nul : {classe.__name__}")
        return a

    def _soleil(self, s):
        a = self._spawn_classe(unreal.DirectionalLight, s)
        if a is None:
            return None
        c = a.get_component_by_class(unreal.DirectionalLightComponent)
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
        c.set_intensity(float(s["intensite_lux"]))
        c.set_light_color(couleur_lineaire(s["couleur_rgb"]), True)
        if s.get("utiliser_temperature"):
            self.essayer_prop(c, "use_temperature", True)
            c.set_temperature(float(s["temperature_k"]))
        try:
            c.set_atmosphere_sun_light(bool(s.get("atmosphere_sun_light", True)))
        except Exception:  # noqa: BLE001
            self.essayer_prop(c, "atmosphere_sun_light", bool(s.get("atmosphere_sun_light", True)))
        return a

    def _atmosphere(self, s):
        return self._spawn_classe(unreal.SkyAtmosphere, s)

    def _skylight(self, s):
        a = self._spawn_classe(unreal.SkyLight, s)
        if a is None:
            return None
        c = a.get_component_by_class(unreal.SkyLightComponent)
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
        self.essayer_prop(c, "real_time_capture", bool(s.get("real_time_capture", True)))
        c.set_intensity(float(s.get("intensite", 1.0)))
        return a

    def _brouillard(self, s):
        a = self._spawn_classe(unreal.ExponentialHeightFog, s)
        if a is None:
            return None
        c = a.get_component_by_class(unreal.ExponentialHeightFogComponent)
        c.set_fog_density(float(s["densite"]))
        c.set_fog_height_falloff(float(s.get("height_falloff", 0.2)))
        couleur = couleur_lineaire(self.d["palette"][s["couleur_diffusion"]]["rgb"])
        # Le nom a changé selon les versions (Color -> Luminance) : on essaie les deux.
        try:
            c.set_editor_property("fog_inscattering_luminance", couleur)
        except Exception:  # noqa: BLE001
            self.essayer_prop(c, "fog_inscattering_color", couleur)
        if s.get("volumetric_fog"):
            try:
                c.set_volumetric_fog(True)
            except Exception:  # noqa: BLE001
                self.essayer_prop(c, "enable_volumetric_fog", True)
        return a

    def _nuages(self, s):
        if not s.get("actif", True):
            return None
        if not hasattr(unreal, "VolumetricCloud"):
            self.avertir("VolumetricCloud indisponible dans cette version : ignoré")
            return None
        return self._spawn_classe(unreal.VolumetricCloud, s)

    def _post_process(self, s):
        a = self._spawn_classe(unreal.PostProcessVolume, s)
        if a is None:
            return None
        a.set_editor_property("unbound", bool(s.get("infini", True)))
        reglages = a.get_editor_property("settings")   # copie du struct
        for nom, val in s.get("reglages", {}).items():
            if isinstance(val, dict) and "enum" in val:
                try:
                    val = getattr(getattr(unreal, val["enum"]), val["valeur"])
                except AttributeError:
                    self.avertir(f"enum {val['enum']}.{val['valeur']} introuvable")
                    continue
            self.essayer_prop(reglages, nom, val)
        a.set_editor_property("settings", reglages)     # réaffectation obligatoire
        return a

    # -- 8. GameMode ----------------------------------------------------------
    def regler_game_mode(self, monde):
        chemin = self.cfg["game_mode_bp"]
        try:
            classe = self.eal.load_blueprint_class(chemin)
            if classe is None:
                self.avertir(f"GameMode {chemin} introuvable : override non réglé")
                return
            ws = monde.get_world_settings()
            ws.set_editor_property("default_game_mode", classe)
            log(f"GameMode Override = {chemin}")
        except Exception:  # noqa: BLE001
            self.erreur(f"GameMode Override : {traceback.format_exc()}")

    # -- 9. sauvegarde et résumé ----------------------------------------------
    def sauvegarder(self):
        try:
            if not self.les.save_current_level():
                self.erreur("save_current_level a renvoyé False")
        except Exception:  # noqa: BLE001
            self.erreur(f"Sauvegarde de la map : {traceback.format_exc()}")
        try:
            self.eal.save_directory(self.cfg["dossier_materiaux"], True, True)
        except Exception:  # noqa: BLE001
            self.erreur(f"Sauvegarde des matériaux : {traceback.format_exc()}")

    def resume(self):
        log("========== RÉSUMÉ ==========")
        totaux = {}
        for cle in sorted(self.compteurs):
            log(f"  {cle} : {self.compteurs[cle]}")
            racine = cle.split("/")[0]
            totaux[racine] = totaux.get(racine, 0) + self.compteurs[cle]
        for racine in ("elements", "prises", "gameplay", "lumiere"):
            log(f"  TOTAL {racine} : {totaux.get(racine, 0)}")
        meta = self.d.get("meta", {})
        if totaux.get("elements", 0) != meta.get("nb_elements_attendu"):
            log(f"  ATTENTION : {totaux.get('elements', 0)} éléments au lieu de {meta.get('nb_elements_attendu')}")
        if totaux.get("prises", 0) != meta.get("nb_prises_attendu"):
            log(f"  ATTENTION : {totaux.get('prises', 0)} prises au lieu de {meta.get('nb_prises_attendu')}")
        log(f"  Avertissements : {self.avertissements}")
        log(f"  ERREURS : {self.erreurs}")
        log("============================")

    def executer(self):
        monde = self.ouvrir_ou_creer_map()
        self.preparer_materiaux()
        self.preparer_meshes()
        self.generer_elements()
        self.generer_prises()
        self.generer_gameplay()
        self.generer_lumiere()
        self.regler_game_mode(monde)
        self.sauvegarder()
        self.resume()


def _ascii(texte):
    import unicodedata
    return unicodedata.normalize("NFKD", texte).encode("ascii", "ignore").decode()


def charger_json():
    for chemin in CHEMIN_JSON:
        if os.path.isfile(chemin):
            log(f"Données : {chemin}")
            with open(chemin, "r", encoding="utf-8") as f:
                return json.load(f)
    raise FileNotFoundError(f"{NOM_FICHIER_JSON} introuvable. Candidats : {CHEMIN_JSON}")


def main():
    try:
        GenerateurGreybox(charger_json()).executer()
    except Exception:  # noqa: BLE001
        log_erreur(f"Échec global : {traceback.format_exc()}")


main()

if os.environ.get("MANSA_AUTOQUIT") == "1":
    unreal.SystemLibrary.quit_editor()
