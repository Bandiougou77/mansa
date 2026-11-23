// MANSA — Outils des contrôles (A6).

#include "MansaControlesLibrary.h"
#include "Engine/DataTable.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"

UMansaControlesSettings::UMansaControlesSettings()
{
	TableLibellesTouches = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Mansa/Data/DT_LibellesTouches.DT_LibellesTouches")));
}

bool UMansaControlesLibrary::TrouverToucheAction(const APlayerController* PC, const UInputAction* Action, EMansaPeripherique Peripherique, FKey& OutTouche)
{
	if (!PC || !Action || !PC->GetLocalPlayer())
	{
		return false;
	}
	const UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Sub)
	{
		return false;
	}
	const bool bManette = Peripherique == EMansaPeripherique::Manette;
	// Clés des contextes actifs, remaps du joueur appliqués, dans l'ordre de priorité.
	for (const FKey& Touche : Sub->QueryKeysMappedToAction(Action))
	{
		if (Touche.IsGamepadKey() == bManette && !Touche.IsTouch())
		{
			OutTouche = Touche;
			return true;
		}
	}
	return false;
}

FText UMansaControlesLibrary::LibelleTouche(const FKey& Touche, EMansaStyleManette Style, UTexture2D*& OutIcone)
{
	OutIcone = nullptr;
	const UMansaControlesSettings* Settings = GetDefault<UMansaControlesSettings>();
	const UDataTable* Table = Settings->TableLibellesTouches.LoadSynchronous();
	const FMansaLibelleToucheRow* Row = Table ? Table->FindRow<FMansaLibelleToucheRow>(Touche.GetFName(), TEXT("MansaLibelle"), false) : nullptr;
	if (Row)
	{
		const bool bManette = Touche.IsGamepadKey();
		const FText& Libelle = !bManette ? Row->Clavier : (Style == EMansaStyleManette::PlayStation ? Row->PlayStation : Row->Xbox);
		const TSoftObjectPtr<UTexture2D>& Icone = !bManette ? Row->IconeClavier : (Style == EMansaStyleManette::PlayStation ? Row->IconePlayStation : Row->IconeXbox);
		OutIcone = Icone.IsNull() ? nullptr : Icone.LoadSynchronous();
		if (!Libelle.IsEmpty())
		{
			return Libelle;
		}
	}
	// Repli : nom d'affichage du moteur (lettres : « E », « V »...).
	return Touche.GetDisplayName(false);
}
