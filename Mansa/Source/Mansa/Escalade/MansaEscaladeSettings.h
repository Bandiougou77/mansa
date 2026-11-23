// MANSA — Réglages projet de l'escalade et du parcours (D3-D6).
// Modifiables sans recompiler : Project Settings > Mansa > Escalade et parcours
// (enregistrés dans Config/DefaultGame.ini).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MansaEscaladeSettings.generated.h"

class UDataTable;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Escalade et parcours"))
class UMansaEscaladeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMansaEscaladeSettings();

	virtual FName GetCategoryName() const override { return TEXT("Mansa"); }

	/** Table des réglages par type de prise (struct FMansaPriseRow). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Escalade", meta=(RequiredAssetDataTags="RowStructure=/Script/Mansa.MansaPriseRow"))
	TSoftObjectPtr<UDataTable> TablePrisesEscalade;

	/** Tag des acteurs prises (le générateur de greybox le pose). Sert au repli si le composant manque. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Escalade")
	FName TagPrise = TEXT("Prise");

	/** Tag des checkpoints (TargetPoint). Le générateur de greybox pose Gameplay_Checkpoint. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours")
	FName TagCheckpoint = TEXT("Gameplay_Checkpoint");

	/**
	 * Ordre de progression des checkpoints (tag d'identifiant de chaque TargetPoint, ex. CP1).
	 * Un checkpoint n'est activé que s'il est plus loin dans cette liste que le courant ;
	 * un checkpoint absent de la liste passe en dernier.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours")
	TArray<FName> OrdreCheckpoints;

	/** Tag des volumes de rattrapage (TriggerBox). Le générateur pose l'identifiant KV_Rattrapage. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours")
	FName TagRattrapage = TEXT("KV_Rattrapage");

	/** Filet de sécurité : sous cette altitude, rattrapage même sans volume. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours", meta=(Units="cm"))
	float AltitudeRattrapage = -1000.f;

	/** Distance au TargetPoint qui active un checkpoint. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours", meta=(ClampMin="10", Units="cm"))
	float RayonActivationCheckpoint = 250.f;

	/** Période de la vérification de proximité des checkpoints (timer, pas de Tick). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours", meta=(ClampMin="0.05", Units="s"))
	float PeriodeVerificationCheckpoint = 0.2f;

	/** Durée du fondu au noir (et du retour) lors d'un rattrapage (GDD §4.4 : 0,5 s). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours", meta=(ClampMin="0", Units="s"))
	float DureeFonduRattrapage = 0.5f;

	/** Réplique du narrateur au rattrapage (placeholder en attendant le système du griot). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours")
	FText RepliqueRattrapage;

	/** Durée d'affichage de la réplique à l'écran (debug, en attendant l'UI). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Parcours", meta=(ClampMin="0", Units="s"))
	float DureeAffichageReplique = 3.f;
};
