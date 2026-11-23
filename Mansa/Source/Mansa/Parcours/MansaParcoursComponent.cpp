// MANSA — Checkpoints et rattrapage du parcours (D6, GDD §4.4).

#include "MansaParcoursComponent.h"
#include "MansaEscaladeSettings.h"
#include "MansaPrisesSubsystem.h"
#include "MansaClimbingComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Mansa.h"

UMansaParcoursComponent::UMansaParcoursComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ACharacter* UMansaParcoursComponent::GetPersonnage() const
{
	return Cast<ACharacter>(GetOwner());
}

void UMansaParcoursComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* Perso = GetPersonnage();
	if (!Perso)
	{
		return;
	}
	// Checkpoint initial : là où Sira apparaît (pieds), ordre -1 pour que tout checkpoint du niveau passe devant.
	Courant.Id = TEXT("Depart");
	Courant.Ordre = -1;
	Courant.Transform = FTransform(FRotator(0.f, Perso->GetActorRotation().Yaw, 0.f),
		Perso->GetActorLocation() - FVector(0.f, 0.f, Perso->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

	CollecterCheckpoints();
	LierVolumesRattrapage();

	const UMansaEscaladeSettings* S = GetDefault<UMansaEscaladeSettings>();
	GetWorld()->GetTimerManager().SetTimer(TimerVerification, this, &UMansaParcoursComponent::VerifierCheckpoints, S->PeriodeVerificationCheckpoint, true);
}

void UMansaParcoursComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerVerification);
		World->GetTimerManager().ClearTimer(TimerRattrapage);
	}
	for (AActor* Volume : VolumesLies)
	{
		if (IsValid(Volume))
		{
			Volume->OnActorBeginOverlap.RemoveDynamic(this, &UMansaParcoursComponent::OnVolumeRattrapage);
		}
	}
	VolumesLies.Reset();
	Super::EndPlay(EndPlayReason);
}

void UMansaParcoursComponent::CollecterCheckpoints()
{
	const UMansaEscaladeSettings* S = GetDefault<UMansaEscaladeSettings>();
	Checkpoints.Reset();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		const AActor* Acteur = *It;
		if (!Acteur->ActorHasTag(S->TagCheckpoint))
		{
			continue;
		}
		FMansaCheckpoint C;
		C.Ordre = S->OrdreCheckpoints.Num();
		C.Id = FName(*Acteur->GetName());
		// Identifiant = premier tag présent dans OrdreCheckpoints (ex. CP1).
		for (const FName& Tag : Acteur->Tags)
		{
			const int32 Index = S->OrdreCheckpoints.IndexOfByKey(Tag);
			if (Index != INDEX_NONE)
			{
				C.Id = Tag;
				C.Ordre = Index;
				break;
			}
		}
		C.Transform = FTransform(FRotator(0.f, Acteur->GetActorRotation().Yaw, 0.f), Acteur->GetActorLocation());
		Checkpoints.Add(C);
	}
	Checkpoints.Sort([](const FMansaCheckpoint& A, const FMansaCheckpoint& B) { return A.Ordre < B.Ordre; });
	FString Liste;
	for (const FMansaCheckpoint& C : Checkpoints)
	{
		Liste += FString::Printf(TEXT(" %s"), *C.Id.ToString());
	}
	UE_LOG(LogMansa, Log, TEXT("Parcours : %d checkpoint(s) tagué(s) '%s' :%s"), Checkpoints.Num(), *S->TagCheckpoint.ToString(), *Liste);
}

void UMansaParcoursComponent::LierVolumesRattrapage()
{
	const UMansaEscaladeSettings* S = GetDefault<UMansaEscaladeSettings>();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Acteur = *It;
		if (Acteur->ActorHasTag(S->TagRattrapage))
		{
			Acteur->OnActorBeginOverlap.AddUniqueDynamic(this, &UMansaParcoursComponent::OnVolumeRattrapage);
			VolumesLies.Add(Acteur);
		}
	}
	UE_LOG(LogMansa, Log, TEXT("Parcours : %d volume(s) de rattrapage tagué(s) '%s'"), VolumesLies.Num(), *S->TagRattrapage.ToString());
}

void UMansaParcoursComponent::OnVolumeRattrapage(AActor* Volume, AActor* Autre)
{
	if (Autre == GetOwner())
	{
		UE_LOG(LogMansa, Log, TEXT("Parcours : Sira entre dans %s"), *GetNameSafe(Volume));
		DeclencherRattrapage();
	}
}

