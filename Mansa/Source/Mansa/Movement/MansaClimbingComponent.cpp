// MANSA — Escalade à prises discrètes (D4-D6, GDD §4.4).

#include "MansaClimbingComponent.h"
#include "MansaPriseComponent.h"
#include "MansaPrisesSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "Mansa.h"

namespace
{
	FString NomPrise(const UMansaPriseComponent* Prise)
	{
		return Prise ? Prise->GetNomDebug() : FString(TEXT("<aucune>"));
	}

	FString TexteVecteur(const FVector& V)
	{
		return FString::Printf(TEXT("(%.0f, %.0f, %.0f)"), V.X, V.Y, V.Z);
	}
}

UMansaClimbingComponent::UMansaClimbingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Tick uniquement pendant un mouvement interpolé (accroche, saut, mantle).
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMansaClimbingComponent::AppliquerReglages(const FMansaMovementRow& Reglages)
{
	DureeMantle = Reglages.DureeMantle;
}

void UMansaClimbingComponent::BeginPlay()
{
	Super::BeginPlay();
	if (ACharacter* Perso = GetPersonnage())
	{
		Perso->MovementModeChangedDelegate.AddUniqueDynamic(this, &UMansaClimbingComponent::OnModeMouvementChange);
	}
}

void UMansaClimbingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ACharacter* Perso = GetPersonnage())
	{
		Perso->MovementModeChangedDelegate.RemoveDynamic(this, &UMansaClimbingComponent::OnModeMouvementChange);
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerChute);
	}
	Super::EndPlay(EndPlayReason);
}

ACharacter* UMansaClimbingComponent::GetPersonnage() const
{
	return Cast<ACharacter>(GetOwner());
}

TArray<UMansaPriseComponent*> UMansaClimbingComponent::GetPrises() const
{
	const UMansaPrisesSubsystem* Sub = UMansaPrisesSubsystem::Get(this);
	return Sub ? Sub->GetPrises() : TArray<UMansaPriseComponent*>();
}

void UMansaClimbingComponent::SetEtat(EMansaEtatEscalade Nouveau)
{
	if (Etat == Nouveau)
	{
		return;
	}
	const EMansaEtatEscalade Ancien = Etat;
	Etat = Nouveau;
	OnEtatChange.Broadcast(Ancien, Nouveau);
}

bool UMansaClimbingComponent::EstIgnoree(const UMansaPriseComponent* Prise) const
{
	return Prise && PriseIgnoree.Get() == Prise && GetWorld() && GetWorld()->GetTimeSeconds() - TempsPriseIgnoree < DelaiReaccroche;
}

// ---------------------------------------------------------------------------
// Géométrie
// ---------------------------------------------------------------------------

FVector UMansaClimbingComponent::PointMainsDepuisCapsule() const
{
	const ACharacter* Perso = GetPersonnage();
	const float Rayon = Perso->GetCapsuleComponent()->GetScaledCapsuleRadius();
	return Perso->GetActorLocation() + FVector::UpVector * DecalageSuspensionVertical
		+ Perso->GetActorForwardVector().GetSafeNormal2D() * (Rayon + EcartMainCapsule);
}

bool UMansaClimbingComponent::PlaceLibre(const FVector& Centre) const
{
	const ACharacter* Perso = GetPersonnage();
	const UCapsuleComponent* Capsule = Perso->GetCapsuleComponent();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaEscaladePlace), false, Perso);
	const FCollisionShape Forme = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight());
	const bool bBloque = GetWorld()->OverlapBlockingTestByChannel(Centre, FQuat::Identity, CanalCollision, Forme, Params);
	if (bDebug)
	{
		DrawDebugCapsule(GetWorld(), Centre, Forme.GetCapsuleHalfHeight(), Forme.GetCapsuleRadius(), FQuat::Identity, bBloque ? FColor::Red : FColor::Green, false, 2.f);
	}
	return !bBloque;
}

bool UMansaClimbingComponent::TrajetLibre(const FVector& A, const FVector& B) const
{
	const ACharacter* Perso = GetPersonnage();
	const UCapsuleComponent* Capsule = Perso->GetCapsuleComponent();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaEscaladeTrajet), false, Perso);
	// Capsule à peine réduite : on tolère le contact rasant avec l'arête et le mur.
	const FCollisionShape Forme = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius() - 1.f, Capsule->GetScaledCapsuleHalfHeight() - 1.f);
	const bool bBloque = GetWorld()->SweepTestByChannel(A, B, FQuat::Identity, CanalCollision, Forme, Params);
	if (bDebug)
	{
		DrawDebugLine(GetWorld(), A, B, bBloque ? FColor::Red : FColor::Cyan, false, 2.f, 0, 2.f);
	}
	return !bBloque;
}

