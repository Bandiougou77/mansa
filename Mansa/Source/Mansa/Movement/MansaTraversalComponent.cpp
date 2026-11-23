// MANSA — Enjambement et mantle (D2, GDD §4.3).

#include "MansaTraversalComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Mansa.h"

namespace
{
	/** Petite marge (cm) pour éviter de démarrer une capsule exactement au contact d'une surface. */
	constexpr float MargeContact = 2.f;
}

UMansaTraversalComponent::UMansaTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Tick uniquement pendant un franchissement.
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMansaTraversalComponent::AppliquerReglages(const FMansaMovementRow& Reglages)
{
	HauteurEnjambementMax = Reglages.HauteurEnjambementMax;
	HauteurMantleMax = Reglages.HauteurMantleMax;
	DureeEnjambement = Reglages.DureeEnjambement;
	DureeMantle = Reglages.DureeMantle;
}

void UMansaTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bEnjambementAuto && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerAuto, this, &UMansaTraversalComponent::VerifierEnjambementAuto, PeriodeDetectionAuto, true);
	}
}

void UMansaTraversalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerAuto);
	}
	Super::EndPlay(EndPlayReason);
}

ACharacter* UMansaTraversalComponent::GetPersonnage() const
{
	return Cast<ACharacter>(GetOwner());
}

bool UMansaTraversalComponent::PlaceLibre(const FVector& Centre) const
{
	const ACharacter* Perso = GetPersonnage();
	const UCapsuleComponent* Capsule = Perso->GetCapsuleComponent();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaPlaceLibre), false, Perso);
	const FCollisionShape Forme = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight());
	const bool bBloque = GetWorld()->OverlapBlockingTestByChannel(Centre, FQuat::Identity, Capsule->GetCollisionObjectType(), Forme, Params);
	if (bDebug)
	{
		DrawDebugCapsule(GetWorld(), Centre, Forme.GetCapsuleHalfHeight(), Forme.GetCapsuleRadius(), FQuat::Identity, bBloque ? FColor::Red : FColor::Green, false, 3.f);
	}
	return !bBloque;
}

bool UMansaTraversalComponent::TrajetLibre(const FVector& A, const FVector& B) const
{
	const ACharacter* Perso = GetPersonnage();
	const UCapsuleComponent* Capsule = Perso->GetCapsuleComponent();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaTrajetLibre), false, Perso);
	// Capsule très légèrement réduite : on tolère le contact rasant avec le sommet et le mur.
	const FCollisionShape Forme = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius() - 1.f, Capsule->GetScaledCapsuleHalfHeight() - 1.f);
	const bool bBloque = GetWorld()->SweepTestByChannel(A, B, FQuat::Identity, Capsule->GetCollisionObjectType(), Forme, Params);
	if (bDebug)
	{
		DrawDebugLine(GetWorld(), A, B, bBloque ? FColor::Red : FColor::Cyan, false, 3.f, 0, 2.f);
	}
	return !bBloque;
}

