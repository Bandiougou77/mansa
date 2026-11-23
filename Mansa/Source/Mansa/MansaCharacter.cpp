// Copyright Epic Games, Inc. All Rights Reserved.

#include "MansaCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Engine/Engine.h"
#include "Mansa.h"
#include "MansaCameraProfileComponent.h"
#include "MansaTraversalComponent.h"
#include "MansaClimbingComponent.h"
#include "MansaParcoursComponent.h"
#include "MansaPrisesSubsystem.h"
#include "MansaPriseComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"

AMansaCharacter::AMansaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Vitesses, saut, contrôle en l'air et rotation : lus dans DT_CharacterMovement au BeginPlay
	// (ChargerReglagesMouvement). Les réglages ci-dessous sont du confort de CMC, modifiables dans BP_Sira.
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Profils de caméra (bras, FOV, décalage) pilotés par DT_CameraProfiles
	CameraProfiles = CreateDefaultSubobject<UMansaCameraProfileComponent>(TEXT("CameraProfiles"));
	CameraProfiles->SetCibles(CameraBoom, FollowCamera);

	// Enjambement / mantle, déclenchés par Saut (et automatiquement en course pour les murets)
	Traversal = CreateDefaultSubobject<UMansaTraversalComponent>(TEXT("Traversal"));

	// Escalade à prises (Saut pour s'accrocher / sauter, stick pour viser, IA_DropHold pour lâcher)
	Climbing = CreateDefaultSubobject<UMansaClimbingComponent>(TEXT("Climbing"));

	// Checkpoints et rattrapage sans mort
	Parcours = CreateDefaultSubobject<UMansaParcoursComponent>(TEXT("Parcours"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMansaCharacter::BeginPlay()
{
	Super::BeginPlay();
	ChargerReglagesMouvement();
	if (Climbing)
	{
		Climbing->OnEtatChange.AddUniqueDynamic(this, &AMansaCharacter::OnEtatEscaladeChange);
	}
}

void AMansaCharacter::OnEtatEscaladeChange(EMansaEtatEscalade Ancien, EMansaEtatEscalade Nouveau)
{
	const bool bAvant = Ancien != EMansaEtatEscalade::Aucun;
	const bool bApres = Nouveau != EMansaEtatEscalade::Aucun;
	if (bAvant == bApres)
	{
		return;
	}
	SetEscaladeContextActive(bApres);
	SetCameraProfile(bApres ? FName(TEXT("Escalade")) : FName(TEXT("Exploration")));
}

void AMansaCharacter::MansaDebugPrises(bool bActif)
{
	if (UMansaPrisesSubsystem* Sub = UMansaPrisesSubsystem::Get(this))
	{
		Sub->SetDebugPrises(bActif);
	}
	if (Climbing)
	{
		Climbing->bDebug = bActif;
	}
}

void AMansaCharacter::MansaCheckpoint(FName IdCheckpoint)
{
	if (Parcours)
	{
		Parcours->AllerAuCheckpoint(IdCheckpoint);
	}
}

void AMansaCharacter::MansaRattrapage()
{
	if (Parcours)
	{
		Parcours->DeclencherRattrapage();
	}
}

void AMansaCharacter::MansaTestEscalade(FName Option, bool bQuitterALaFin)
{
	// Étapes : « accroche:<Id> » (approche au sol puis Saut), « saut:<Id> », « glisser:<Id> » (corniche), « mantle ».
	// Même chemin que tools/unreal/verifier_chemin_escalade.py.
	EtapesTestEscalade = {
		TEXT("accroche:D1-01"), TEXT("saut:D1-03"), TEXT("saut:D1-05"), TEXT("mantle"),
		TEXT("accroche:D2-01"), TEXT("saut:D2-02"), TEXT("saut:D2-03"), TEXT("saut:D2-04"), TEXT("saut:D2-05"),
		TEXT("saut:D2-06"), TEXT("saut:D2-07"), TEXT("saut:D2-08"), TEXT("mantle"),
		TEXT("accroche:D3-01"), TEXT("saut:D3-02") };
	if (Option == TEXT("B"))
	{
		EtapesTestEscalade.Append({ TEXT("saut:D3-B1"), TEXT("saut:D3-B2"), TEXT("saut:D3-B3"), TEXT("saut:D3-B4"),
			TEXT("saut:D3-B5"), TEXT("saut:D3-CorB"), TEXT("mantle") });
	}
	else
	{
		// A2 -> CorB : on glisse le long de la corniche A2 jusqu'à l'enchaînement sur CorB.
		EtapesTestEscalade.Append({ TEXT("saut:D3-A1"), TEXT("saut:D3-A2"), TEXT("glisser:D3-CorB"), TEXT("mantle") });
	}
	bQuitterApresTestEscalade = bQuitterALaFin;
	IndexTestEscalade = 0;
	UE_LOG(LogMansa, Log, TEXT("Test escalade : option %s, %d étapes"), *Option.ToString(), EtapesTestEscalade.Num());
	if (!Parcours || !Parcours->AllerAuCheckpoint(TEXT("CP1")))
	{
		FinTestEscalade(false, TEXT("checkpoint CP1 introuvable"));
		return;
	}
	GetWorldTimerManager().SetTimer(TimerTestEscalade, this, &AMansaCharacter::EtapeTestEscalade, 1.f, false);
}

void AMansaCharacter::EtapeTestEscalade()
{
	const UMansaPrisesSubsystem* Sub = UMansaPrisesSubsystem::Get(this);
	if (!Climbing || !Sub)
	{
		FinTestEscalade(false, TEXT("composant d'escalade ou registre des prises absent"));
		return;
	}
	// Bilan de l'étape précédente.
	if (IndexTestEscalade > 0)
	{
		const FString& Precedente = EtapesTestEscalade[IndexTestEscalade - 1];
		if (Precedente == TEXT("mantle"))
		{
			if (Climbing->EstEnEscalade() || !GetCharacterMovement()->IsMovingOnGround())
			{
				FinTestEscalade(false, TEXT("mantle non terminé : Sira n'est pas debout sur le rebord"));
				return;
			}
		}
		else
		{
			const FString Id = Precedente.RightChop(Precedente.Find(TEXT(":")) + 1);
			const UMansaPriseComponent* Tenue = Climbing->GetPriseCourante();
			if (!Climbing->EstSuspendu() || !Tenue || Tenue->IdPrise != FName(*Id))
			{
				FinTestEscalade(false, FString::Printf(TEXT("après '%s', Sira tient %s"), *Precedente, Tenue ? *Tenue->GetNomDebug() : TEXT("<rien>")));
				return;
			}
		}
		const float Pieds = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		UE_LOG(LogMansa, Log, TEXT("Test escalade : OK  %-16s (pieds Z=%.0f)"), *Precedente, Pieds);
	}
	if (!EtapesTestEscalade.IsValidIndex(IndexTestEscalade))
	{
		const float Pieds = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		FinTestEscalade(Pieds > 600.f, FString::Printf(TEXT("arrivée, pieds à Z=%.0f (terrasse attendue vers 650)"), Pieds));
		return;
	}

	const FString Etape = EtapesTestEscalade[IndexTestEscalade++];
	bool bLance = false;
	if (Etape == TEXT("mantle"))
	{
		bLance = Climbing->TenterMantle();
	}
	else
	{
		const FString Id = Etape.RightChop(Etape.Find(TEXT(":")) + 1);
		UMansaPriseComponent* Prise = Sub->TrouverPrise(FName(*Id));
		if (!Prise)
		{
			FinTestEscalade(false, FString::Printf(TEXT("prise %s absente du monde"), *Id));
			return;
		}
		if (Etape.StartsWith(TEXT("accroche:")))
		{
			// Approche : Sira au sol, 90 cm devant la prise, face au mur.
			const FVector N = Prise->GetNormale();
			const FVector Point = Prise->GetCentre();
			const float DemiHauteur = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			FVector Depart = Point + N * 90.f;
			FHitResult Sol;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(MansaTestEscalade), false, this);
			if (GetWorld()->LineTraceSingleByChannel(Sol, FVector(Depart.X, Depart.Y, Point.Z), FVector(Depart.X, Depart.Y, Point.Z - 400.f), ECC_Pawn, Params))
			{
				Depart.Z = Sol.ImpactPoint.Z + DemiHauteur + 2.f;
			}
			GetCharacterMovement()->StopMovementImmediately();
			TeleportTo(Depart, FRotator(0.f, (-N).Rotation().Yaw, 0.f));
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			bLance = Climbing->TenterAccroche();
		}
		else if (Etape.StartsWith(TEXT("glisser:")))
		{
			// Glissement au stick, accéléré (×5) pour le test, jusqu'à tenir la prise visée.
			CibleGlissement = Prise;
			TempsGlissement = 0.f;
			GetWorldTimerManager().SetTimer(TimerTestEscalade, this, &AMansaCharacter::GlissementTestEscalade, 0.02f, true);
			return;
		}
		else
		{
			bLance = Climbing->SauterVersPrise(Prise);
		}
	}
	if (!bLance)
	{
		FinTestEscalade(false, FString::Printf(TEXT("étape '%s' refusée (voir les logs Escalade ci-dessus)"), *Etape));
		return;
	}
	GetWorldTimerManager().SetTimer(TimerTestEscalade, this, &AMansaCharacter::EtapeTestEscalade, 1.f, false);
}

void AMansaCharacter::GlissementTestEscalade()
{
	UMansaPriseComponent* Cible = CibleGlissement.Get();
	if (!Climbing || !Cible)
	{
		FinTestEscalade(false, TEXT("glissement : prise visée perdue"));
		return;
	}
	if (Climbing->GetPriseCourante() == Cible)
	{
		// Arrivée : bilan à la prochaine étape.
		GetWorldTimerManager().SetTimer(TimerTestEscalade, this, &AMansaCharacter::EtapeTestEscalade, 0.5f, false);
		return;
	}
	TempsGlissement += 0.02f;
	const float Sens = Climbing->DirectionVers(Cible->GetPointLePlusProche(Climbing->GetPointMain())).X >= 0.f ? 1.f : -1.f;
	if (TempsGlissement > 10.f || !Climbing->DeplacerSurCorniche(Sens, 0.1f))
	{
		FinTestEscalade(false, FString::Printf(TEXT("glissement bloqué vers %s (main %s)"), *Cible->GetNomDebug(), *Climbing->GetPointMain().ToString()));
	}
}

void AMansaCharacter::FinTestEscalade(bool bSucces, const FString& Raison)
{
	GetWorldTimerManager().ClearTimer(TimerTestEscalade);
	const FString Message = FString::Printf(TEXT("Test escalade : %s (%d/%d étapes) - %s"), bSucces ? TEXT("RÉUSSI") : TEXT("ÉCHEC"),
		bSucces ? EtapesTestEscalade.Num() : FMath::Max(0, IndexTestEscalade - 1), EtapesTestEscalade.Num(), *Raison);
	if (bSucces)
	{
		UE_LOG(LogMansa, Log, TEXT("%s"), *Message);
	}
	else
	{
		UE_LOG(LogMansa, Error, TEXT("%s"), *Message);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, bSucces ? FColor::Green : FColor::Red, Message);
	}
	if (bQuitterApresTestEscalade)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ConsoleCommand(TEXT("quit"));
		}
	}
}

