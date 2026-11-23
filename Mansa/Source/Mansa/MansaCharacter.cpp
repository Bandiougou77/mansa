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
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
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
}

void AMansaCharacter::OnSprintStarted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Courir (maintien) : début"));
}

void AMansaCharacter::OnSprintCompleted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Courir (maintien) : fin"));
}

void AMansaCharacter::OnSprintToggle(const FInputActionValue& Value)
{
	DebugInput(TEXT("Courir (bascule L3)"));
}

void AMansaCharacter::OnWalkStarted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Marcher : début"));
}

void AMansaCharacter::OnWalkCompleted(const FInputActionValue& Value)
{
	DebugInput(TEXT("Marcher : fin"));
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
	// signal the character to jump
	Jump();
}

void AMansaCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