FMansaAnalyseObstacle UMansaTraversalComponent::AnalyserObstacle(float Distance) const
{
	FMansaAnalyseObstacle Resultat;
	const ACharacter* Perso = GetPersonnage();
	if (!Perso || !GetWorld())
	{
		return Resultat;
	}
	const UCapsuleComponent* Capsule = Perso->GetCapsuleComponent();
	const UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
	const float Rayon = Capsule->GetScaledCapsuleRadius();
	const float DemiHauteur = Capsule->GetScaledCapsuleHalfHeight();
	const FVector Centre = Perso->GetActorLocation();
	const float Pieds = Centre.Z - DemiHauteur;
	const FVector Avant = Perso->GetActorForwardVector().GetSafeNormal2D();
	// En dessous de MaxStepHeight, le CMC monte tout seul la marche : rien à faire ici.
	const float HauteurMin = CMC->MaxStepHeight;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaFranchissement), false, Perso);

	// 1. Face avant : traces horizontales de bas en haut, on garde la première touchée.
	FHitResult ImpactMur;
	bool bMurTrouve = false;
	for (float Z = Pieds + HauteurMin + 5.f; Z <= Pieds + HauteurMantleMax; Z += PasTraceVertical)
	{
		const FVector Debut(Centre.X, Centre.Y, Z);
		const FVector Fin = Debut + Avant * (Rayon + Distance);
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Debut, Fin, CanalTrace, Params))
		{
			// Face à peu près verticale uniquement (pas une rampe).
			if (FMath::Abs(Hit.ImpactNormal.Z) < 0.3f)
			{
				ImpactMur = Hit;
				bMurTrouve = true;
			}
			if (bDebug) { DrawDebugLine(GetWorld(), Debut, Hit.ImpactPoint, FColor::Orange, false, 3.f); }
			break;
		}
		if (bDebug) { DrawDebugLine(GetWorld(), Debut, Fin, FColor::Silver, false, 3.f); }
	}
	if (!bMurTrouve)
	{
		return Resultat;
	}
	const FVector Normale = ImpactMur.ImpactNormal.GetSafeNormal2D();
	const FVector DansObstacle = -Normale;

	// 2. Sommet : trace verticale juste derrière la face avant.
	//    Si la trace démarre DANS l'obstacle (mur trop haut), elle ne touche pas son sommet : rejet.
	const FVector PointSonde = ImpactMur.ImpactPoint + DansObstacle * 15.f;
	const FVector HautSonde(PointSonde.X, PointSonde.Y, Pieds + HauteurMantleMax + 5.f);
	const FVector BasSonde(PointSonde.X, PointSonde.Y, Pieds + HauteurMin);
	FHitResult ImpactSommet;
	if (!GetWorld()->LineTraceSingleByChannel(ImpactSommet, HautSonde, BasSonde, CanalTrace, Params)
		|| ImpactSommet.bStartPenetrating
		|| ImpactSommet.ImpactNormal.Z < NormaleMinSommet)
	{
		return Resultat;
	}
	if (bDebug) { DrawDebugSphere(GetWorld(), ImpactSommet.ImpactPoint, 8.f, 8, FColor::Yellow, false, 3.f); }

	const float Hauteur = ImpactSommet.ImpactPoint.Z - Pieds;
	if (Hauteur <= HauteurMin || Hauteur > HauteurMantleMax)
	{
		return Resultat;
	}

	Resultat.Hauteur = Hauteur;
	Resultat.PointMur = ImpactMur.ImpactPoint;
	Resultat.NormaleMur = Normale;
	Resultat.PointSommet = ImpactSommet.ImpactPoint;

	// Point haut commun : capsule juste au-dessus du sommet, collée à la face avant.
	const float ZHaut = ImpactSommet.ImpactPoint.Z + DemiHauteur + MargeContact;
	const FVector FaceAvant = ImpactMur.ImpactPoint;
	const FVector PointMonte(FaceAvant.X + Normale.X * (Rayon + MargeContact), FaceAvant.Y + Normale.Y * (Rayon + MargeContact), ZHaut);

	// 3. Obstacle bas : fin (enjambement) ou profond (montée) ?
	if (Hauteur <= HauteurEnjambementMax)
	{
		// Trace depuis l'arrière vers la face avant, juste sous le sommet : si elle touche, l'obstacle est fin.
		const float ZFace = ImpactSommet.ImpactPoint.Z - 5.f;
		const FVector DebutArriere = FVector(FaceAvant.X, FaceAvant.Y, ZFace) + DansObstacle * (EpaisseurMaxEnjambement + 5.f);
		const FVector FinArriere(FaceAvant.X, FaceAvant.Y, ZFace);
		FHitResult ImpactArriere;
		if (GetWorld()->LineTraceSingleByChannel(ImpactArriere, DebutArriere, FinArriere, CanalTrace, Params) && !ImpactArriere.bStartPenetrating)
		{
			const FVector FaceArriere = ImpactArriere.ImpactPoint;
			const FVector PointPasse(FaceArriere.X + DansObstacle.X * (Rayon + MargeContact), FaceArriere.Y + DansObstacle.Y * (Rayon + MargeContact), ZHaut);
			const FVector PointReception = PointPasse + DansObstacle * DistanceReception;
			if (TrajetLibre(Centre, PointMonte) && TrajetLibre(PointMonte, PointPasse) && TrajetLibre(PointPasse, PointReception))
			{
				Resultat.Type = EMansaFranchissement::Enjambement;
				Resultat.Trajet = { Centre, PointMonte, PointPasse, PointReception };
				return Resultat;
			}
		}
	}

	// 4. Montée / mantle : on finit debout sur le sommet.
	const FVector PointArrivee = FVector(FaceAvant.X, FaceAvant.Y, ZHaut) + DansObstacle * (Rayon + AvanceeSurRebord);
	if (!PlaceLibre(PointArrivee) || !TrajetLibre(Centre, PointMonte) || !TrajetLibre(PointMonte, PointArrivee))
	{
		return Resultat;
	}
	Resultat.Type = Hauteur <= HauteurEnjambementMax ? EMansaFranchissement::MonteeBasse : EMansaFranchissement::Mantle;
	Resultat.Trajet = { Centre, PointMonte, PointArrivee };
	return Resultat;
}

bool UMansaTraversalComponent::TenterFranchissement()
{
	ACharacter* Perso = GetPersonnage();
	if (!bActif || !Perso || EstEnFranchissement())
	{
		return false;
	}
	const UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
	const bool bAuSol = CMC->IsMovingOnGround();
	if (!bAuSol && !(bAutoriserEnLair && CMC->IsFalling()))
	{
		return false;
	}

	const FMansaAnalyseObstacle Analyse = AnalyserObstacle(DistanceDetection);
	if (Analyse.Type == EMansaFranchissement::Aucun)
	{
		return false;
	}
	Demarrer(Analyse);
	return true;
}

