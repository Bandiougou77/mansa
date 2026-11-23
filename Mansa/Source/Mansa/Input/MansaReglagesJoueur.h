// MANSA — Réglages du joueur liés aux contrôles et au HUD (A6).
// Sauvegardés par joueur dans Saved/Config/<plateforme>/GameUserSettings.ini.
// Le menu Options (bloc H) lira et écrira ces valeurs ; en attendant : commandes console.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MansaInputTypes.h"
#include "MansaReglagesJoueur.generated.h"

UCLASS(Config=GameUserSettings, BlueprintType)
class UMansaReglagesJoueur : public UObject
{
	GENERATED_BODY()

public:
	/** Réglages courants (objet par défaut, modifiable). */
	UFUNCTION(BlueprintPure, Category="Mansa|Reglages", meta=(DisplayName="Get Mansa Reglages Joueur"))
	static UMansaReglagesJoueur* Get();

	/** Disposition du clavier choisie (Auto = détection). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Mansa|Reglages")
	EMansaDispositionClavier DispositionClavier = EMansaDispositionClavier::Auto;

	/** Icônes manette affichées. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Mansa|Reglages")
	EMansaStyleManette StyleManette = EMansaStyleManette::Xbox;

	/** Mode immersion : masque boussole et marqueurs de distance (GDD §9). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Mansa|Reglages")
	bool bModeImmersion = false;

	/** Affiche les prompts contextuels (« [Espace] Grimper »). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category="Mansa|Reglages")
	bool bAfficherPrompts = true;

	/** Disposition effective : la choisie, ou la détectée si Auto. */
	UFUNCTION(BlueprintPure, Category="Mansa|Reglages")
	EMansaDispositionClavier GetDispositionEffective() const;

	/** Détecte AZERTY / QWERTY d'après la disposition clavier du système (Windows). */
	UFUNCTION(BlueprintPure, Category="Mansa|Reglages")
	static EMansaDispositionClavier DetecterDispositionSysteme();

	/** Écrit les réglages sur disque. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Reglages")
	void Sauvegarder();
};