bool UMansaClimbingComponent::CalculerSuspension(const UMansaPriseComponent* Prise, FVector& InOutPoint, FVector& OutCentre, bool bAutoriserGlissement) const
{
	const ACharacter* Perso = GetPersonnage();
	if (!Prise || !Perso)
	{
		return false;
	}
	const float Rayon = Perso->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float DemiHauteur = Perso->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector N = Prise->GetNormale();

	// Points de saisie candidats : le point demandé, puis (corniche) des points qui glissent
	// alternativement de chaque côté. C'est ce qui règle le coin intérieur (D2-08) : la capsule
	// ne tient pas contre le mur voisin, on décale les mains le long de l'arête.
	TArray<FVector> Candidats = { InOutPoint };
	if (bAutoriserGlissement && Prise->EstSegment())
	{
		FVector A, B;
		Prise->GetSegment(A, B);
		const FVector Axe = (B - A).GetSafeNormal();
		const float Longueur = FVector::Dist(A, B);
		for (float D = PasGlissementCorniche; D <= Longueur; D += PasGlissementCorniche)
		{
			for (const float Signe : { 1.f, -1.f })
			{
				const FVector P = Prise->GetPointLePlusProche(InOutPoint + Axe * D * Signe);
				if (!P.Equals(Candidats.Last(), 0.5f) && !P.Equals(InOutPoint, 0.5f))
				{
					Candidats.Add(P);
				}
			}
		}
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaEscaladeSol), false, Perso);
	for (const FVector& Point : Candidats)
	{
		FVector Base = Point + N * (Rayon + EcartMainCapsule);
		Base.Z = Point.Z - DecalageSuspensionVertical;
		for (float Recul = 0.f; Recul <= ReculMax; Recul += PasRecul)
		{
			FVector Centre = Base + N * Recul;
			// Prise basse (saisie depuis le sol) : les pieds restent au sol, bras levés.
			FHitResult Sol;
			const FVector Haut(Centre.X, Centre.Y, Centre.Z + DemiHauteur);
			const FVector Bas(Centre.X, Centre.Y, Centre.Z - DemiHauteur - MargeContact);
			if (GetWorld()->LineTraceSingleByChannel(Sol, Haut, Bas, CanalCollision, Params) && !Sol.bStartPenetrating)
			{
				Centre.Z = FMath::Max(Centre.Z, Sol.ImpactPoint.Z + DemiHauteur + MargeContact);
			}
			if (PlaceLibre(Centre))
			{
				InOutPoint = Point;
				OutCentre = Centre;
				return true;
			}
		}
	}
	return false;
}

bool UMansaClimbingComponent::EstAccessible(const FVector& Depuis, const UMansaPriseComponent* Cible, FVector& OutPoint) const
{
	if (!Cible || !Cible->EstDisponible())
	{
		return false;
	}
	const FMansaPriseRow& R = Cible->GetReglages();
	OutPoint = Cible->GetPointLePlusProche(Depuis);
	const FVector Delta = OutPoint - Depuis;
	return Delta.Size2D() <= R.DistanceMaxLaterale && FMath::Abs(Delta.Z) <= R.DistanceMaxVerticale;
}

UMansaPriseComponent* UMansaClimbingComponent::ChercherPrise(const FVector& Depuis, const FVector& NormaleDepart, const UMansaPriseComponent* Exclue,
	FVector2D Direction, FVector& OutPoint) const
{
	Direction = Direction.IsNearlyZero() ? FVector2D(0.f, 1.f) : Direction.GetSafeNormal();
	const FVector Droite = DroiteMur(NormaleDepart);
	UMansaPriseComponent* Meilleure = nullptr;
	float MeilleurScore = TNumericLimits<float>::Max();
	for (UMansaPriseComponent* P : GetPrises())
	{
		if (P == Exclue || !P->EstDisponible() || FVector::DotProduct(P->GetNormale(), NormaleDepart) < CosNormalesMin)
		{
			continue;
		}
		FVector Q;
		if (!EstAccessible(Depuis, P, Q))
		{
			continue;
		}
		const FVector Delta = Q - Depuis;
		if (Delta.Size() < 1.f)
		{
			continue;
		}
		// Direction de la prise dans le repère du mur (X = droite de Sira, Y = haut).
		const FVector2D DirPrise = FVector2D(FVector::DotProduct(Delta, Droite), Delta.Z).GetSafeNormal();
		const float Cos = FVector2D::DotProduct(DirPrise, Direction);
		if (Cos < CosDirectionMin)
		{
			continue;
		}
		const float Score = Delta.Size() * (1.f + (1.f - Cos) * PoidsAngle);
		if (Score < MeilleurScore)
		{
			MeilleurScore = Score;
			Meilleure = P;
			OutPoint = Q;
		}
	}
	return Meilleure;
}

