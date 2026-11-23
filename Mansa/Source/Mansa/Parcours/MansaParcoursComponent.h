// MANSA — Checkpoints et rattrapage du parcours (D6, GDD §4.4).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MansaParcoursComponent.generated.h"

class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMansaOnCheckpoint, FName, IdCheckpoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMansaOnRattrapage);

/** Un checkpoint du niveau (TargetPoint tagué). */
USTRUCT(BlueprintType)
struct FMansaCheckpoint
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mansa|Parcours")
	FName Id;

	/** Position au sol (pieds). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mansa|Parcours")
	FTransform Transform;

	/** Rang dans OrdreCheckpoints (Project Settings). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mansa|Parcours")
	int32 Ordre = 0;
};

/**
 * Pas de mort en escalade (GDD §4.4) : une chute dans un volume de rattrapage (ou sous
 * AltitudeRattrapage) déclenche un fondu au noir, un retour au dernier checkpoint atteint,
 * la remise en place des prises effritées et une réplique du narrateur.
 *
 * Checkpoints : TargetPoint tagués (TagCheckpoint), activés quand Sira passe à proximité, au sol,
 * dans l'ordre de progression. Le point de départ du joueur sert de checkpoint initial.
 * Réglages : Project Settings > Mansa > Escalade et parcours. Aucun Tick (timer).
 */
UCLASS(ClassGroup=(Mansa), meta=(BlueprintSpawnableComponent))
class UMansaParcoursComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMansaParcoursComponent();

	UPROPERTY(BlueprintAssignable, Category="Mansa|Parcours")
	FMansaOnCheckpoint OnCheckpointAtteint;

	/** Diffusé au moment du retour au checkpoint (écran noir). */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Parcours")
	FMansaOnRattrapage OnRattrapage;

	/** Lance le rattrapage (fondu, retour au checkpoint). Ignoré si un rattrapage est en cours. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Parcours")
	void DeclencherRattrapage();

	/** Téléporte au checkpoint d'identifiant Id (test), et en fait le checkpoint courant. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Parcours")
	bool AllerAuCheckpoint(FName Id);

	UFUNCTION(BlueprintPure, Category="Mansa|Parcours")
	FName GetCheckpointCourant() const { return Courant.Id; }

	UFUNCTION(BlueprintPure, Category="Mansa|Parcours")
	bool EstEnRattrapage() const { return bRattrapageEnCours; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	ACharacter* GetPersonnage() const;
	void CollecterCheckpoints();
	void LierVolumesRattrapage();
	void VerifierCheckpoints();
	void RetourAuCheckpoint();
	void FinRattrapage();
	void PlacerAu(const FMansaCheckpoint& Checkpoint);

	UFUNCTION()
	void OnVolumeRattrapage(AActor* Volume, AActor* Autre);

	TArray<FMansaCheckpoint> Checkpoints;
	FMansaCheckpoint Courant;
	bool bRattrapageEnCours = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> VolumesLies;

	FTimerHandle TimerVerification;
	FTimerHandle TimerRattrapage;
};
