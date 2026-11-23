// MANSA — Enjambement et mantle (D2, GDD §4.3).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "MansaDataTypes.h"
#include "MansaTraversalComponent.generated.h"

class ACharacter;

/** Type de franchissement détecté. */
UENUM(BlueprintType)
enum class EMansaFranchissement : uint8
{
	Aucun,
	/** Obstacle bas et fin (muret ≤ 90 cm) : on passe par-dessus et on retombe de l'autre côté. */
	Enjambement,
	/** Obstacle bas mais profond (caisse, estrade) : on monte dessus. */
	MonteeBasse,
	/** Rebord haut (90-180 cm) : on se hisse dessus. */
	Mantle
};

/** Résultat de l'analyse d'un obstacle devant le personnage. */
USTRUCT(BlueprintType)
struct FMansaAnalyseObstacle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Mansa|Franchissement")
	EMansaFranchissement Type = EMansaFranchissement::Aucun;

	/** Hauteur du sommet de l'obstacle au-dessus des pieds (cm). */
	UPROPERTY(BlueprintReadOnly, Category="Mansa|Franchissement")
	float Hauteur = 0.f;

	/** Point d'impact sur la face avant de l'obstacle. */
	UPROPERTY(BlueprintReadOnly, Category="Mansa|Franchissement")
	FVector PointMur = FVector::ZeroVector;

	/** Normale de la face avant (horizontale, orientée vers le personnage). */
	UPROPERTY(BlueprintReadOnly, Category="Mansa|Franchissement")
	FVector NormaleMur = FVector::ZeroVector;

	/** Point du sommet où la main se pose (cible de Motion Warping plus tard). */
	UPROPERTY(BlueprintReadOnly, Category="Mansa|Franchissement")
	FVector PointSommet = FVector::ZeroVector;

	/** Trajet du centre de la capsule (départ inclus). */
	TArray<FVector> Trajet;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMansaOnFranchissementDebut, EMansaFranchissement, Type, const FMansaAnalyseObstacle&, Analyse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMansaOnFranchissementFin, EMansaFranchissement, Type);

/**
 * Enjambement (≤ 90 cm) et mantle (90-180 cm) sans animation dédiée.
 *
 * Principe :
 *  1. traces horizontales devant la capsule pour trouver une face d'obstacle ;
 *  2. trace verticale juste derrière cette face pour trouver le sommet (hauteur) ;
 *  3. pour un obstacle bas, trace depuis l'arrière pour savoir s'il est fin (enjambement)
 *     ou profond (montée) ;
 *  4. tests de capsule (balayage + chevauchement) pour garantir que le trajet est libre ;
 *  5. déplacement interpolé du personnage le long du trajet (mode de mouvement Flying,
 *     sans collision balayée), puis retour en chute libre.
 *
 * Motion Warping : quand une anim de mantle existera, il suffira de jouer un montage sur
 * OnFranchissementDebut avec PointMur / PointSommet comme cibles de warp, et de désactiver
 * l'interpolation manuelle (bInterpolationManuelle = false).
 *
 * Le Tick n'est actif QUE pendant un franchissement ; la détection automatique en course
 * passe par un timer à basse fréquence.
 */