UMansaPriseComponent* UMansaClimbingComponent::ChercherPriseAuContact(FVector& OutPoint) const
{
	const ACharacter* Perso = GetPersonnage();
	const FVector Mains = PointMainsDepuisCapsule();
	const FVector Centre = Perso->GetActorLocation();
	UMansaPriseComponent* Meilleure = nullptr;
	float MeilleureDistance = RayonAccrocheEnChute;
	for (UMansaPriseComponent* P : GetPrises())
	{
		if (!P->EstDisponible() || EstIgnoree(P))
		{
			continue;
		}
		const FVector Q = P->GetPointLePlusProche(Mains);
		const float D = FVector::Dist(Q, Mains);
		// Sira doit être du côté extérieur du mur.
		if (D <= MeilleureDistance && FVector::DotProduct(P->GetNormale(), (Centre - Q).GetSafeNormal2D()) > 0.f)
		{
			MeilleureDistance = D;
			Meilleure = P;
			OutPoint = Q;
		}
	}
	return Meilleure;
}

FVector2D UMansaClimbingComponent::DirectionVers(const FVector& Cible) const
{
	const FVector N = PriseCourante ? PriseCourante->GetNormale() : NormaleChute;
	const FVector Delta = Cible - PointMain;
	return FVector2D(FVector::DotProduct(Delta, DroiteMur(N)), Delta.Z).GetSafeNormal();
}

FVector2D UMansaClimbingComponent::StickVersDirectionMur(FVector2D Stick) const
{
	if (Stick.Size() < SeuilStick)
	{
		return FVector2D::ZeroVector;
	}
	// Le stick est relatif à la caméra : si la caméra regarde le mur « à l'envers », on inverse la gauche/droite.
	const FVector N = PriseCourante ? PriseCourante->GetNormale() : NormaleChute;
	float Signe = 1.f;
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const AController* C = Pawn->GetController())
		{
			const FVector DroiteCamera = FRotationMatrix(FRotator(0.f, C->GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y);
			Signe = FVector::DotProduct(DroiteCamera, DroiteMur(N)) >= 0.f ? 1.f : -1.f;
		}
	}
	return FVector2D(Stick.X * Signe, Stick.Y);
}

// ---------------------------------------------------------------------------
// Entrées
// ---------------------------------------------------------------------------

bool UMansaClimbingComponent::GererSaut()
{
	ACharacter* Perso = GetPersonnage();
	if (!bActif || !Perso)
	{
		return false;
	}
	switch (Etat)
	{
	case EMansaEtatEscalade::Saut:
	case EMansaEtatEscalade::Mantle:
		return true;

	case EMansaEtatEscalade::Suspendu:
	{
		FVector2D Dir = StickVersDirectionMur(DernierStick);
		if (Dir.IsNearlyZero())
		{
			Dir = FVector2D(0.f, 1.f);
		}
		// Vers le haut sur une corniche qui l'autorise : on se hisse.
		if (Dir.GetSafeNormal().Y >= 0.7f && PriseCourante && PriseCourante->PeutMantle() && TenterMantle())
		{
			return true;
		}
		if (!SauterDansDirection(Dir))
		{
			UE_LOG(LogMansa, Log, TEXT("Escalade : aucune prise à portée dans la direction (%.2f, %.2f) depuis %s"), Dir.X, Dir.Y, *NomPrise(PriseCourante));
		}
		return true;
	}

	default:
		if (Perso->GetCharacterMovement()->IsFalling())
		{
			FVector2D Dir = StickVersDirectionMur(DernierStick);
			if (TenterRattrapageEnLair(Dir.IsNearlyZero() ? FVector2D(0.f, 1.f) : Dir))
			{
				return true;
			}
		}
		return TenterAccroche();
	}
}