void UMansaTraversalComponent::VerifierEnjambementAuto()
{
	ACharacter* Perso = GetPersonnage();
	if (!bActif || !bEnjambementAuto || !Perso || EstEnFranchissement())
	{
		return;
	}
	const UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
	// Coût quasi nul hors course : on ne trace que si l'on va assez vite au sol.
	if (!CMC->IsMovingOnGround() || CMC->Velocity.Size2D() < VitesseMinEnjambementAuto)
	{
		return;
	}
	const FMansaAnalyseObstacle Analyse = AnalyserObstacle(DistanceEnjambementAuto);
	if (Analyse.Type == EMansaFranchissement::Enjambement)
	{
		Demarrer(Analyse);
	}
}

void UMansaTraversalComponent::Demarrer(const FMansaAnalyseObstacle& Analyse)
{
	ACharacter* Perso = GetPersonnage();
	UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();

	TypeEnCours = Analyse.Type;
	TrajetEnCours = Analyse.Trajet;
	DirectionSortie = -Analyse.NormaleMur;
	DureeEnCours = Analyse.Type == EMansaFranchissement::Enjambement ? DureeEnjambement : DureeMantle;
	TempsEcoule = 0.f;

	// Longueurs cumulées : la vitesse le long du trajet est constante (puis lissée globalement).
	LongueursCumulees.Reset();
	LongueursCumulees.Add(0.f);
	for (int32 i = 1; i < TrajetEnCours.Num(); ++i)
	{
		LongueursCumulees.Add(LongueursCumulees.Last() + FVector::Dist(TrajetEnCours[i - 1], TrajetEnCours[i]));
	}

	// Face à l'obstacle, sans vitesse ni gravité pendant le mouvement.
	Perso->SetActorRotation(FRotator(0.f, DirectionSortie.Rotation().Yaw, 0.f));
	CMC->StopMovementImmediately();
	CMC->SetMovementMode(MOVE_Flying);

	UE_LOG(LogMansa, Log, TEXT("Franchissement : %s, hauteur %.0f cm"), *UEnum::GetValueAsString(TypeEnCours), Analyse.Hauteur);
	if (bDebug)
	{
		for (int32 i = 1; i < TrajetEnCours.Num(); ++i)
		{
			DrawDebugLine(GetWorld(), TrajetEnCours[i - 1], TrajetEnCours[i], FColor::Green, false, 3.f, 0, 3.f);
		}
	}

	OnFranchissementDebut.Broadcast(TypeEnCours, Analyse);

	if (bInterpolationManuelle)
	{
		SetComponentTickEnabled(true);
	}
}

void UMansaTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACharacter* Perso = GetPersonnage();
	if (!Perso || TrajetEnCours.Num() < 2 || DureeEnCours <= 0.f)
	{
		TerminerFranchissement();
		return;
	}

	TempsEcoule += DeltaTime;
	const float Alpha = FMath::Clamp(TempsEcoule / DureeEnCours, 0.f, 1.f);
	// Départ et arrivée adoucis.
	const float Distance = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f) * LongueursCumulees.Last();

	int32 Segment = 1;
	while (Segment < LongueursCumulees.Num() - 1 && LongueursCumulees[Segment] < Distance)
	{
		++Segment;
	}
	const float LongueurSegment = LongueursCumulees[Segment] - LongueursCumulees[Segment - 1];
	const float AlphaSegment = LongueurSegment > KINDA_SMALL_NUMBER ? (Distance - LongueursCumulees[Segment - 1]) / LongueurSegment : 1.f;
	const FVector Position = FMath::Lerp(TrajetEnCours[Segment - 1], TrajetEnCours[Segment], FMath::Clamp(AlphaSegment, 0.f, 1.f));

	// Pas de balayage : le trajet a été validé par des tests de capsule à l'analyse.
	Perso->SetActorLocation(Position, false, nullptr, ETeleportType::None);

	if (Alpha >= 1.f)
	{
		TerminerFranchissement();
	}
}

void UMansaTraversalComponent::TerminerFranchissement()
{
	if (!EstEnFranchissement())
	{
		return;
	}
	const EMansaFranchissement TypeFini = TypeEnCours;
	TypeEnCours = EMansaFranchissement::Aucun;
	SetComponentTickEnabled(false);

	if (ACharacter* Perso = GetPersonnage())
	{
		UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
		// Chute libre : le CMC retrouve le sol tout seul (sommet ou autre côté du muret).
		CMC->SetMovementMode(MOVE_Falling);
		if (TypeFini == EMansaFranchissement::Enjambement)
		{
			CMC->Velocity = DirectionSortie * VitesseSortieEnjambement;
		}
		UE_LOG(LogMansa, Log, TEXT("Franchissement terminé : %s, position Z %.0f"), *UEnum::GetValueAsString(TypeFini), Perso->GetActorLocation().Z);
	}
	OnFranchissementFin.Broadcast(TypeFini);
}
