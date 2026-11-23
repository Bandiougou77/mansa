// MANSA — Registre des prises d'escalade du monde (D3).

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MansaPrisesSubsystem.generated.h"

class UMansaPriseComponent;

/**
 * Liste des prises présentes dans le monde (l'escalade ne parcourt qu'elle : pas d'itération
 * d'acteurs à chaque saut), remise en place après un checkpoint, affichage de debug.
 *
 * Repli : au démarrage, tout acteur portant le tag de prise (Project Settings) mais sans
 * UMansaPriseComponent en reçoit un (les tags du générateur donnent son type).
 */
UCLASS()
class UMansaPrisesSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UMansaPrisesSubsystem* Get(const UObject* ContexteMonde);

	void Enregistrer(UMansaPriseComponent* Prise);
	void Desenregistrer(UMansaPriseComponent* Prise);

	/** Prises valides du monde. */
	TArray<UMansaPriseComponent*> GetPrises() const;

	/** Prise par identifiant (ex. D2-05), nullptr si absente. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	UMansaPriseComponent* TrouverPrise(FName IdPrise) const;

	/** Remet toutes les prises (effritées) en place. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void ReinitialiserPrises();

	/** Affiche le type et l'identifiant de chaque prise (sphère + texte), rafraîchi par timer. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void SetDebugPrises(bool bActif);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void DessinerDebug();

	TArray<TWeakObjectPtr<UMansaPriseComponent>> Prises;
	FTimerHandle TimerDebug;
};