void UMansaParcoursComponent::VerifierCheckpoints()
{
	ACharacter* Perso = GetPersonnage();
	if (!Perso || bRattrapageEnCours)
	{
		return;
	}
	const UMansaEscaladeSettings* S = GetDefault<UMansaEscaladeSettings>();
	if (Perso->GetActorLocation().Z < S->AltitudeRattrapage)
	{
		UE_LOG(LogMansa, Log, TEXT("Parcours : Sira est sous l'altitude de rattrapage (%.0f)"), S->AltitudeRattrapage);
		DeclencherRattrapage();
		return;
	}
	// On n'active un checkpoint que debout au sol (pas en plein saut ni suspendue).
	if (!Perso->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}
	const FVector Pieds = Perso->GetActorLocation() - FVector(0.f, 0.f, Perso->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	for (const FMansaCheckpoint& C : Checkpoints)
	{
		if (C.Ordre > Courant.Ordre && FVector::Dist(Pieds, C.Transform.GetLocation()) <= S->RayonActivationCheckpoint)
		{
			Courant = C;
			UE_LOG(LogMansa, Log, TEXT("Parcours : checkpoint %s atteint"), *C.Id.ToString());
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("Checkpoint %s"), *C.Id.ToString()));
			}
			OnCheckpointAtteint.Broadcast(C.Id);
		}
	}
}

void UMansaParcoursComponent::DeclencherRattrapage()
{
	ACharacter* Perso = GetPersonnage();
	if (!Perso || bRattrapageEnCours)
	{
		return;
	}
	bRattrapageEnCours = true;
	const float Duree = GetDefault<UMansaEscaladeSettings>()->DureeFonduRattrapage;
	UE_LOG(LogMansa, Log, TEXT("Parcours : rattrapage -> retour au checkpoint %s"), *Courant.Id.ToString());

	if (APlayerController* PC = Cast<APlayerController>(Perso->GetController()))
	{
		Perso->DisableInput(PC);
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, Duree, FLinearColor::Black, false, true);
		}
	}
	if (Duree > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerRattrapage, this, &UMansaParcoursComponent::RetourAuCheckpoint, Duree, false);
	}
	else
	{
		RetourAuCheckpoint();
	}
}

void UMansaParcoursComponent::PlacerAu(const FMansaCheckpoint& Checkpoint)
{
	ACharacter* Perso = GetPersonnage();
	if (!Perso)
	{
		return;
	}
	if (UMansaClimbingComponent* Escalade = Perso->FindComponentByClass<UMansaClimbingComponent>())
	{
		Escalade->Reinitialiser();
	}
	UCharacterMovementComponent* CMC = Perso->GetCharacterMovement();
	CMC->StopMovementImmediately();
	const FVector Centre = Checkpoint.Transform.GetLocation() + FVector(0.f, 0.f, Perso->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.f);
	const FRotator Rotation = Checkpoint.Transform.Rotator();
	Perso->TeleportTo(Centre, Rotation, false, true);
	CMC->SetMovementMode(MOVE_Falling);
	if (AController* C = Perso->GetController())
	{
		C->SetControlRotation(Rotation);
	}
}

void UMansaParcoursComponent::RetourAuCheckpoint()
{
	PlacerAu(Courant);
	if (UMansaPrisesSubsystem* Prises = UMansaPrisesSubsystem::Get(this))
	{
		Prises->ReinitialiserPrises();
	}
	OnRattrapage.Broadcast();

	const UMansaEscaladeSettings* S = GetDefault<UMansaEscaladeSettings>();
	// Réplique du narrateur (placeholder : texte à l'écran en attendant le griot et l'UI).
	UE_LOG(LogMansa, Log, TEXT("Griot : %s"), *S->RepliqueRattrapage.ToString());
	if (GEngine && !S->RepliqueRattrapage.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1, S->DureeAffichageReplique, FColor(255, 220, 150), S->RepliqueRattrapage.ToString());
	}

	const ACharacter* Perso = GetPersonnage();
	if (const APlayerController* PC = Perso ? Cast<APlayerController>(Perso->GetController()) : nullptr)
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, S->DureeFonduRattrapage, FLinearColor::Black, false, false);
		}
	}
	if (S->DureeFonduRattrapage > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerRattrapage, this, &UMansaParcoursComponent::FinRattrapage, S->DureeFonduRattrapage, false);
	}
	else
	{
		FinRattrapage();
	}
}

void UMansaParcoursComponent::FinRattrapage()
{
	bRattrapageEnCours = false;
	ACharacter* Perso = GetPersonnage();
	if (APlayerController* PC = Perso ? Cast<APlayerController>(Perso->GetController()) : nullptr)
	{
		Perso->EnableInput(PC);
	}
}

bool UMansaParcoursComponent::AllerAuCheckpoint(FName Id)
{
	for (const FMansaCheckpoint& C : Checkpoints)
	{
		if (C.Id == Id)
		{
			Courant = C;
			PlacerAu(C);
			if (UMansaPrisesSubsystem* Prises = UMansaPrisesSubsystem::Get(this))
			{
				Prises->ReinitialiserPrises();
			}
			UE_LOG(LogMansa, Log, TEXT("Parcours : téléportation au checkpoint %s"), *Id.ToString());
			return true;
		}
	}
	UE_LOG(LogMansa, Warning, TEXT("Parcours : checkpoint '%s' inconnu"), *Id.ToString());
	return false;
}
