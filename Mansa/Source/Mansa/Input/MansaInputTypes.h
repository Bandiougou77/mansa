// MANSA — Types des contrôles : périphérique, disposition clavier, libellés de touches, prompts (A6).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MansaInputTypes.generated.h"

class UInputAction;
class UTexture2D;

/** Dernier périphérique utilisé par le joueur (choisit les libellés et icônes affichés). */
UENUM(BlueprintType)
enum class EMansaPeripherique : uint8
{
	ClavierSouris,
	Manette
};

/** Disposition du clavier : détermine le profil de touches (ZQSD + A ou WASD + Q). */
UENUM(BlueprintType)
enum class EMansaDispositionClavier : uint8
{
	/** Détectée à partir de la disposition Windows. */
	Auto,
	AZERTY,
	QWERTY
};

/** Famille d'icônes manette. */
UENUM(BlueprintType)
enum class EMansaStyleManette : uint8
{
	Xbox,
	PlayStation
};

/**
 * Une ligne de DT_LibellesTouches (design/data/DT_LibellesTouches.csv). Nom de ligne = nom de la
 * touche Unreal (SpaceBar, Gamepad_FaceButton_Bottom...). Les icônes sont optionnelles : sans
 * icône, le prompt affiche le libellé dans un cadre.
 */
USTRUCT(BlueprintType)
struct FMansaLibelleToucheRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Libellé clavier / souris (« Espace », « Clic gauche »). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	FText Clavier;

	/** Libellé manette Xbox (« A », « RB »). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	FText Xbox;

	/** Libellé manette PlayStation (« Croix », « R1 »). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	FText PlayStation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	TSoftObjectPtr<UTexture2D> IconeClavier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	TSoftObjectPtr<UTexture2D> IconeXbox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	TSoftObjectPtr<UTexture2D> IconePlayStation;
};

/** Un prompt contextuel à afficher (« [Espace] Grimper »). */
USTRUCT(BlueprintType)
struct FMansaPrompt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	TObjectPtr<const UInputAction> Action = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Controles")
	FText Verbe;
};