void UMansaClimbingComponent::TraiterStick(FVector2D Stick, float DeltaTime)
{
	DernierStick = Stick;
	if (Etat != EMansaEtatEscalade::Suspendu || !PriseCourante || !PriseCourante->GetReglages().bDeplacementLateral)
	{
		return;
	}
	const FVector2D Dir = StickVersDirectionMur(Stick);
	if (FMath::Abs(Dir.X) >= SeuilStick)
	{
		DeplacerSurCorniche(Dir.X, DeltaTime);
	}
}

// ---------------------------------------------------------------------------
// Accroche, saut, corniche, mantle, lâcher
// ---------------------------------------------------------------------------

bool UMansaClimbingComponent::TenterAccroche()
{
	ACharacter* Perso = GetPersonnage();
	if (!bActif || !Perso || Etat != EMansaEtatEscalade::Aucun)
	{
		return false;
	}
	const UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
	if (CMC->IsFalling())
	{
		FVector Q;
		if (UMansaPriseComponent* P = ChercherPriseAuContact(Q))
		{
			return DemarrerAccroche(P, Q, DureeAccroche, 0.f);
		}
		return false;
	}
	if (!CMC->IsMovingOnGround())
	{
		return false;
	}
	FVector MeilleurPoint;
	UMansaPriseComponent* Meilleure = TrouverPriseDepuisSol(MeilleurPoint);
	return Meilleure && DemarrerAccroche(Meilleure, MeilleurPoint, DureeAccroche, 0.f);
}

UMansaPriseComponent* UMansaClimbingComponent::TrouverPriseDepuisSol(FVector& OutPoint) const
{
	const ACharacter* Perso = GetPersonnage();
	if (!bActif || !Perso || Etat != EMansaEtatEscalade::Aucun || !Perso->GetCharacterMovement()->IsMovingOnGround())
	{
		return nullptr;
	}
	// Saisie depuis le sol : prise devant Sira, à hauteur de saut, côté extérieur du mur.
	const FVector Centre = Perso->GetActorLocation();
	const float Pieds = Centre.Z - Perso->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Avant = Perso->GetActorForwardVector().GetSafeNormal2D();
	UMansaPriseComponent* Meilleure = nullptr;
	FVector MeilleurPoint = FVector::ZeroVector;
	float MeilleureDistance = TNumericLimits<float>::Max();
	for (UMansaPriseComponent* P : GetPrises())
	{
		if (!P->EstDisponible())
		{
			continue;
		}
		const FMansaPriseRow& R = P->GetReglages();
		const FVector Q = P->GetPointLePlusProche(Centre);
		const float Hauteur = Q.Z - Pieds;
		if (Hauteur < HauteurSaisieMin || Hauteur > R.HauteurSaisieDepuisSol)
		{
			continue;
		}
		const FVector VersPrise = FVector(Q.X - Centre.X, Q.Y - Centre.Y, 0.f);
		const float Distance = VersPrise.Size();
		if (Distance > R.DistanceSaisieDepuisSol || FVector::DotProduct(P->GetNormale(), -VersPrise) <= 0.f)
		{
			continue;
		}
		if (Distance > 1.f && FVector::DotProduct(Avant, VersPrise / Distance) < CosFaceSaisieSol)
		{
			continue;
		}
		if (Distance < MeilleureDistance)
		{
			MeilleureDistance = Distance;
			Meilleure = P;
			MeilleurPoint = Q;
		}
	}
	OutPoint = MeilleurPoint;
	return Meilleure;
}

bool UMansaClimbingComponent::TenterRattrapageEnLair(FVector2D Direction)
{
	ACharacter* Perso = GetPersonnage();
	if (!bActif || !Perso || Etat != EMansaEtatEscalade::Aucun || !Perso->GetCharacterMovement()->IsFalling())
	{
		return false;
	}
	if (GetWorld()->GetTimeSeconds() - TempsDebutChute > FenetreRattrapage)
	{
		return false;
	}
	FVector Q;
	UMansaPriseComponent* P = ChercherPrise(PointMainsDepuisCapsule(), NormaleChute, nullptr, Direction, Q);
	if (!P)
	{
		return false;
	}
	UE_LOG(LogMansa, Log, TEXT("Escalade : rattrapage en l'air vers %s"), *NomPrise(P));
	return DemarrerAccroche(P, Q, P->GetReglages().DureeSaut, HauteurArcSaut);
}

