// MANSA — Structures de lignes des Data Tables du socle (A5).
// Sources CSV : /design/data/*.csv (voir /design/data/README.md), import par
// tools/unreal/a5_importer_data_tables.py. Les valeurs par défaut ci-dessous ne
// servent que de REPLI si la Data Table manque (un warning est alors logué).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MansaDataTypes.generated.h"

/**
 * Une ligne de DT_CharacterMovement (GDD §4.3).
 * Unités : cm, cm/s, secondes.
 */
USTRUCT(BlueprintType)
struct FMansaMovementRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Vitesse en marche lente (maintien Alt / stick partiel). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement", meta=(ClampMin="0", Units="cm/s"))
	float VitesseMarche = 200.f;

	/** Vitesse de jogging (allure par défaut). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement", meta=(ClampMin="0", Units="cm/s"))
	float VitesseJogging = 400.f;

	/** Vitesse de course (maintien Maj / L3). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement", meta=(ClampMin="0", Units="cm/s"))
	float VitesseCourse = 650.f;

	/** Hauteur de saut visée ; la vitesse verticale est calculée à partir de la gravité. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement", meta=(ClampMin="0", Units="cm"))
	float HauteurSaut = 120.f;

	/** Contrôle en l'air (0 = aucun, 1 = total). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement", meta=(ClampMin="0", ClampMax="1"))
	float ControleEnLair = 0.35f;

	/** Vitesse de rotation du personnage vers la direction de déplacement (degrés/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement", meta=(ClampMin="0"))
	float VitesseRotation = 500.f;

	/** Si vrai, pousser le stick à fond fait courir (GDD §4.2 « ou stick à fond »). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement")
	bool bCourseStickAFond = false;

	/** Seuil d'inclinaison du stick (0-1) au-delà duquel on court si bCourseStickAFond. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement", meta=(ClampMin="0", ClampMax="1"))
	float SeuilStickCourse = 0.95f;

	/** Obstacle enjambé automatiquement au saut si sa hauteur est inférieure ou égale (muret). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0", Units="cm"))
	float HauteurEnjambementMax = 90.f;

	/** Hauteur maximale d'un rebord accessible en mantle (au-dessus de l'enjambement). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0", Units="cm"))
	float HauteurMantleMax = 180.f;

	/** Durée d'un enjambement (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0.05", Units="s"))
	float DureeEnjambement = 0.45f;

	/** Durée d'un mantle (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0.05", Units="s"))
	float DureeMantle = 0.7f;
};

/**
 * Une ligne de DT_CameraProfiles (GDD §4.5). Nom de ligne = nom du profil
 * (Exploration, Escalade, Combat).
 */
USTRUCT(BlueprintType)
struct FMansaCameraProfileRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Longueur du bras de caméra. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Camera", meta=(ClampMin="0", Units="cm"))
	float LongueurBras = 300.f;

	/** Champ de vision horizontal (degrés). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Camera", meta=(ClampMin="5", ClampMax="170"))
	float FOV = 85.f;

	/** Décalage de la caméra au bout du bras (Y positif = épaule droite, Z = hauteur). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Camera")
	FVector DecalageCamera = FVector(0.f, 50.f, 40.f);

	/** Durée de la transition VERS ce profil (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Camera", meta=(ClampMin="0", Units="s"))
	float DureeTransition = 0.4f;
};

/**
 * Types de prises d'escalade (GDD §4.4). Le nom de chaque valeur est aussi le nom de la
 * ligne correspondante dans DT_PrisesEscalade, et le suffixe du tag acteur PriseType_<Type>.
 */
UENUM(BlueprintType)
enum class EMansaPriseType : uint8
{
	/** Poutre de rônier ou gouttière : prise sûre, suspension illimitée. */
	Toron,
	/** Arête, saillie de contrefort, rebord de fenêtre en banco : prise standard. */
	BancoRelief,
	/** Banco érodé : s'effrite après DelaiEffritement secondes de suspension. */
	BancoFragile,
	/** Rebord horizontal : déplacement latéral + mantle vers le haut (si autorisé). */
	Corniche,
	/** Réservé Phase 2 (saisons, crépi frais) : non saisissable dans la slice. */
	Reserve_CrepiFrais
};

/**
 * Une ligne de DT_PrisesEscalade (GDD §4.4, fiche djenne-slice §10). Nom de ligne = type de prise.
 * Les distances de saut sont celles de la prise VISÉE.
 */
USTRUCT(BlueprintType)
struct FMansaPriseRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Faux = la prise est ignorée par l'escalade (ex. Reserve_CrepiFrais). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	bool bSaisissable = true;

	/** Écart vertical max pour sauter vers cette prise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0", Units="cm"))
	float DistanceMaxVerticale = 150.f;

	/** Écart horizontal max pour sauter vers cette prise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0", Units="cm"))
	float DistanceMaxLaterale = 200.f;

	/** Hauteur max de la prise au-dessus des pieds pour la saisir depuis le sol (saut). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0", Units="cm"))
	float HauteurSaisieDepuisSol = 180.f;

	/** Distance horizontale max (centre de Sira -> prise) pour la saisir depuis le sol. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0", Units="cm"))
	float DistanceSaisieDepuisSol = 170.f;

	/** > 0 : la prise s'effrite ce nombre de secondes après la première accroche (BancoFragile). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0", Units="s"))
	float DelaiEffritement = 0.f;

	/** Amplitude max du tremblement de la prise pendant l'effritement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0", Units="cm"))
	float AmplitudeTremblement = 0.f;

	/** Vrai = on peut se déplacer le long de la prise au stick (Corniche). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	bool bDeplacementLateral = false;

	/** Vitesse de déplacement le long d'une corniche. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0", Units="cm/s"))
	float VitesseCorniche = 60.f;

	/** Mantle vers le haut autorisé par défaut (chaque prise peut le surcharger). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	bool bMantleAutorise = false;

	/** Durée du saut (interpolé) vers cette prise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0.05", Units="s"))
	float DureeSaut = 0.35f;
};