void AMansaCharacter::ChargerReglagesMouvement()
{
	const FMansaMovementRow* Ligne = nullptr;
	if (MovementTable)
	{
		Ligne = MovementTable->FindRow<FMansaMovementRow>(MovementRowName, TEXT("MansaMovement"), /*bWarnIfRowMissing*/ false);
	}
	if (Ligne)
	{
		MouvementCourant = *Ligne;
	}
	else
	{
		UE_LOG(LogMansa, Warning, TEXT("%s : ligne '%s' introuvable dans %s (DT_CharacterMovement), valeurs de repli utilisées."),
			*GetName(), *MovementRowName.ToString(), MovementTable ? *MovementTable->GetName() : TEXT("<aucune table>"));
		MouvementCourant = MouvementDeRepli;
	}

	UCharacterMovementComponent* CMC = GetCharacterMovement();
	CMC->AirControl = MouvementCourant.ControleEnLair;
	CMC->RotationRate = FRotator(0.f, MouvementCourant.VitesseRotation, 0.f);

	// Hauteur de saut -> vitesse initiale : h = v² / 2g  =>  v = sqrt(2 g h)
	const float Gravite = FMath::Abs(CMC->GetGravityZ());
	CMC->JumpZVelocity = FMath::Sqrt(2.f * Gravite * FMath::Max(0.f, MouvementCourant.HauteurSaut));

	MettreAJourAllure();

	if (Traversal)
	{
		Traversal->AppliquerReglages(MouvementCourant);
	}
	if (Climbing)
	{
		Climbing->AppliquerReglages(MouvementCourant);
	}

	UE_LOG(LogMansa, Log, TEXT("%s : déplacement chargé (marche %.0f, jogging %.0f, course %.0f, saut %.0f cm -> JumpZ %.0f)"),
		*GetName(), MouvementCourant.VitesseMarche, MouvementCourant.VitesseJogging, MouvementCourant.VitesseCourse,
		MouvementCourant.HauteurSaut, CMC->JumpZVelocity);
}

