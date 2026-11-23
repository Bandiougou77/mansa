// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MansaInputTypes.h"
#include "MansaPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMansaOnPeripheriqueChange, EMansaPeripherique, Peripherique);

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AMansaPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/** Suit le dernier périphérique utilisé (clavier/souris ou manette). */
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

	EMansaPeripherique Peripherique = EMansaPeripherique::ClavierSouris;

public:

	/** Dernier périphérique utilisé : choisit les libellés et icônes des prompts. */
	UFUNCTION(BlueprintPure, Category="Mansa|Controles")
	EMansaPeripherique GetPeripherique() const { return Peripherique; }

	UPROPERTY(BlueprintAssignable, Category="Mansa|Controles")
	FMansaOnPeripheriqueChange OnPeripheriqueChange;

	/** Console : « MansaClavier Auto | AZERTY | QWERTY » (sauvegardé). */
	UFUNCTION(Exec)
	void MansaClavier(const FString& Disposition);

	/** Console : « MansaManette Xbox | PlayStation » (sauvegardé). */
	UFUNCTION(Exec)
	void MansaManette(const FString& Style);

	/** Console : « MansaImmersion 1 » masque boussole et marqueurs (sauvegardé). */
	UFUNCTION(Exec)
	void MansaImmersion(bool bActif);

	/** Console : « MansaControles » logue les contextes actifs et la touche de chaque action (clavier et manette). */
	UFUNCTION(Exec)
	void MansaControles();

};