UMansaPriseComponent* UMansaClimbingComponent::TrouverPriseDansDirection(FVector2D Direction, FVector& PointSaisie) const
{
	if (Etat != EMansaEtatEscalade::Suspendu || !PriseCourante)
	{
		return nullptr;
	}
	return ChercherPrise(PointMain, PriseCourante->GetNormale(), PriseCourante, Direction, PointSaisie);
}

bool UMansaClimbingComponent::SauterDansDirection(FVector2D Direction)
{
	FVector Q;
	UMansaPriseComponent* Cible = TrouverPriseDansDirection(Direction, Q);
	return Cible && DemarrerAccroche(Cible, Q, Cible->GetReglages().DureeSaut, HauteurArcSaut);
}

bool UMansaClimbingComponent::SauterVersPrise(UMansaPriseComponent* Cible)
{
	if (Etat != EMansaEtatEscalade::Suspendu || !Cible || Cible == PriseCourante)
	{
		return false;
	}
	FVector Q;
	if (!EstAccessible(PointMain, Cible, Q))
	{
		const FVector Delta = Cible->GetPointLePlusProche(PointMain) - PointMain;
		UE_LOG(LogMansa, Log, TEXT("Escalade : %s hors de portée depuis %s (latéral %.0f / vertical %.0f)"),
			*NomPrise(Cible), *NomPrise(PriseCourante), Delta.Size2D(), Delta.Z);
		return false;
	}
	return DemarrerAccroche(Cible, Q, Cible->GetReglages().DureeSaut, HauteurArcSaut);
}

bool UMansaClimbingComponent::DemarrerAccroche(UMansaPriseComponent* Cible, FVector Point, float Duree, float Arc)
{
	ACharacter* Perso = GetPersonnage();
	if (!Perso || !Cible)
	{
		return false;
	}
	FVector Centre;
	if (!CalculerSuspension(Cible, Point, Centre, true))
	{
		UE_LOG(LogMansa, Log, TEXT("Escalade : pas de place pour se suspendre à %s"), *NomPrise(Cible));
		return false;
	}

	UMansaPriseComponent* Depart = PriseCourante;
	if (Depart)
	{
		QuitterPriseCourante();
	}
	PriseCible = Cible;
	PointCible = Point;

	Trajet = { Perso->GetActorLocation(), Centre };
	LongueursCumulees = { 0.f, static_cast<float>(FVector::Dist(Trajet[0], Trajet[1])) };
	DureeMouvement = Duree;
	TempsMouvement = 0.f;
	ArcMouvement = Arc;

	UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
	CMC->StopMovementImmediately();
	CMC->SetMovementMode(MOVE_Custom, ModeCustomEscalade);
	Perso->SetActorRotation(FRotator(0.f, (-Cible->GetNormale()).Rotation().Yaw, 0.f));

	SetEtat(EMansaEtatEscalade::Saut);
	UE_LOG(LogMansa, Log, TEXT("Escalade : %s -> %s (main %s)"), *NomPrise(Depart), *NomPrise(Cible), *TexteVecteur(Point));
	OnSautDebut.Broadcast(Depart, Cible);
	SetComponentTickEnabled(true);
	return true;
}

void UMansaClimbingComponent::DevenirSuspendu(UMansaPriseComponent* Prise, const FVector& Point)
{
	PriseCourante = Prise;
	PointMain = Point;
	Prise->OnEffritement.AddUniqueDynamic(this, &UMansaClimbingComponent::OnPriseEffriteeInterne);
	SetEtat(EMansaEtatEscalade::Suspendu);
	UE_LOG(LogMansa, Log, TEXT("Escalade : accrochée à %s"), *NomPrise(Prise));
	Prise->NotifierAccroche();
	OnAccroche.Broadcast(Prise);
}

void UMansaClimbingComponent::QuitterPriseCourante()
{
	if (!PriseCourante)
	{
		return;
	}
	PriseCourante->OnEffritement.RemoveDynamic(this, &UMansaClimbingComponent::OnPriseEffriteeInterne);
	PriseIgnoree = PriseCourante;
	TempsPriseIgnoree = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	NormaleChute = PriseCourante->GetNormale();
	PriseCourante = nullptr;
}