void AMansaCharacter::MettreAJourAllure()
{
	// Priorité : marche lente > course > jogging (marcher est un choix volontaire du joueur).
	const bool bCourseStick = MouvementCourant.bCourseStickAFond && MagnitudeStick >= MouvementCourant.SeuilStickCourse;
	EMansaAllure Nouvelle = EMansaAllure::Jogging;
	if (bMarcheMaintenue)
	{
		Nouvelle = EMansaAllure::Marche;
	}
	else if (bSprintMaintenu || bSprintBascule || bCourseStick)
	{
		Nouvelle = EMansaAllure::Course;
	}
	AllureCourante = Nouvelle;

	float Vitesse = MouvementCourant.VitesseJogging;
	switch (Nouvelle)
	{
	case EMansaAllure::Marche: Vitesse = MouvementCourant.VitesseMarche; break;
	case EMansaAllure::Course: Vitesse = MouvementCourant.VitesseCourse; break;
	default: break;
	}
	// À la manette, un stick partiel réduit déjà la vitesse (CMC analogique) : c'est la « marche » manette.
	GetCharacterMovement()->MaxWalkSpeed = Vitesse;
}

void AMansaCharacter::SetCameraProfile(FName NomProfil)
{
	if (CameraProfiles)
	{
		CameraProfiles->SetCameraProfile(NomProfil);
	}
}

