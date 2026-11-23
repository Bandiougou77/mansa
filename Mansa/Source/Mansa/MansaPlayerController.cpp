// Copyright Epic Games, Inc. All Rights Reserved.


#include "MansaPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Mansa.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "InputKeyEventArgs.h"
#include "MansaCharacter.h"
#include "MansaControlesLibrary.h"
#include "MansaReglagesJoueur.h"

void AMansaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogMansa, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AMansaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AMansaPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

bool AMansaPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	// Une touche pressée, ou un axe assez franc (le bruit d'un stick au repos ne compte pas).
	const bool bFranc = Params.Event == IE_Pressed
		|| (Params.Event == IE_Axis && FMath::Abs(Params.AmountDepressed) >= GetDefault<UMansaControlesSettings>()->SeuilAnalogiqueBascule);
	if (bFranc && !Params.Key.IsTouch())
	{
		const EMansaPeripherique Nouveau = Params.Key.IsGamepadKey() ? EMansaPeripherique::Manette : EMansaPeripherique::ClavierSouris;
		if (Nouveau != Peripherique)
		{
			Peripherique = Nouveau;
			UE_LOG(LogMansa, Log, TEXT("Contrôles : périphérique -> %s"), Nouveau == EMansaPeripherique::Manette ? TEXT("manette") : TEXT("clavier/souris"));
			OnPeripheriqueChange.Broadcast(Nouveau);
		}
	}
	return Super::InputKey(Params);
}

void AMansaPlayerController::MansaClavier(const FString& Disposition)
{
	UMansaReglagesJoueur* R = UMansaReglagesJoueur::Get();
	if (Disposition.Equals(TEXT("AZERTY"), ESearchCase::IgnoreCase))
	{
		R->DispositionClavier = EMansaDispositionClavier::AZERTY;
	}
	else if (Disposition.Equals(TEXT("QWERTY"), ESearchCase::IgnoreCase))
	{
		R->DispositionClavier = EMansaDispositionClavier::QWERTY;
	}
	else
	{
		R->DispositionClavier = EMansaDispositionClavier::Auto;
	}
	R->Sauvegarder();
	if (AMansaCharacter* Sira = Cast<AMansaCharacter>(GetPawn()))
	{
		Sira->AppliquerDispositionClavier();
	}
}

void AMansaPlayerController::MansaManette(const FString& Style)
{
	UMansaReglagesJoueur* R = UMansaReglagesJoueur::Get();
	R->StyleManette = Style.Equals(TEXT("PlayStation"), ESearchCase::IgnoreCase) || Style.Equals(TEXT("PS"), ESearchCase::IgnoreCase)
		? EMansaStyleManette::PlayStation : EMansaStyleManette::Xbox;
	R->Sauvegarder();
	UE_LOG(LogMansa, Log, TEXT("Contrôles : icônes manette %s"), R->StyleManette == EMansaStyleManette::PlayStation ? TEXT("PlayStation") : TEXT("Xbox"));
}

void AMansaPlayerController::MansaImmersion(bool bActif)
{
	UMansaReglagesJoueur* R = UMansaReglagesJoueur::Get();
	R->bModeImmersion = bActif;
	R->Sauvegarder();
	UE_LOG(LogMansa, Log, TEXT("Mode immersion : %s"), bActif ? TEXT("activé (boussole et marqueurs masqués)") : TEXT("désactivé"));
}

void AMansaPlayerController::MansaControles()
{
	const UMansaReglagesJoueur* R = UMansaReglagesJoueur::Get();
	UE_LOG(LogMansa, Log, TEXT("Contrôles : disposition réglée %s, détectée %s, effective %s ; icônes %s ; périphérique %s"),
		*StaticEnum<EMansaDispositionClavier>()->GetNameStringByValue((int64)R->DispositionClavier),
		*StaticEnum<EMansaDispositionClavier>()->GetNameStringByValue((int64)UMansaReglagesJoueur::DetecterDispositionSysteme()),
		*StaticEnum<EMansaDispositionClavier>()->GetNameStringByValue((int64)R->GetDispositionEffective()),
		R->StyleManette == EMansaStyleManette::PlayStation ? TEXT("PlayStation") : TEXT("Xbox"),
		Peripherique == EMansaPeripherique::Manette ? TEXT("manette") : TEXT("clavier/souris"));
	if (AMansaCharacter* Sira = Cast<AMansaCharacter>(GetPawn()))
	{
		for (const UInputAction* Action : Sira->GetToutesLesActions())
		{
			FString Ligne = GetNameSafe(Action);
			for (const EMansaPeripherique P : { EMansaPeripherique::ClavierSouris, EMansaPeripherique::Manette })
			{
				FKey Touche;
				UTexture2D* Icone = nullptr;
				const FString Libelle = UMansaControlesLibrary::TrouverToucheAction(this, Action, P, Touche)
					? UMansaControlesLibrary::LibelleTouche(Touche, R->StyleManette, Icone).ToString() + TEXT(" (") + Touche.ToString() + TEXT(")")
					: FString(TEXT("-"));
				Ligne += FString::Printf(TEXT(" | %s : %s"), P == EMansaPeripherique::Manette ? TEXT("manette") : TEXT("clavier"), *Libelle);
			}
			UE_LOG(LogMansa, Log, TEXT("Contrôles : %s"), *Ligne);
		}
		UE_LOG(LogMansa, Log, TEXT("Contrôles : prompts affichés maintenant : %s"), *Sira->DecrirePrompts(Peripherique));
	}
}
