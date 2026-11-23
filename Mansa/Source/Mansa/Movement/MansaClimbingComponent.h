// MANSA — Escalade à prises discrètes (D4-D6, GDD §4.4).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "MansaDataTypes.h"
#include "MansaClimbingComponent.generated.h"

class ACharacter;
class UMansaPriseComponent;

/** État de l'escalade. */
UENUM(BlueprintType)
enum class EMansaEtatEscalade : uint8
{
	/** Pas en escalade (au sol, en l'air). */
	Aucun,
	/** En mouvement vers une prise (accroche depuis le sol ou saut entre prises). */
	Saut,
	/** Suspendu à une prise. */
	Suspendu,
	/** Se hisse sur le rebord d'une corniche. */
	Mantle
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMansaOnEtatEscalade, EMansaEtatEscalade, Ancien, EMansaEtatEscalade, Nouveau);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMansaOnPriseEscalade, UMansaPriseComponent*, Prise);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMansaOnSautEscalade, UMansaPriseComponent*, Depart, UMansaPriseComponent*, Cible);

/**
 * Escalade façon Uncharted, sans animation dédiée (les déplacements sont interpolés ; les
 * événements permettront de jouer des montages plus tard, avec Motion Warping sur PointMain).
 *
 *  - Accroche : Saut face à une prise à portée (depuis le sol), ou automatiquement en tombant
 *    contre une prise (mains à moins de RayonAccrocheEnChute).
 *  - Suspendu : mode de mouvement Custom (aucune physique), capsule plaquée devant la prise selon
 *    sa normale, orientée face au mur.
 *  - Saut entre prises : Saut + direction du stick -> prise la plus proche dans cette direction
 *    respectant les distances max de DT_PrisesEscalade (150 cm vertical, 200 cm latéral).
 *  - Corniche : déplacement latéral au stick, enchaînement avec une corniche voisine alignée.
 *  - Mantle : Saut (vers le haut) sur une corniche qui l'autorise.
 *  - Lâcher : IA_DropHold. Pendant FenetreRattrapage après une chute depuis une prise, Saut
 *    permet de rattraper une prise à portée (ex. toron de secours sous une BancoFragile).
 *
 * Performance : Tick seulement pendant un mouvement interpolé ; détection en chute par timer,
 * actif uniquement en mode Falling.
 */
