// MANSA — HUD de la slice : prompts contextuels (A6).

#include "MansaHUD.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "MansaCharacter.h"
#include "MansaControlesLibrary.h"
#include "MansaPlayerController.h"
#include "MansaReglagesJoueur.h"

void AMansaHUD::DrawHUD()
{
	Super::DrawHUD();

	const UMansaReglagesJoueur* Reglages = UMansaReglagesJoueur::Get();
	AMansaPlayerController* PC = Cast<AMansaPlayerController>(PlayerOwner);
	const AMansaCharacter* Sira = PC ? Cast<AMansaCharacter>(PC->GetPawn()) : nullptr;
	if (!Canvas || !Sira || !Reglages->bAfficherPrompts)
	{
		return;
	}
	TArray<FMansaPrompt> Prompts;
	Sira->GetPromptsContextuels(Prompts);
	if (Prompts.IsEmpty())
	{
		return;
	}

	UFont* Police = GEngine->GetLargeFont();
	const float Echelle = Canvas->ClipY / 1080.f;
	const float Icone = TailleIcone * Echelle;
	float Y = Canvas->ClipY - MargePrompts.Y * Echelle;
	const float XDroite = Canvas->ClipX - MargePrompts.X * Echelle;

	// Du bas vers le haut : le premier prompt est le plus bas.
	for (const FMansaPrompt& Prompt : Prompts)
	{
		FKey Touche;
		if (!UMansaControlesLibrary::TrouverToucheAction(PC, Prompt.Action, PC->GetPeripherique(), Touche))
		{
			continue;
		}
		UTexture2D* Texture = nullptr;
		const FString Libelle = UMansaControlesLibrary::LibelleTouche(Touche, Reglages->StyleManette, Texture).ToString();
		const FString Verbe = Prompt.Verbe.ToString();

		float LVerbe, HVerbe, LTouche, HTouche;
		Canvas->TextSize(Police, Verbe, LVerbe, HVerbe, Echelle, Echelle);
		Canvas->TextSize(Police, Libelle, LTouche, HTouche, Echelle, Echelle);
		const float Pad = 8.f * Echelle;
		const float LargeurTouche = Texture ? Icone : LTouche + 2.f * Pad;
		const float Hauteur = FMath::Max(Icone, HTouche + Pad);
		const float Largeur = LargeurTouche + Pad + LVerbe + 2.f * Pad;
		const float X = XDroite - Largeur;
		const float YHaut = Y - Hauteur;

		// Fond translucide pour la lisibilité sur un décor clair (banco au soleil).
		DrawRect(CouleurFond, X - Pad, YHaut - Pad * 0.5f, Largeur + Pad, Hauteur + Pad);

		const float XTouche = X + Pad;
		if (Texture)
		{
			DrawTextureSimple(Texture, XTouche, YHaut, Icone / FMath::Max(1, Texture->GetSizeX()));
		}
		else
		{
			// Cadre de touche dessiné (en attendant les icônes) + libellé centré.
			FCanvasBoxItem Cadre(FVector2D(XTouche, YHaut), FVector2D(LargeurTouche, Hauteur));
			Cadre.SetColor(CouleurCadre);
			Cadre.LineThickness = 2.f * Echelle;
			Canvas->DrawItem(Cadre);
			DrawText(Libelle, CouleurCadre, XTouche + Pad, YHaut + (Hauteur - HTouche) * 0.5f, Police, Echelle);
		}
		DrawText(Verbe, CouleurTexte, XTouche + LargeurTouche + Pad, YHaut + (Hauteur - HVerbe) * 0.5f, Police, Echelle);

		Y = YHaut - InterlignePrompts * Echelle;
	}
}