void AMansaCharacter::MansaCamera(FName NomProfil)
{
	UE_LOG(LogMansa, Log, TEXT("Console : profil caméra %s"), *NomProfil.ToString());
	SetCameraProfile(NomProfil);
}

void AMansaCharacter::MansaContexte(FName NomContexte, bool bActif)
{
	if (NomContexte == TEXT("Combat"))
	{
		SetCombatContextActive(bActif);
	}
	else if (NomContexte == TEXT("Escalade"))
	{
		SetEscaladeContextActive(bActif);
	}
	else
	{
		UE_LOG(LogMansa, Warning, TEXT("Console : contexte inconnu '%s' (Combat ou Escalade)."), *NomContexte.ToString());
	}
}

void AMansaCharacter::MansaTestFranchissement()
{
	ObstaclesDeTest.Reset();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("MansaObstacleTest")))
		{
			ObstaclesDeTest.Add(*It);
		}
	}
	// Ordre stable (par nom) pour des logs comparables d'un lancement à l'autre.
	ObstaclesDeTest.Sort([](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B) { return A->GetName() < B->GetName(); });
	UE_LOG(LogMansa, Log, TEXT("Test franchissement : %d obstacle(s) tagué(s) MansaObstacleTest"), ObstaclesDeTest.Num());
	IndexTestFranchissement = 0;
	EtapeTestFranchissement();
}