UCLASS(ClassGroup=(Mansa), meta=(BlueprintSpawnableComponent))
class UMansaClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMansaClimbingComponent();

	// ------------------------------------------------------------------
	// Réglages (les règles par type de prise viennent de DT_PrisesEscalade)
	// ------------------------------------------------------------------

	/** Interrupteur général. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	bool bActif = true;

	/** Suspendu : centre de la capsule à cette distance SOUS le point de saisie (mains au-dessus de la tête). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Suspension", meta=(Units="cm"))
	float DecalageSuspensionVertical = 106.f;

	/** Suspendu : écart entre le point de saisie et l'avant de la capsule, le long de la normale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Suspension", meta=(ClampMin="0", Units="cm"))
	float EcartMainCapsule = 2.f;

	/** Si la place est prise (géométrie voisine), on recule le long de la normale par pas de... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Suspension", meta=(ClampMin="1", Units="cm"))
	float PasRecul = 10.f;

	/** ...jusqu'à cette distance au maximum. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Suspension", meta=(ClampMin="0", Units="cm"))
	float ReculMax = 100.f;

	/** Corniche : pas de glissement du point de saisie pour trouver une place libre (coin intérieur). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Suspension", meta=(ClampMin="1", Units="cm"))
	float PasGlissementCorniche = 5.f;

	/** Marge de contact (cm) pour ne pas démarrer une capsule pile contre une surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Suspension", meta=(ClampMin="0", Units="cm"))
	float MargeContact = 2.f;

	/** Saisie depuis le sol : hauteur minimale de la prise au-dessus des pieds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="0", Units="cm"))
	float HauteurSaisieMin = 60.f;

	/** Saisie depuis le sol : la prise doit être devant Sira (cosinus mini entre l'avant et la direction de la prise). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="-1", ClampMax="1"))
	float CosFaceSaisieSol = 0.3f;

	/** Durée de l'accroche depuis le sol ou en l'air. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="0.05", Units="s"))
	float DureeAccroche = 0.3f;

	/** En chute : distance mains -> prise en dessous de laquelle Sira s'accroche toute seule. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="0", Units="cm"))
	float RayonAccrocheEnChute = 60.f;

	/** Période de la détection en chute (timer actif seulement en Falling). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="0.01", Units="s"))
	float PeriodeDetectionChute = 0.05f;

	/** Après avoir lâché / quitté une prise, elle est ignorée par l'accroche auto pendant... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="0", Units="s"))
	float DelaiReaccroche = 0.5f;

	/** Après une chute depuis une prise : fenêtre pendant laquelle Saut rattrape une prise à portée. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="0", Units="s"))
	float FenetreRattrapage = 0.6f;

	/** Vitesse donnée en lâchant (le long de la normale, pour se décoller du mur). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Accroche", meta=(ClampMin="0", Units="cm/s"))
	float ImpulsionLacher = 120.f;

	/** Choix de la prise visée : cosinus mini entre la direction du stick et la direction de la prise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Saut", meta=(ClampMin="-1", ClampMax="1"))
	float CosDirectionMin = 0.5f;

	/** Poids de l'écart angulaire dans le score (score = distance × (1 + (1 - cos) × poids)). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Saut", meta=(ClampMin="0"))
	float PoidsAngle = 2.f;

	/** Normales de la prise de départ et de la cible : cosinus mini (évite de passer de l'autre côté d'un mur). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Saut", meta=(ClampMin="-1", ClampMax="1"))
	float CosNormalesMin = -0.3f;

	/** Stick en dessous de cette amplitude = pas de direction (Saut seul = vers le haut). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Saut", meta=(ClampMin="0", ClampMax="1"))
	float SeuilStick = 0.3f;

	/** Hauteur de l'arc pendant un saut entre prises. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Saut", meta=(ClampMin="0", Units="cm"))
	float HauteurArcSaut = 15.f;

	/** Corniche : distance max entre deux corniches alignées pour passer de l'une à l'autre. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Corniche", meta=(ClampMin="0", Units="cm"))
	float DistanceEnchainementCorniche = 30.f;

	/** Mantle : avancée derrière l'arête à l'arrivée (en plus du rayon de la capsule). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Mantle", meta=(ClampMin="0", Units="cm"))
	float AvanceeMantle = 60.f;

	/** Mantle bloqué (pinacle, merlon) : on cherche une place libre le long de la corniche jusqu'à cette distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Mantle", meta=(ClampMin="0", Units="cm"))
	float GlissementMaxMantle = 80.f;

	/** Durée du mantle (copiée depuis DT_CharacterMovement.DureeMantle par le personnage). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade|Mantle", meta=(ClampMin="0.05", Units="s"))
	float DureeMantle = 0.7f;

	/** Sous-mode Custom du Character Movement Component utilisé pendant l'escalade. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	uint8 ModeCustomEscalade = 1;

	/** Canal des tests de place libre et des traces (les prises l'ignorent). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Escalade")
	TEnumAsByte<ECollisionChannel> CanalCollision = ECC_Pawn;

	/** Dessine les tests et les trajets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Debug")
	bool bDebug = false;

	// ------------------------------------------------------------------
	// Événements (pour brancher animations, sons, caméra)
	// ------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnEtatEscalade OnEtatChange;

	/** Sira vient de s'accrocher (fin d'une accroche ou d'un saut, ou passage à une corniche voisine). */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnPriseEscalade OnAccroche;

	/** Début d'un saut ou d'une accroche (Depart = nullptr depuis le sol / l'air). */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnSautEscalade OnSautDebut;

	/** Sira a lâché la prise (volontairement ou parce qu'elle s'est effritée). */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnPriseEscalade OnLacher;

	/** La prise tenue s'est effritée sous les mains de Sira. */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnPriseEscalade OnPriseEffritee;

	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnPriseEscalade OnMantleDebut;

	UPROPERTY(BlueprintAssignable, Category="Mansa|Escalade")
	FMansaOnPriseEscalade OnMantleFin;

	// ------------------------------------------------------------------
	// API
	// ------------------------------------------------------------------

	/** Appui sur Saut, routé par le personnage. Vrai si l'escalade a consommé l'appui. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	bool GererSaut();

	/** Stick (X = droite, Y = avant/haut), routé par le personnage. Déplace sur une corniche. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void TraiterStick(FVector2D Stick, float DeltaTime);

	/** Tente de s'accrocher à une prise à portée (depuis le sol, ou en l'air au contact). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	bool TenterAccroche();

	/** En l'air, pendant la fenêtre de rattrapage : saute vers la prise à portée dans la direction (repère du mur quitté). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	bool TenterRattrapageEnLair(FVector2D Direction);

	/** Suspendu : saute vers la meilleure prise dans Direction (X = droite, Y = haut, repère du mur). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	bool SauterDansDirection(FVector2D Direction);

	/** Suspendu : saute vers Cible si elle respecte les distances max. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	bool SauterVersPrise(UMansaPriseComponent* Cible);

	/** Meilleure prise atteignable dans Direction depuis la prise tenue (sans sauter). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	UMansaPriseComponent* TrouverPriseDansDirection(FVector2D Direction, FVector& PointSaisie) const;

	/** Suspendu à une corniche : glisse le long de celle-ci (Lateral -1..1 = gauche..droite de Sira). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	bool DeplacerSurCorniche(float Lateral, float DeltaTime);

	/** Suspendu à une prise qui l'autorise : se hisse sur le rebord. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	bool TenterMantle();

	/** Lâche la prise (IA_DropHold). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void Lacher();

	/** Sort de l'escalade sans chute ni impulsion (retour au checkpoint). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Escalade")
	void Reinitialiser();

	/** Direction (repère du mur : X = droite, Y = haut) de PointCible vu depuis la prise tenue. */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	FVector2D DirectionVers(const FVector& PointCible) const;

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	EMansaEtatEscalade GetEtat() const { return Etat; }

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	bool EstEnEscalade() const { return Etat != EMansaEtatEscalade::Aucun; }

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	bool EstSuspendu() const { return Etat == EMansaEtatEscalade::Suspendu; }

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	UMansaPriseComponent* GetPriseCourante() const { return PriseCourante; }

	/** Point de saisie actuel (cible de Motion Warping pour les mains, plus tard). */
	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	FVector GetPointMain() const { return PointMain; }

	/** Copie la durée de mantle depuis DT_CharacterMovement. */
	void AppliquerReglages(const FMansaMovementRow& Reglages);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	ACharacter* GetPersonnage() const;
	TArray<UMansaPriseComponent*> GetPrises() const;

	/** Axe « droite » de Sira face à un mur de normale N. */
	static FVector DroiteMur(const FVector& N) { return FVector(N.Y, -N.X, 0.f); }

	/** Point des mains (en l'air) déduit de la position de la capsule. */
	FVector PointMainsDepuisCapsule() const;

	/** Position de la capsule suspendue à Prise ; InOutPoint peut glisser le long d'une corniche. */
	bool CalculerSuspension(const UMansaPriseComponent* Prise, FVector& InOutPoint, FVector& OutCentre, bool bAutoriserGlissement) const;
	bool PlaceLibre(const FVector& Centre) const;
	bool TrajetLibre(const FVector& A, const FVector& B) const;

	/** Meilleure prise dans Direction depuis un point de saisie et un repère de mur donnés. */
	UMansaPriseComponent* ChercherPrise(const FVector& Depuis, const FVector& NormaleDepart, const UMansaPriseComponent* Exclue,
		FVector2D Direction, FVector& OutPoint) const;

	/** En l'air : prise la plus proche des mains (à moins de RayonAccrocheEnChute), côté extérieur du mur. */
	UMansaPriseComponent* ChercherPriseAuContact(FVector& OutPoint) const;

	/** Vrai si Cible est accessible par un saut depuis Depuis (distances de la DT). */
	bool EstAccessible(const FVector& Depuis, const UMansaPriseComponent* Cible, FVector& OutPoint) const;

	/** Lance l'interpolation vers la prise. */
	bool DemarrerAccroche(UMansaPriseComponent* Cible, FVector Point, float Duree, float Arc);

	/** Fin d'un mouvement interpolé. */
	void TerminerMouvement();

	/** Passe à l'état Suspendu sur Prise (sans mouvement). */
	void DevenirSuspendu(UMansaPriseComponent* Prise, const FVector& Point);

	/** Quitte la prise courante (désabonnement, prise ignorée quelques instants). */
	void QuitterPriseCourante();

	void SetEtat(EMansaEtatEscalade Nouveau);
	void PasserEnChute(const FVector& Vitesse);
	bool EstIgnoree(const UMansaPriseComponent* Prise) const;
	FVector2D StickVersDirectionMur(FVector2D Stick) const;

	UFUNCTION()
	void OnModeMouvementChange(ACharacter* Personnage, EMovementMode ModePrecedent, uint8 ModeCustomPrecedent);

	UFUNCTION()
	void OnPriseEffriteeInterne(UMansaPriseComponent* Prise);

	void VerifierAccrocheEnChute();

	EMansaEtatEscalade Etat = EMansaEtatEscalade::Aucun;

	UPROPERTY(Transient)
	TObjectPtr<UMansaPriseComponent> PriseCourante;

	UPROPERTY(Transient)
	TObjectPtr<UMansaPriseComponent> PriseCible;

	/** Dernière prise quittée (ignorée DelaiReaccroche secondes par l'accroche auto). */
	TWeakObjectPtr<UMansaPriseComponent> PriseIgnoree;
	float TempsPriseIgnoree = -1000.f;

	/** Lâcher volontaire (IA_DropHold) : pas d'accroche auto avant de toucher le sol (Saut reste possible). */
	bool bLacherVolontaire = false;

	/** Chute depuis une prise : début de la fenêtre de rattrapage et repère du mur quitté. */
	float TempsDebutChute = -1000.f;
	FVector NormaleChute = FVector::ForwardVector;

	FVector PointMain = FVector::ZeroVector;
	FVector PointCible = FVector::ZeroVector;
	FVector2D DernierStick = FVector2D::ZeroVector;

	// Mouvement interpolé
	TArray<FVector> Trajet;
	TArray<float> LongueursCumulees;
	float DureeMouvement = 0.f;
	float TempsMouvement = 0.f;
	float ArcMouvement = 0.f;

	FTimerHandle TimerChute;
};