bool UMansaClimbingComponent::DeplacerSurCorniche(float Lateral, float DeltaTime)
{
	ACharacter* Perso = GetPersonnage();
	if (!Perso || Etat != EMansaEtatEscalade::Suspendu || !PriseCourante || !PriseCourante->EstSegment()
		|| !PriseCourante->GetReglages().bDeplacementLateral)
	{
		return false;
	}
	const FVector N = PriseCourante->GetNormale();
	const FVector Voulu = PointMain + DroiteMur(N) * FMath::Clamp(Lateral, -1.f, 1.f) * PriseCourante->GetReglages().VitesseCorniche * DeltaTime;
	FVector Nouveau = PriseCourante->GetPointLePlusProche(Voulu);

	if (!Nouveau.Equals(Voulu, 0.5f))
	{
		// Bout de la corniche : on enchaîne sur une corniche voisine alignée (ex. CorA -> CorB).
		UMansaPriseComponent* Voisine = nullptr;
		FVector PointVoisin = FVector::ZeroVector;
		float MeilleureDistance = DistanceEnchainementCorniche;
		for (UMansaPriseComponent* P : GetPrises())
		{
			if (P == PriseCourante || !P->EstDisponible() || !P->EstSegment() || FVector::DotProduct(P->GetNormale(), N) < 0.9f)
			{
				continue;
			}
			const FVector Q = P->GetPointLePlusProche(Voulu);
			const float D = FVector::Dist(Q, Voulu);
			if (D <= MeilleureDistance)
			{
				MeilleureDistance = D;
				Voisine = P;
				PointVoisin = Q;
			}
		}
		FVector Centre;
		if (Voisine && CalculerSuspension(Voisine, PointVoisin, Centre, false))
		{
			UE_LOG(LogMansa, Log, TEXT("Escalade : passage de %s à %s le long de l'arête"), *NomPrise(PriseCourante), *NomPrise(Voisine));
			QuitterPriseCourante();
			Perso->SetActorLocation(Centre);
			DevenirSuspendu(Voisine, PointVoisin);
			return true;
		}
		if (Nouveau.Equals(PointMain, 0.01f))
		{
			return false;
		}
	}

	FVector Centre;
	if (!CalculerSuspension(PriseCourante, Nouveau, Centre, false))
	{
		return false;
	}
	PointMain = Nouveau;
	Perso->SetActorLocation(Centre);
	return true;
}

bool UMansaClimbingComponent::TenterMantle()
{
	ACharacter* Perso = GetPersonnage();
	if (!Perso || Etat != EMansaEtatEscalade::Suspendu || !PriseCourante)
	{
		return false;
	}
	if (!PriseCourante->PeutMantle())
	{
		UE_LOG(LogMansa, Log, TEXT("Escalade : mantle interdit sur %s"), *NomPrise(PriseCourante));
		return false;
	}
	const float Rayon = Perso->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float DemiHauteur = Perso->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector N = PriseCourante->GetNormale();
	const FVector Centre = Perso->GetActorLocation();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaEscaladeMantle), false, Perso);

	// Points de franchissement candidats : la main, puis (corniche) de part et d'autre le long de
	// l'arête, pour contourner un pinacle ou un merlon posé juste au-dessus des mains.
	TArray<FVector> Candidats = { PointMain };
	if (PriseCourante->EstSegment())
	{
		FVector A, B;
		PriseCourante->GetSegment(A, B);
		const FVector Axe = (B - A).GetSafeNormal();
		for (float D = PasGlissementCorniche; D <= GlissementMaxMantle; D += PasGlissementCorniche)
		{
			for (const float Signe : { 1.f, -1.f })
			{
				const FVector P = PriseCourante->GetPointLePlusProche(PointMain + Axe * D * Signe);
				if (!P.Equals(PointMain, 0.5f))
				{
					Candidats.Add(P);
				}
			}
		}
	}

	bool bPasDeSol = true;
	bool bTrouve = false;
	FVector H = PointMain;
	FVector P1 = Centre;
	FVector P2 = Centre;
	FVector Arrivee = Centre;
	for (const FVector& Candidat : Candidats)
	{
		H = Candidat;
		// 1. Sommet de l'arête, juste derrière le point de saisie.
		float Sommet = H.Z;
		FHitResult Hit;
		const FVector Sonde = H - N * 10.f;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Sonde + FVector(0.f, 0.f, 100.f), Sonde - FVector(0.f, 0.f, 60.f), CanalCollision, Params) && !Hit.bStartPenetrating)
		{
			Sommet = Hit.ImpactPoint.Z;
		}
		float ZHaut = Sommet + DemiHauteur + MargeContact;

		// 2. Sol d'arrivée derrière l'arête.
		const FVector Dessus = H - N * (Rayon + AvanceeMantle);
		if (!GetWorld()->LineTraceSingleByChannel(Hit, FVector(Dessus.X, Dessus.Y, ZHaut + 10.f), FVector(Dessus.X, Dessus.Y, Sommet - 200.f), CanalCollision, Params)
			|| Hit.bStartPenetrating)
		{
			continue;
		}
		bPasDeSol = false;
		Arrivee = FVector(Dessus.X, Dessus.Y, Hit.ImpactPoint.Z + DemiHauteur + MargeContact);
		ZHaut = FMath::Max(ZHaut, Arrivee.Z);

		// 3. Trajet : monter le long du mur (en glissant vers le candidat), passer au-dessus de l'arête, se poser.
		const FVector Decalage = (H - PointMain) * FVector(1.f, 1.f, 0.f);
		P1 = FVector(Centre.X + Decalage.X, Centre.Y + Decalage.Y, ZHaut);
		P2 = FVector(Arrivee.X, Arrivee.Y, ZHaut);
		if (PlaceLibre(Arrivee) && TrajetLibre(Centre, P1) && TrajetLibre(P1, P2) && TrajetLibre(P2, Arrivee))
		{
			bTrouve = true;
			break;
		}
	}
	if (!bTrouve)
	{
		UE_LOG(LogMansa, Log, TEXT("Escalade : mantle impossible depuis %s (%s)"), *NomPrise(PriseCourante),
			bPasDeSol ? TEXT("pas de sol derrière l'arête") : TEXT("trajet bloqué"));
		return false;
	}
	PointMain = H;

	Trajet = { Centre, P1, P2, Arrivee };
	LongueursCumulees.Reset();
	LongueursCumulees.Add(0.f);
	for (int32 i = 1; i < Trajet.Num(); ++i)
	{
		LongueursCumulees.Add(LongueursCumulees.Last() + FVector::Dist(Trajet[i - 1], Trajet[i]));
	}
	DureeMouvement = DureeMantle;
	TempsMouvement = 0.f;
	ArcMouvement = 0.f;

	SetEtat(EMansaEtatEscalade::Mantle);
	UE_LOG(LogMansa, Log, TEXT("Escalade : mantle depuis %s -> arrivée %s"), *NomPrise(PriseCourante), *TexteVecteur(Arrivee));
	OnMantleDebut.Broadcast(PriseCourante);
	SetComponentTickEnabled(true);
	return true;
}

