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