void AMansaCharacter::EtapeTestFranchissement()
{
	// Bilan de l'obstacle précédent
	if (IndexTestFranchissement > 0)
	{
		const AActor* Precedent = ObstaclesDeTest[IndexTestFranchissement - 1].Get();
		const float Pieds = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		UE_LOG(LogMansa, Log, TEXT("Test franchissement : %s -> pieds à Z=%.0f, X=%.0f"), *GetNameSafe(Precedent), Pieds, GetActorLocation().X);
	}
	if (!ObstaclesDeTest.IsValidIndex(IndexTestFranchissement))
	{
		UE_LOG(LogMansa, Log, TEXT("Test franchissement : terminé"));
		return;
	}

	const AActor* Obstacle = ObstaclesDeTest[IndexTestFranchissement++].Get();
	if (Obstacle)
	{
		// Sira se place à 100 cm devant la face -X de l'obstacle, au sol, face à lui.
		FVector Origine, Etendue;
		Obstacle->GetActorBounds(true, Origine, Etendue);
		const float DemiHauteur = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector Depart(Origine.X - Etendue.X - GetCapsuleComponent()->GetScaledCapsuleRadius() - 30.f, Origine.Y, Origine.Z - Etendue.Z + DemiHauteur + 2.f);
		GetCharacterMovement()->StopMovementImmediately();
		TeleportTo(Depart, FRotator::ZeroRotator);
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		const bool bOk = Traversal && Traversal->TenterFranchissement();
		UE_LOG(LogMansa, Log, TEXT("Test franchissement : %s (haut %.0f cm) -> %s"), *Obstacle->GetName(), Etendue.Z * 2.f, bOk ? TEXT("franchissement lancé") : TEXT("aucun franchissement"));
	}
	GetWorldTimerManager().SetTimer(TimerTestFranchissement, this, &AMansaCharacter::EtapeTestFranchissement, 1.5f, false);
}

void AMansaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogMansa, Error, TEXT("'%s' : composant Enhanced Input introuvable (vérifie DefaultInput.ini)."), *GetNameSafe(this));
		return;
	}

	// Déplacement / caméra / saut
	BindIfValid(Input, JumpAction, ETriggerEvent::Started, &AMansaCharacter::OnJumpStarted);
	BindIfValid(Input, JumpAction, ETriggerEvent::Completed, &AMansaCharacter::OnJumpCompleted);
	BindIfValid(Input, MoveAction, ETriggerEvent::Triggered, &AMansaCharacter::Move);
	BindIfValid(Input, MoveAction, ETriggerEvent::Completed, &AMansaCharacter::OnMoveCompleted);
	BindIfValid(Input, LookAction, ETriggerEvent::Triggered, &AMansaCharacter::Look);
	BindIfValid(Input, MouseLookAction, ETriggerEvent::Triggered, &AMansaCharacter::Look);

	// Allures
	BindIfValid(Input, SprintAction, ETriggerEvent::Started, &AMansaCharacter::OnSprintStarted);
	BindIfValid(Input, SprintAction, ETriggerEvent::Completed, &AMansaCharacter::OnSprintCompleted);
	BindIfValid(Input, SprintToggleAction, ETriggerEvent::Started, &AMansaCharacter::OnSprintToggle);
	BindIfValid(Input, WalkAction, ETriggerEvent::Started, &AMansaCharacter::OnWalkStarted);
	BindIfValid(Input, WalkAction, ETriggerEvent::Completed, &AMansaCharacter::OnWalkCompleted);

	// Actions ponctuelles
	BindIfValid(Input, InteractAction, ETriggerEvent::Started, &AMansaCharacter::OnInteract);
	BindIfValid(Input, AttackLightAction, ETriggerEvent::Started, &AMansaCharacter::OnAttackLight);
	BindIfValid(Input, AttackHeavyAction, ETriggerEvent::Started, &AMansaCharacter::OnAttackHeavy);
	BindIfValid(Input, DodgeAction, ETriggerEvent::Started, &AMansaCharacter::OnDodge);
	BindIfValid(Input, LockOnAction, ETriggerEvent::Started, &AMansaCharacter::OnLockOn);
	BindIfValid(Input, DropHoldAction, ETriggerEvent::Started, &AMansaCharacter::OnDropHold);
	BindIfValid(Input, PauseAction, ETriggerEvent::Started, &AMansaCharacter::OnPause);
}