void UMansaClimbingComponent::PasserEnChute(const FVector& Vitesse)
{
	if (ACharacter* Perso = GetPersonnage())
	{
		UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
		CMC->SetMovementMode(MOVE_Falling);
		CMC->Velocity = Vitesse;
	}
}

void UMansaClimbingComponent::Lacher()
{
	if (Etat != EMansaEtatEscalade::Suspendu || !PriseCourante)
	{
		return;
	}
	UMansaPriseComponent* Prise = PriseCourante;
	const FVector N = Prise->GetNormale();
	QuitterPriseCourante();
	bLacherVolontaire = true;
	SetEtat(EMansaEtatEscalade::Aucun);
	TempsDebutChute = GetWorld()->GetTimeSeconds();
	PasserEnChute(N * ImpulsionLacher);
	UE_LOG(LogMansa, Log, TEXT("Escalade : Sira lâche %s"), *NomPrise(Prise));
	OnLacher.Broadcast(Prise);
}

void UMansaClimbingComponent::OnPriseEffriteeInterne(UMansaPriseComponent* Prise)
{
	if (Prise != PriseCourante || Etat != EMansaEtatEscalade::Suspendu)
	{
		return;
	}
	UE_LOG(LogMansa, Log, TEXT("Escalade : %s cède sous les mains de Sira, elle tombe !"), *NomPrise(Prise));
	QuitterPriseCourante();
	SetEtat(EMansaEtatEscalade::Aucun);
	TempsDebutChute = GetWorld()->GetTimeSeconds();
	PasserEnChute(FVector::ZeroVector);
	OnPriseEffritee.Broadcast(Prise);
	OnLacher.Broadcast(Prise);
}

void UMansaClimbingComponent::Reinitialiser()
{
	SetComponentTickEnabled(false);
	if (PriseCourante)
	{
		PriseCourante->OnEffritement.RemoveDynamic(this, &UMansaClimbingComponent::OnPriseEffriteeInterne);
	}
	PriseCourante = nullptr;
	PriseCible = nullptr;
	PriseIgnoree = nullptr;
	bLacherVolontaire = false;
	TempsDebutChute = -1000.f;
	Trajet.Reset();
	SetEtat(EMansaEtatEscalade::Aucun);
}