UCLASS(ClassGroup=(Mansa), meta=(BlueprintSpawnableComponent))
class UMansaTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMansaTraversalComponent();

	// --- Réglages (hauteurs et durées viennent de DT_CharacterMovement via AppliquerReglages) ---

	/** Hauteur max d'enjambement (cm), copiée depuis DT_CharacterMovement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mansa|Franchissement")
	float HauteurEnjambementMax = 90.f;

	/** Hauteur max de mantle (cm), copiée depuis DT_CharacterMovement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mansa|Franchissement")
	float HauteurMantleMax = 180.f;

	/** Durées (s), copiées depuis DT_CharacterMovement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mansa|Franchissement")
	float DureeEnjambement = 0.45f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mansa|Franchissement")
	float DureeMantle = 0.7f;

	/** Interrupteur général. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement")
	bool bActif = true;

	/** Autorise un mantle en plein saut (attraper un rebord en l'air). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement")
	bool bAutoriserEnLair = true;

	/** Distance de détection devant la capsule (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="1", Units="cm"))
	float DistanceDetection = 70.f;

	/** Pas vertical entre deux traces de détection de face (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="5", Units="cm"))
	float PasTraceVertical = 20.f;

	/** Épaisseur max d'un obstacle pour être ENJAMBÉ (au-delà, on monte dessus). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="1", Units="cm"))
	float EpaisseurMaxEnjambement = 60.f;

	/** Distance de réception après la face arrière lors d'un enjambement (en plus du rayon). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0", Units="cm"))
	float DistanceReception = 30.f;

	/** Avancée sur le rebord après un mantle (en plus du rayon de la capsule). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0", Units="cm"))
	float AvanceeSurRebord = 15.f;

	/** Pente max du sommet (cosinus mini de la normale) pour s'y poser. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0", ClampMax="1"))
	float NormaleMinSommet = 0.7f;

	/** Vitesse horizontale rendue en fin d'enjambement (cm/s), pour garder l'élan. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement", meta=(ClampMin="0", Units="cm/s"))
	float VitesseSortieEnjambement = 300.f;

	/** Enjambement automatique en courant vers un muret (GDD §4.3 « enjambement auto »). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement|Auto")
	bool bEnjambementAuto = true;

	/** Vitesse au sol minimale pour l'enjambement auto (par défaut : au-dessus du jogging, donc en course). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement|Auto", meta=(ClampMin="0", Units="cm/s"))
	float VitesseMinEnjambementAuto = 500.f;

	/** Distance de déclenchement de l'enjambement auto devant la capsule (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement|Auto", meta=(ClampMin="1", Units="cm"))
	float DistanceEnjambementAuto = 40.f;

	/** Période de la détection automatique (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement|Auto", meta=(ClampMin="0.02", Units="s"))
	float PeriodeDetectionAuto = 0.1f;

	/** Si faux, le déplacement est laissé à une animation (Motion Warping) : seuls les événements sont émis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement")
	bool bInterpolationManuelle = true;

	/** Canal des traces de détection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Franchissement")
	TEnumAsByte<ECollisionChannel> CanalTrace = ECC_Visibility;

	/** Dessine les traces et le trajet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Debug")
	bool bDebug = false;

	UPROPERTY(BlueprintAssignable, Category="Mansa|Franchissement")
	FMansaOnFranchissementDebut OnFranchissementDebut;

	UPROPERTY(BlueprintAssignable, Category="Mansa|Franchissement")
	FMansaOnFranchissementFin OnFranchissementFin;

	/** Copie les hauteurs et durées depuis une ligne de DT_CharacterMovement. */
	void AppliquerReglages(const FMansaMovementRow& Reglages);

	/** Analyse l'obstacle devant le personnage, sans rien déclencher. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Franchissement")
	FMansaAnalyseObstacle AnalyserObstacle(float Distance) const;

	/** Tente un franchissement (appelé sur l'appui de Saut). Renvoie vrai s'il a commencé. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Franchissement")
	bool TenterFranchissement();

	UFUNCTION(BlueprintPure, Category="Mansa|Franchissement")
	bool EstEnFranchissement() const { return TypeEnCours != EMansaFranchissement::Aucun; }

	/** Termine immédiatement le franchissement en cours (si une anim le pilote, par ex.). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Franchissement")
	void TerminerFranchissement();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Démarre le franchissement décrit par l'analyse. */
	void Demarrer(const FMansaAnalyseObstacle& Analyse);

	/** Timer de l'enjambement automatique. */
	void VerifierEnjambementAuto();

	/** Teste qu'une capsule de la taille du personnage peut aller de A à B sans rien toucher. */
	bool TrajetLibre(const FVector& A, const FVector& B) const;

	/** Teste qu'une capsule tient à cet endroit. */
	bool PlaceLibre(const FVector& Centre) const;

	ACharacter* GetPersonnage() const;

	EMansaFranchissement TypeEnCours = EMansaFranchissement::Aucun;
	TArray<FVector> TrajetEnCours;
	TArray<float> LongueursCumulees;
	float DureeEnCours = 0.f;
	float TempsEcoule = 0.f;
	FVector DirectionSortie = FVector::ZeroVector;
	FTimerHandle TimerAuto;
};