void AMansaCharacter::BindIfValid(UEnhancedInputComponent* Input, UInputAction* Action, ETriggerEvent Event, void (AMansaCharacter::*Func)(const FInputActionValue&))
{
	if (Input && Action)
	{
		Input->BindAction(Action, Event, this, Func);
	}
}

void AMansaCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Le contexte d'exploration est la base : on l'ajoute dès qu'un joueur local possède Sira.
	SetContextActive(ExplorationContext, ExplorationPriority, true);
}

void AMansaCharacter::SetContextActive(UInputMappingContext* Context, int32 Priority, bool bActive)
{
	if (!Context)
	{
		return;
	}
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (bActive)
		{
			if (!Subsystem->HasMappingContext(Context))
			{
				Subsystem->AddMappingContext(Context, Priority);
				UE_LOG(LogMansa, Log, TEXT("Contexte d'entrée ajouté : %s (priorité %d)"), *GetNameSafe(Context), Priority);
			}
		}
		else
		{
			Subsystem->RemoveMappingContext(Context);
			UE_LOG(LogMansa, Log, TEXT("Contexte d'entrée retiré : %s"), *GetNameSafe(Context));
		}
	}
}

void AMansaCharacter::SetEscaladeContextActive(bool bActive)
{
	SetContextActive(EscaladeContext, EscaladePriority, bActive);
	DebugInput(FString::Printf(TEXT("Contexte Escalade : %s"), bActive ? TEXT("actif") : TEXT("inactif")));
}

void AMansaCharacter::SetCombatContextActive(bool bActive)
{
	SetContextActive(CombatContext, CombatPriority, bActive);
	DebugInput(FString::Printf(TEXT("Contexte Combat : %s"), bActive ? TEXT("actif") : TEXT("inactif")));
}

void AMansaCharacter::DebugInput(const FString& Message) const
{
	if (!bAfficherDebugInput)
	{
		return;
	}
	UE_LOG(LogMansa, Log, TEXT("[Input] %s"), *Message);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DureeMessageDebug, FColor::Yellow, FString::Printf(TEXT("[Input] %s"), *Message));
	}
}

void AMansaCharacter::OnJumpStarted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Sauter / grimper"));
	DoJumpStart();
}

void AMansaCharacter::OnJumpCompleted(const FInputActionValue& Value)
{
	DoJumpEnd();
}

void AMansaCharacter::OnMoveCompleted(const FInputActionValue& Value)
{
	// Stick / touches relâchés : la course en bascule (L3) s'arrête, comme dans la plupart des jeux manette.
	MagnitudeStick = 0.f;
	bSprintBascule = false;
	if (Climbing)
	{
		Climbing->TraiterStick(FVector2D::ZeroVector, 0.f);
	}
	MettreAJourAllure();
}

