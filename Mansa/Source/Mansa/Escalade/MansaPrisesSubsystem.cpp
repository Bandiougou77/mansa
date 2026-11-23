// MANSA — Registre des prises d'escalade du monde (D3).

#include "MansaPrisesSubsystem.h"
#include "MansaPriseComponent.h"
#include "MansaEscaladeSettings.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Mansa.h"

namespace
{
	/** Période de rafraîchissement de l'affichage de debug (s). */
	constexpr float PeriodeDebug = 0.25f;

	FColor CouleurType(EMansaPriseType Type)
	{
		switch (Type)
		{
		case EMansaPriseType::Toron: return FColor(139, 90, 43);
		case EMansaPriseType::BancoRelief: return FColor(230, 190, 120);
		case EMansaPriseType::BancoFragile: return FColor::Red;
		case EMansaPriseType::Corniche: return FColor::Cyan;
		default: return FColor::Silver;
		}
	}
}

UMansaPrisesSubsystem* UMansaPrisesSubsystem::Get(const UObject* ContexteMonde)
{
	const UWorld* World = ContexteMonde ? ContexteMonde->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UMansaPrisesSubsystem>() : nullptr;
}

bool UMansaPrisesSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMansaPrisesSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Repli : prises taguées sans composant (ex. StaticMeshActor d'un greybox généré sans BP_PriseEscalade).
	const FName TagPrise = GetDefault<UMansaEscaladeSettings>()->TagPrise;
	int32 Ajouts = 0;
	for (TActorIterator<AActor> It(&InWorld); It; ++It)
	{
		AActor* Acteur = *It;
		if (Acteur->ActorHasTag(TagPrise) && !Acteur->FindComponentByClass<UMansaPriseComponent>())
		{
			UMansaPriseComponent* Comp = NewObject<UMansaPriseComponent>(Acteur, TEXT("PriseAuto"));
			Acteur->AddInstanceComponent(Comp);
			Comp->RegisterComponent();
			++Ajouts;
		}
	}
	if (Ajouts > 0)
	{
		UE_LOG(LogMansa, Log, TEXT("Escalade : %d prise(s) taguée(s) '%s' sans composant -> UMansaPriseComponent ajouté au démarrage."), Ajouts, *TagPrise.ToString());
	}
}

void UMansaPrisesSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerDebug);
	}
	Prises.Reset();
	Super::Deinitialize();
}

void UMansaPrisesSubsystem::Enregistrer(UMansaPriseComponent* Prise)
{
	if (Prise)
	{
		Prises.AddUnique(Prise);
	}
}

void UMansaPrisesSubsystem::Desenregistrer(UMansaPriseComponent* Prise)
{
	Prises.Remove(Prise);
}

TArray<UMansaPriseComponent*> UMansaPrisesSubsystem::GetPrises() const
{
	TArray<UMansaPriseComponent*> Resultat;
	Resultat.Reserve(Prises.Num());
	for (const TWeakObjectPtr<UMansaPriseComponent>& P : Prises)
	{
		if (P.IsValid())
		{
			Resultat.Add(P.Get());
		}
	}
	return Resultat;
}

UMansaPriseComponent* UMansaPrisesSubsystem::TrouverPrise(FName IdPrise) const
{
	for (const TWeakObjectPtr<UMansaPriseComponent>& P : Prises)
	{
		if (P.IsValid() && P->IdPrise == IdPrise)
		{
			return P.Get();
		}
	}
	return nullptr;
}

void UMansaPrisesSubsystem::ReinitialiserPrises()
{
	for (const TWeakObjectPtr<UMansaPriseComponent>& P : Prises)
	{
		if (P.IsValid())
		{
			P->Reinitialiser();
		}
	}
}

void UMansaPrisesSubsystem::SetDebugPrises(bool bActif)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (bActif)
	{
		World->GetTimerManager().SetTimer(TimerDebug, this, &UMansaPrisesSubsystem::DessinerDebug, PeriodeDebug, true, 0.f);
	}
	else
	{
		World->GetTimerManager().ClearTimer(TimerDebug);
	}
	UE_LOG(LogMansa, Log, TEXT("Escalade : debug des prises %s (%d prises)"), bActif ? TEXT("affiché") : TEXT("masqué"), GetPrises().Num());
}

void UMansaPrisesSubsystem::DessinerDebug()
{
	UWorld* World = GetWorld();
	const UEnum* EnumType = StaticEnum<EMansaPriseType>();
	for (const UMansaPriseComponent* P : GetPrises())
	{
		const FColor Couleur = P->EstDisponible() ? CouleurType(P->TypePrise) : FColor(60, 60, 60);
		const FVector C = P->GetCentre();
		if (P->EstSegment())
		{
			FVector A, B;
			P->GetSegment(A, B);
			DrawDebugLine(World, A, B, Couleur, false, PeriodeDebug * 1.1f, 0, 3.f);
		}
		DrawDebugSphere(World, C, 10.f, 8, Couleur, false, PeriodeDebug * 1.1f);
		DrawDebugDirectionalArrow(World, C, C + P->GetNormale() * 40.f, 10.f, Couleur, false, PeriodeDebug * 1.1f);
		const FString Texte = FString::Printf(TEXT("%s %s%s"), *P->GetNomDebug(), *EnumType->GetNameStringByValue(static_cast<int64>(P->TypePrise)),
			P->PeutMantle() ? TEXT(" [mantle]") : TEXT(""));
		DrawDebugString(World, C + FVector(0.f, 0.f, 25.f), Texte, nullptr, Couleur, PeriodeDebug * 1.1f, true);
	}
}
