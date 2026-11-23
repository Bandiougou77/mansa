// MANSA — Prise d'escalade (D3, GDD §4.4).

#include "MansaPriseComponent.h"
#include "MansaEscaladeSettings.h"
#include "MansaPrisesSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Mansa.h"

UMansaPriseComponent::UMansaPriseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Reglages = ReglagesDeRepli(TypePrise);
}

FMansaPriseRow UMansaPriseComponent::ReglagesDeRepli(EMansaPriseType Type)
{
	// Mêmes valeurs que design/data/DT_PrisesEscalade.csv : ne servent que si la DT est absente.
	FMansaPriseRow R;
	switch (Type)
	{
	case EMansaPriseType::BancoFragile:
		R.DelaiEffritement = 1.5f;
		R.AmplitudeTremblement = 2.5f;
		break;
	case EMansaPriseType::Corniche:
		R.bDeplacementLateral = true;
		R.bMantleAutorise = true;
		break;
	case EMansaPriseType::Reserve_CrepiFrais:
		R.bSaisissable = false;
		break;
	default:
		break;
	}
	return R;
}

void UMansaPriseComponent::BeginPlay()
{
	Super::BeginPlay();
	Initialiser();
	if (UMansaPrisesSubsystem* Sub = UMansaPrisesSubsystem::Get(this))
	{
		Sub->Enregistrer(this);
	}
}

void UMansaPriseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerEffritement);
		World->GetTimerManager().ClearTimer(TimerTremblement);
	}
	if (UMansaPrisesSubsystem* Sub = UMansaPrisesSubsystem::Get(this))
	{
		Sub->Desenregistrer(this);
	}
	Super::EndPlay(EndPlayReason);
}

void UMansaPriseComponent::Initialiser()
{
	if (bLireTagsActeur)
	{
		LireTags();
	}
	ChargerReglages();
	CalculerGeometrie();

	if (bIgnorerCollisionPawn && GetOwner())
	{
		TArray<UPrimitiveComponent*> Primitives;
		GetOwner()->GetComponents<UPrimitiveComponent>(Primitives);
		for (UPrimitiveComponent* Prim : Primitives)
		{
			Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		}
	}
	if (GetOwner())
	{
		PositionInitiale = GetOwner()->GetActorLocation();
	}
}

void UMansaPriseComponent::LireTags()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	const UEnum* EnumType = StaticEnum<EMansaPriseType>();
	for (const FName& Tag : Owner->Tags)
	{
		const FString S = Tag.ToString();
		if (S.StartsWith(TEXT("PriseType_")))
		{
			const FString Valeur = S.RightChop(10);
			const int64 V = EnumType->GetValueByNameString(Valeur);
			if (V != INDEX_NONE)
			{
				TypePrise = static_cast<EMansaPriseType>(V);
			}
			else
			{
				UE_LOG(LogMansa, Warning, TEXT("Prise %s : type inconnu dans le tag '%s'"), *GetNameSafe(Owner), *S);
			}
		}
		else if (S.StartsWith(TEXT("PriseId_")))
		{
			IdPrise = FName(*S.RightChop(8));
		}
		else if (S == TEXT("MantleAutorise"))
		{
			MantleAutorise = EMansaSurcharge::Oui;
		}
		else if (S == TEXT("MantleInterdit"))
		{
			MantleAutorise = EMansaSurcharge::Non;
		}
	}
}

void UMansaPriseComponent::ChargerReglages()
{
	const UMansaEscaladeSettings* Settings = GetDefault<UMansaEscaladeSettings>();
	const UDataTable* Table = Settings ? Settings->TablePrisesEscalade.LoadSynchronous() : nullptr;
	const FName Ligne(*StaticEnum<EMansaPriseType>()->GetNameStringByValue(static_cast<int64>(TypePrise)));
	const FMansaPriseRow* Row = Table ? Table->FindRow<FMansaPriseRow>(Ligne, TEXT("MansaPrise"), false) : nullptr;
	if (Row)
	{
		Reglages = *Row;
	}
	else
	{
		Reglages = ReglagesDeRepli(TypePrise);
		UE_LOG(LogMansa, Warning, TEXT("Prise %s : ligne '%s' introuvable dans DT_PrisesEscalade (%s), valeurs de repli."),
			*GetNomDebug(), *Ligne.ToString(), Table ? *Table->GetName() : TEXT("table absente"));
	}
}

