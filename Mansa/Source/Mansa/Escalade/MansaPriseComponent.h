// MANSA — Prise d'escalade (D3, GDD §4.4).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MansaDataTypes.h"
#include "MansaPriseComponent.generated.h"

class UMansaPriseComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMansaOnPriseEvenement, UMansaPriseComponent*, Prise);

/** Surcharge d'un réglage hérité de la Data Table. */
UENUM(BlueprintType)
enum class EMansaSurcharge : uint8
{
	Herite,
	Oui,
	Non
};

/**
 * Fait d'un acteur une prise d'escalade.
 *
 * Géométrie (calculée au BeginPlay à partir des bounds de l'acteur) :
 *  - point de saisie = centre des bounds ;
 *  - Corniche = segment le long de la plus grande dimension horizontale ;
 *  - normale (vers l'extérieur du mur) = Yaw de l'acteur, ou tag PriseNormaleYaw_<degrés>.
 *
 * Réglages : ligne <TypePrise> de DT_PrisesEscalade (Project Settings > Mansa > Escalade et parcours).
 * Si bLireTagsActeur, le type, l'identifiant et le mantle sont lus dans les tags posés par le
 * générateur de greybox (PriseType_<Type>, PriseId_<Id>, MantleAutorise / MantleInterdit).
 *
 * BancoFragile : à la première accroche, la prise tremble puis s'effrite après DelaiEffritement
 * (masquée, sans collision) et diffuse OnEffritement. Reinitialiser() la remet en place (checkpoint).
 * Aucun Tick : le tremblement passe par un timer.
 */
UCLASS(ClassGroup=(Mansa), meta=(BlueprintSpawnableComponent))
class UMansaPriseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMansaPriseComponent();

	/** Type de prise (nom de ligne dans DT_PrisesEscalade). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	EMansaPriseType TypePrise = EMansaPriseType::Toron;

	/** Identifiant lisible (ex. D2-05), pour les logs et le debug. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	FName IdPrise;

	/** Surcharge du mantle de la DT pour CETTE prise (ex. CorA : Non). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	EMansaSurcharge MantleAutorise = EMansaSurcharge::Herite;

	/** Lit type / id / mantle / normale dans les tags de l'acteur (posés par le générateur). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	bool bLireTagsActeur = true;

	/** La capsule de Sira traverse la géométrie de la prise (petits reliefs) ; les traces Visibility la voient toujours. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	bool bIgnorerCollisionPawn = true;

	/** Période du timer de tremblement pendant l'effritement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade", meta=(ClampMin="0.01", Units="s"))
	float PeriodeTremblement = 0.03f;

	/** Diffusé quand une prise fragile commence à trembler. */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnPriseEvenement OnDebutEffritement;

	/** Diffusé quand la prise s'effrite (disparaît). */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnPriseEvenement OnEffritement;

	/** Réglages en vigueur (ligne de la DT, ou repli). */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	const FMansaPriseRow& GetReglages() const { return Reglages; }

	/** Normale horizontale, vers l'extérieur du mur. */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	FVector GetNormale() const { return Normale; }

	/** Point de saisie le plus proche de Point (centre, ou point du segment d'une corniche). */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	FVector GetPointLePlusProche(const FVector& Point) const;

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	FVector GetCentre() const { return Centre; }

	/** Vrai pour une prise linéaire (corniche). */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	bool EstSegment() const { return bSegment; }

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	void GetSegment(FVector& A, FVector& B) const { A = SegmentA; B = SegmentB; }

	/** Mantle autorisé (DT + surcharge). */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	bool PeutMantle() const;

	/** Saisissable maintenant (type saisissable, pas effritée). */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	bool EstDisponible() const { return Reglages.bSaisissable && !bEffritee; }

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	bool EstEffritee() const { return bEffritee; }

	/** Nom pour les logs : IdPrise, ou le label de l'acteur. */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	FString GetNomDebug() const;

	/** Appelé par l'escalade quand Sira s'accroche (déclenche l'effritement d'une BancoFragile). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void NotifierAccroche();

	/** Remet la prise dans son état initial (après un retour au checkpoint). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void Reinitialiser();

	/** Recalcule réglages et géométrie (si l'acteur a été déplacé ou la DT réimportée). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void Initialiser();

	/** Réglages de repli par type si la DT manque. */
	static FMansaPriseRow ReglagesDeRepli(EMansaPriseType Type);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void LireTags();
	void ChargerReglages();
	void CalculerGeometrie();
	void Trembler();
	void Effriter();

	FMansaPriseRow Reglages;
	FVector Centre = FVector::ZeroVector;
	FVector SegmentA = FVector::ZeroVector;
	FVector SegmentB = FVector::ZeroVector;
	FVector Normale = FVector::ForwardVector;
	bool bSegment = false;

	bool bEffritementLance = false;
	bool bEffritee = false;
	float TempsDebutEffritement = 0.f;
	FVector PositionInitiale = FVector::ZeroVector;
	FTimerHandle TimerEffritement;
	FTimerHandle TimerTremblement;
};
