// MANSA — Outils des contrôles : touche d'une action, libellé et icône selon le périphérique (A6).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputCoreTypes.h"
#include "MansaInputTypes.h"
#include "MansaControlesLibrary.generated.h"

class APlayerController;
class UDataTable;
class UInputAction;
class UTexture2D;

/** Réglages projet des contrôles : Project Settings > Mansa > Contrôles et interface. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Contrôles et interface"))
class UMansaControlesSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMansaControlesSettings();

	virtual FName GetCategoryName() const override { return TEXT("Mansa"); }

	/** Libellés et icônes des touches (struct FMansaLibelleToucheRow). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Controles", meta=(RequiredAssetDataTags="RowStructure=/Script/Mansa.MansaLibelleToucheRow"))
	TSoftObjectPtr<UDataTable> TableLibellesTouches;

	/** Une entrée analogique (stick, gâchette) plus faible que ce seuil ne fait pas basculer le périphérique. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Controles", meta=(ClampMin="0", ClampMax="1"))
	float SeuilAnalogiqueBascule = 0.3f;
};

UCLASS()
class UMansaControlesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Touche actuellement associée à Action pour ce périphérique (contextes actifs du joueur, remaps
	 * compris). Faux si aucune touche de ce périphérique n'est mappée.
	 */
	UFUNCTION(BlueprintCallable, Category="Mansa|Controles")
	static bool TrouverToucheAction(const APlayerController* PC, const UInputAction* Action, EMansaPeripherique Peripherique, FKey& OutTouche);

	/** Libellé (et icône éventuelle) d'une touche pour le périphérique et le style de manette donnés. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Controles")
	static FText LibelleTouche(const FKey& Touche, EMansaStyleManette Style, UTexture2D*& OutIcone);
};
