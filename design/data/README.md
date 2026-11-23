# Données de design (sources CSV des Data Tables)

Les **CSV de ce dossier sont la source de vérité** des Data Tables du jeu. Ils sont importés
dans `/Game/Mansa/Data/` par `tools/unreal/a5_importer_data_tables.py`.

| CSV | Data Table | Struct C++ | Usage |
|---|---|---|---|
| `DT_CharacterMovement.csv` | `DT_CharacterMovement` | `FMansaMovementRow` | Vitesses, saut, enjambement / mantle (GDD §4.3). Ligne lue par défaut : `Defaut` (réglable sur BP_Sira, `Movement Row Name`) |
| `DT_CameraProfiles.csv` | `DT_CameraProfiles` | `FMansaCameraProfileRow` | Profils caméra `Exploration`, `Escalade`, `Combat` (GDD §4.5) |

## Modifier une valeur

1. Ouvre le CSV (séparateur **virgule**, point décimal), change la valeur.
2. Réimporte : dans l'éditeur, clic droit sur la DT > **Reimport**, ou relance le script
   (`py "C:/Dev/Mansa/tools/unreal/a5_importer_data_tables.py"` dans la barre Cmd).
3. Play : la valeur est prise en compte, sans recompiler.

Une modification faite directement dans l'éditeur de Data Table est **écrasée** au prochain import :
reporte-la dans le CSV.

## Formats

- Première colonne `---` = nom de la ligne (clé).
- Booléens : `True` / `False`.
- Vecteurs : `"(X=0.0,Y=50.0,Z=40.0)"` entre guillemets.
- Unités : cm, cm/s, secondes, degrés.