void AMansaCharacter::OnSprintStarted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Courir (maintien) : début"));
	bSprintMaintenu = true;
	MettreAJourAllure();
}

void AMansaCharacter::OnSprintCompleted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Courir (maintien) : fin"));
	bSprintMaintenu = false;
	MettreAJourAllure();
}

void AMansaCharacter::OnSprintToggle(const FInputActionValue& Value)
{
	bSprintBascule = !bSprintBascule;
	DebugInput(FString::Printf(TEXT("Courir (bascule L3) : %s"), bSprintBascule ? TEXT("oui") : TEXT("non")));
	MettreAJourAllure();
}

void AMansaCharacter::OnWalkStarted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Marcher : début"));
	bMarcheMaintenue = true;
	MettreAJourAllure();
}

void AMansaCharacter::OnWalkCompleted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Marcher : fin"));
	bMarcheMaintenue = false;
	MettreAJourAllure();
}

void AMansaCharacter::OnInteract(const FInputActionValue& Value)
{
	DebugInput(TEXT("Interagir"));
}

void AMansaCharacter::OnAttackLight(const FInputActionValue& Value)
{
	DebugInput(TEXT("Attaque légère"));
}

void AMansaCharacter::OnAttackHeavy(const FInputActionValue& Value)
{
	DebugInput(TEXT("Attaque lourde"));
}

void AMansaCharacter::OnDodge(const FInputActionValue& Value)
{
	DebugInput(TEXT("Esquive"));
}

void AMansaCharacter::OnLockOn(const FInputActionValue& Value)
{
	DebugInput(TEXT("Verrouiller une cible"));
}

void AMansaCharacter::OnDropHold(const FInputActionValue& Value)
{
	DebugInput(TEXT("Lâcher la prise"));
	if (Climbing)
	{
		Climbing->Lacher();
	}
}

void AMansaCharacter::OnPause(const FInputActionValue& Value)
{
	// Le menu pause arrive en H1.
	DebugInput(TEXT("Pause"));
}

void AMansaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Course « stick à fond » (option de la DT) : on ne recalcule que si on franchit le seuil.
	if (MouvementCourant.bCourseStickAFond)
	{
		const float Seuil = MouvementCourant.SeuilStickCourse;
		const bool bAvant = MagnitudeStick >= Seuil;
		MagnitudeStick = MovementVector.Size();
		if (bAvant != (MagnitudeStick >= Seuil))
		{
			MettreAJourAllure();
		}
	}

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMansaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMansaCharacter::DoMove(float Right, float Forward)
{
	// En escalade, le stick vise les prises et glisse le long des corniches (repère du mur).
	if (Climbing)
	{
		Climbing->TraiterStick(FVector2D(Right, Forward), GetWorld()->GetDeltaSeconds());
		if (Climbing->EstEnEscalade())
		{
			return;
		}
	}
	// Pendant un enjambement / mantle, le déplacement est piloté par le composant.
	if (Traversal && Traversal->EstEnFranchissement())
	{
		return;
	}
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMansaCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMansaCharacter::DoJumpStart()
{
	// Saut = « sauter / grimper » (GDD 4.2).
	// 1. En escalade (suspendue, en saut, ou en l'air juste après une chute) : l'escalade décide.
	if (Climbing && (Climbing->EstEnEscalade() || GetCharacterMovement()->IsFalling()) && Climbing->GererSaut())
	{
		return;
	}
	if (Traversal && Traversal->EstEnFranchissement())
	{
		return;
	}
	// 2. Un obstacle franchissable devant (muret, rebord 90-180 cm) passe avant l'escalade.
	if (Traversal && Traversal->TenterFranchissement())
	{
		return;
	}
	// 3. Une prise à portée devant Sira : elle s'y accroche.
	if (Climbing && Climbing->TenterAccroche())
	{
		return;
	}
	Jump();
}

void AMansaCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
