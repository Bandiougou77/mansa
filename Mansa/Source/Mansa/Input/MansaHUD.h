// MANSA — HUD de la slice : prompts contextuels (A6). Boussole, objectif et distances viendront en bloc H.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MansaHUD.generated.h"

/**
 * Dessine les prompts contextuels en bas à droite : « [Espace] Grimper » au clavier,
 * « [A] Grimper » / « [Croix] Grimper » à la manette, d'après le dernier périphérique utilisé,
 * la touche réellement mappée (remaps compris) et DT_LibellesTouches. Une icône est dessinée
 * si la ligne de la DT en fournit une ; sinon le libellé est encadré.
 * Version Canvas volontairement simple : l'UMG final (style bandes tissées) arrive en bloc H.
 */
UCLASS()
class AMansaHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	/** Marge depuis le bord bas droit de l'écran (pixels, à 1080p). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|HUD")
	FVector2D MargePrompts = FVector2D(60.f, 70.f);

	/** Espace vertical entre deux prompts (pixels, à 1080p). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|HUD")
	float InterlignePrompts = 14.f;

	/** Taille des icônes de touches (pixels, à 1080p). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|HUD")
	float TailleIcone = 32.f;

	/** Couleur du cadre de touche (sans icône) et du texte. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|HUD")
	FLinearColor CouleurCadre = FLinearColor(0.95f, 0.85f, 0.6f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|HUD")
	FLinearColor CouleurTexte = FLinearColor(1.f, 0.97f, 0.9f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|HUD")
	FLinearColor CouleurFond = FLinearColor(0.f, 0.f, 0.f, 0.45f);
};