// ---------------------------------------------------------------------------
// Mouvement interpolé et chute
// ---------------------------------------------------------------------------

void UMansaClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACharacter* Perso = GetPersonnage();
	if (!Perso || Trajet.Num() < 2 || DureeMouvement <= 0.f)
	{
		TerminerMouvement();
		return;
	}
	TempsMouvement += DeltaTime;
	const float Alpha = FMath::Clamp(TempsMouvement / DureeMouvement, 0.f, 1.f);
	const float Distance = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f) * LongueursCumulees.Last();

	int32 Segment = 1;
	while (Segment < LongueursCumulees.Num() - 1 && LongueursCumulees[Segment] < Distance)
	{
		++Segment;
	}
	const float Longueur = LongueursCumulees[Segment] - LongueursCumulees[Segment - 1];
	const float AlphaSegment = Longueur > KINDA_SMALL_NUMBER ? (Distance - LongueursCumulees[Segment - 1]) / Longueur : 1.f;
	FVector Position = FMath::Lerp(Trajet[Segment - 1], Trajet[Segment], FMath::Clamp(AlphaSegment, 0.f, 1.f));
	// Petit arc pendant les sauts (lisibilité, en attendant les animations).
	Position.Z += FMath::Sin(Alpha * PI) * ArcMouvement;

	// Pas de balayage : la destination a été validée par des tests de capsule.
	Perso->SetActorLocation(Position, false, nullptr, ETeleportType::None);

	if (Alpha >= 1.f)
	{
		TerminerMouvement();
	}
}

void UMansaClimbingComponent::TerminerMouvement()
{
	SetComponentTickEnabled(false);
	if (Etat == EMansaEtatEscalade::Saut)
	{
		UMansaPriseComponent* Cible = PriseCible;
		PriseCible = nullptr;
		if (Cible && Cible->EstDisponible())
		{
			DevenirSuspendu(Cible, PointCible);
		}
		else
		{
			UE_LOG(LogMansa, Log, TEXT("Escalade : la prise %s n'est plus là, Sira tombe"), *NomPrise(Cible));
			if (Cible)
			{
				NormaleChute = Cible->GetNormale();
			}
			SetEtat(EMansaEtatEscalade::Aucun);
			TempsDebutChute = GetWorld()->GetTimeSeconds();
			PasserEnChute(FVector::ZeroVector);
		}
	}
	else if (Etat == EMansaEtatEscalade::Mantle)
	{
		UMansaPriseComponent* Prise = PriseCourante;
		QuitterPriseCourante();
		SetEtat(EMansaEtatEscalade::Aucun);
		PasserEnChute(FVector::ZeroVector);
		UE_LOG(LogMansa, Log, TEXT("Escalade : mantle terminé, Sira est sur le rebord %s"), *TexteVecteur(GetOwner()->GetActorLocation()));
		OnMantleFin.Broadcast(Prise);
	}
}

void UMansaClimbingComponent::OnModeMouvementChange(ACharacter* Personnage, EMovementMode ModePrecedent, uint8 ModeCustomPrecedent)
{
	if (!Personnage || !GetWorld())
	{
		return;
	}
	FTimerManager& TM = GetWorld()->GetTimerManager();
	if (Personnage->GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		if (!TM.IsTimerActive(TimerChute))
		{
			TM.SetTimer(TimerChute, this, &UMansaClimbingComponent::VerifierAccrocheEnChute, PeriodeDetectionChute, true);
		}
	}
	else
	{
		TM.ClearTimer(TimerChute);
		// Au sol : on oublie le lâcher volontaire (l'accroche auto en chute redevient possible).
		bLacherVolontaire = false;
	}
}

void UMansaClimbingComponent::VerifierAccrocheEnChute()
{
	// Après un lâcher volontaire, on ne se raccroche pas tout seul (il faut appuyer sur Saut).
	if (!bActif || Etat != EMansaEtatEscalade::Aucun || bLacherVolontaire)
	{
		return;
	}
	FVector Q;
	if (UMansaPriseComponent* P = ChercherPriseAuContact(Q))
	{
		UE_LOG(LogMansa, Log, TEXT("Escalade : Sira attrape %s en tombant"), *NomPrise(P));
		DemarrerAccroche(P, Q, DureeAccroche, 0.f);
	}
}