void UMansaPriseComponent::CalculerGeometrie()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	FVector Origine, Etendue;
	Owner->GetActorBounds(false, Origine, Etendue);
	Centre = Origine;

	// Normale : tag explicite (le générateur le pose, car un cylindre couché a un Pitch de 90°
	// qui rend le Yaw ambigu), sinon Yaw de l'acteur.
	float Yaw = Owner->GetActorRotation().Yaw;
	for (const FName& Tag : Owner->Tags)
	{
		const FString S = Tag.ToString();
		if (S.StartsWith(TEXT("PriseNormaleYaw_")))
		{
			Yaw = FCString::Atof(*S.RightChop(16));
		}
	}
	Normale = FRotator(0.f, Yaw, 0.f).Vector().GetSafeNormal2D();

	bSegment = TypePrise == EMansaPriseType::Corniche;
	if (bSegment)
	{
		const FVector Axe = Etendue.X >= Etendue.Y ? FVector(Etendue.X, 0.f, 0.f) : FVector(0.f, Etendue.Y, 0.f);
		SegmentA = Centre - Axe;
		SegmentB = Centre + Axe;
	}
	else
	{
		SegmentA = SegmentB = Centre;
	}
}

FVector UMansaPriseComponent::GetPointLePlusProche(const FVector& Point) const
{
	if (!bSegment)
	{
		return Centre;
	}
	return FMath::ClosestPointOnSegment(Point, SegmentA, SegmentB);
}

bool UMansaPriseComponent::PeutMantle() const
{
	switch (MantleAutorise)
	{
	case EMansaSurcharge::Oui: return true;
	case EMansaSurcharge::Non: return false;
	default: return Reglages.bMantleAutorise;
	}
}

FString UMansaPriseComponent::GetNomDebug() const
{
	if (!IdPrise.IsNone())
	{
		return IdPrise.ToString();
	}
#if WITH_EDITOR
	if (GetOwner())
	{
		return GetOwner()->GetActorLabel();
	}
#endif
	return GetNameSafe(GetOwner());
}

void UMansaPriseComponent::NotifierAccroche()
{
	if (bEffritementLance || bEffritee || Reglages.DelaiEffritement <= 0.f || !GetWorld())
	{
		return;
	}
	bEffritementLance = true;
	TempsDebutEffritement = GetWorld()->GetTimeSeconds();
	PositionInitiale = GetOwner()->GetActorLocation();
	UE_LOG(LogMansa, Log, TEXT("Prise %s (%s) : le banco s'effrite... chute dans %.1f s"),
		*GetNomDebug(), *StaticEnum<EMansaPriseType>()->GetNameStringByValue(static_cast<int64>(TypePrise)), Reglages.DelaiEffritement);

	FTimerManager& TM = GetWorld()->GetTimerManager();
	TM.SetTimer(TimerEffritement, this, &UMansaPriseComponent::Effriter, Reglages.DelaiEffritement, false);
	// Le tremblement n'est possible que si la racine est mobile (BP_PriseEscalade l'est).
	const USceneComponent* Racine = GetOwner()->GetRootComponent();
	if (Reglages.AmplitudeTremblement > 0.f && Racine && Racine->Mobility == EComponentMobility::Movable)
	{
		TM.SetTimer(TimerTremblement, this, &UMansaPriseComponent::Trembler, PeriodeTremblement, true);
	}
	OnDebutEffritement.Broadcast(this);
}

void UMansaPriseComponent::Trembler()
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return;
	}
	// Amplitude croissante : le joueur sent que ça va lâcher.
	const float Alpha = FMath::Clamp((GetWorld()->GetTimeSeconds() - TempsDebutEffritement) / FMath::Max(Reglages.DelaiEffritement, KINDA_SMALL_NUMBER), 0.f, 1.f);
	const FVector Decalage = FMath::VRand() * Reglages.AmplitudeTremblement * (0.3f + 0.7f * Alpha);
	Owner->SetActorLocation(PositionInitiale + Decalage, false, nullptr, ETeleportType::TeleportPhysics);
}

void UMansaPriseComponent::Effriter()
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(TimerTremblement);
	if (Owner->GetRootComponent() && Owner->GetRootComponent()->Mobility == EComponentMobility::Movable)
	{
		Owner->SetActorLocation(PositionInitiale, false, nullptr, ETeleportType::TeleportPhysics);
	}
	bEffritee = true;
	Owner->SetActorHiddenInGame(true);
	Owner->SetActorEnableCollision(false);
	UE_LOG(LogMansa, Log, TEXT("Prise %s : effritée, elle a disparu."), *GetNomDebug());
	OnEffritement.Broadcast(this);
}

void UMansaPriseComponent::Reinitialiser()
{
	AActor* Owner = GetOwner();
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerEffritement);
		GetWorld()->GetTimerManager().ClearTimer(TimerTremblement);
	}
	const bool bEtaitModifiee = bEffritementLance || bEffritee;
	bEffritementLance = false;
	bEffritee = false;
	if (Owner && bEtaitModifiee)
	{
		if (Owner->GetRootComponent() && Owner->GetRootComponent()->Mobility == EComponentMobility::Movable)
		{
			Owner->SetActorLocation(PositionInitiale, false, nullptr, ETeleportType::TeleportPhysics);
		}
		Owner->SetActorHiddenInGame(false);
		Owner->SetActorEnableCollision(true);
		UE_LOG(LogMansa, Log, TEXT("Prise %s : remise en place."), *GetNomDebug());
	}
}
