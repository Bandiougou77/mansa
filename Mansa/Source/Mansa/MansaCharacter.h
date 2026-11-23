// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MansaDataTypes.h"
#include "MansaClimbingComponent.h"
#include "MansaInputTypes.h"
#include "MansaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
class UDataTable;
class UMansaCameraProfileComponent;
class UMansaTraversalComponent;
class UMansaParcoursComponent;
class UMansaPriseComponent;
struct FInputActionValue;
enum class ETriggerEvent : uint8;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/** Allure de déplacement au sol (GDD §4.3). */
UENUM(BlueprintType)
enum class EMansaAllure : uint8
{
	Marche,
	Jogging,
	Course
};

/**
 *  Personnage jouable de MANSA (Sira).
 *  - Caméra orbitale (bras + caméra).
 *  - Enhanced Input : trois contextes (Exploration, Escalade, Combat), cf. GDD §4.2.
 *    IMC_Exploration est ajouté à la prise de contrôle ; Escalade et Combat
 *    s'empilent par-dessus avec une priorité plus haute (ils « mangent » les touches
 *    partagées : X/Carré = Interagir en exploration, Attaque légère en combat).
 *
 *  Les noms JumpAction / MoveAction / LookAction / MouseLookAction sont conservés
 *  depuis le template pour que BP_ThirdPersonCharacter (map du template) marche encore.
 */
UCLASS(abstract)
class AMansaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Profils de caméra (DT_CameraProfiles) et transitions. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMansaCameraProfileComponent> CameraProfiles;

	/** Enjambement et mantle (D2). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMansaTraversalComponent> Traversal;

	/** Escalade à prises (D4-D5). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMansaClimbingComponent> Climbing;

	/** Checkpoints et rattrapage (D6). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMansaParcoursComponent> Parcours;

protected:

	// ---------------------------------------------------------------------
	// Déplacement (D1, GDD §4.3) : valeurs lues dans DT_CharacterMovement
	// ---------------------------------------------------------------------

	/** Table des réglages de déplacement (struct FMansaMovementRow). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement")
	TObjectPtr<UDataTable> MovementTable;

	/** Ligne de MovementTable utilisée par ce personnage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement")
	FName MovementRowName = TEXT("Defaut");

	/** Valeurs de repli si la table ou la ligne manque (un warning est logué). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Deplacement")
	FMansaMovementRow MouvementDeRepli;

	/** Réglages effectivement appliqués (lecture seule, pour le debug). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Mansa|Deplacement")
	FMansaMovementRow MouvementCourant;

	/** Allure actuelle. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Mansa|Deplacement")
	EMansaAllure AllureCourante = EMansaAllure::Jogging;

	// ---------------------------------------------------------------------
	// Contextes d'entrée (GDD §4.2)
	// ---------------------------------------------------------------------

	/** Contexte de base, toujours actif : déplacement, caméra, saut, course, interaction, pause. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	TObjectPtr<UInputMappingContext> ExplorationContext;

	/** Contexte ajouté pendant l'escalade (lâcher une prise). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	TObjectPtr<UInputMappingContext> EscaladeContext;

	/** Touches qui dépendent de la disposition (ZQSD + A = parer) : actif si clavier AZERTY. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	TObjectPtr<UInputMappingContext> ClavierAzertyContext;

	/** Touches qui dépendent de la disposition (WASD + Q = parer) : actif si clavier QWERTY. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	TObjectPtr<UInputMappingContext> ClavierQwertyContext;

	/** Contexte ajouté pendant un combat (attaques, esquive, verrouillage). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	TObjectPtr<UInputMappingContext> CombatContext;

	/** Priorité d'IMC_Exploration (la plus basse). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	int32 ExplorationPriority = 0;

	/** Priorité d'IMC_Escalade (au-dessus de l'exploration). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	int32 EscaladePriority = 1;

	/** Priorité d'IMC_Combat (au-dessus de l'exploration). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Contextes")
	int32 CombatPriority = 2;

	// ---------------------------------------------------------------------
	// Actions (noms du template conservés pour les 4 premières)
	// ---------------------------------------------------------------------

	/** Sauter / grimper (Espace, A/Croix) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Se déplacer (ZQSD/WASD, stick gauche) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Caméra (souris, stick droit) */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Caméra souris séparée (template). Dans MANSA, IA_Look gère souris ET stick : laisser vide. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Courir (maintien Maj gauche) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> SprintAction;

	/** Courir, bascule manette (clic L3) : reste en course jusqu'à ce que le stick soit relâché */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> SprintToggleAction;

	/** Marcher lentement (maintien Alt gauche ; à la manette : stick partiel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> WalkAction;

	/** Interagir / parler (E, X/Carré hors combat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> InteractAction;

	/** Attaque légère (clic gauche, X/Carré en combat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> AttackLightAction;

	/** Attaque lourde (clic droit, Y/Triangle) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> AttackHeavyAction;

	/** Esquive (Ctrl gauche / Maj en combat, B/Rond en combat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> DodgeAction;

	/** Verrouiller une cible (clic molette, R3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> LockOnAction;

	/** Lâcher une prise (C, B/Rond en escalade) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> DropHoldAction;

	/** Pause (Échap, Start/Options) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> PauseAction;

	/** S'accroupir (C, bascule) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> CrouchAction;

	/** Parer / bloquer (A en AZERTY, Q en QWERTY, RB/R1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> ParryAction;

	/** Sens du chasseur (V, R3) : révèle prises, traces, ennemis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> HunterSenseAction;

	/** Carte (M, Vue/pavé tactile) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> MapAction;

	/** Journal (J) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> JournalAction;

	/** Inventaire (I) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Input|Actions")
	TObjectPtr<UInputAction> InventoryAction;

	/** Affiche un message à l'écran pour chaque action (aide au test A4). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Debug")
	bool bAfficherDebugInput = true;

	/** Durée d'affichage des messages de debug d'entrée (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Debug", meta=(ClampMin="0.1"))
	float DureeMessageDebug = 2.f;

public:

	/** Constructor */
	AMansaCharacter();

	/** Relit la ligne de DT_CharacterMovement et l'applique au Character Movement Component. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Deplacement")
	void ChargerReglagesMouvement();

	/** Réglages de déplacement en vigueur (lus dans la DT). */
	UFUNCTION(BlueprintPure, Category="Mansa|Deplacement")
	const FMansaMovementRow& GetReglagesMouvement() const { return MouvementCourant; }

	UFUNCTION(BlueprintPure, Category="Mansa|Deplacement")
	EMansaAllure GetAllure() const { return AllureCourante; }

	/** Passe à un profil caméra de DT_CameraProfiles (Exploration, Escalade, Combat), transition interpolée. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Camera")
	void SetCameraProfile(FName NomProfil);

	/** Commande console de test : « MansaCamera Combat » (profils Exploration / Escalade / Combat). */
	UFUNCTION(Exec)
	void MansaCamera(FName NomProfil);

	/** Commande console de test : « MansaContexte Combat 1 » / « MansaContexte Escalade 0 ». */
	UFUNCTION(Exec)
	void MansaContexte(FName NomContexte, bool bActif);

	/**
	 * Commande console de test D2 : « MansaTestFranchissement ». Place Sira devant chaque acteur
	 * tagué MansaObstacleTest (1,5 s d'intervalle), tente le franchissement et logue le résultat.
	 */
	UFUNCTION(Exec)
	void MansaTestFranchissement();

	/** Commande console de test D3 : « MansaDebugPrises 1 » affiche type et identifiant de chaque prise. */
	UFUNCTION(Exec)
	void MansaDebugPrises(bool bActif);

	/** Commande console de test D6 : « MansaCheckpoint CP2 » téléporte au checkpoint (CP1, CP2, CP_Combat). */
	UFUNCTION(Exec)
	void MansaCheckpoint(FName IdCheckpoint);

	/** Commande console de test D6 : « MansaRattrapage » déclenche le fondu et le retour au checkpoint. */
	UFUNCTION(Exec)
	void MansaRattrapage();

	/**
	 * Commande console de test D7 : « MansaTestEscalade » (option A) ou « MansaTestEscalade B ».
	 * Part de CP1 et enchaîne accroches / sauts / mantles jusqu'à la terrasse (une étape par seconde),
	 * en loguant chaque étape. Les approches au sol sont des téléportations (le déplacement est testé en D1).
	 * « MansaTestEscalade A 1 » quitte le jeu à la fin (test sans surveillance).
	 */
	UFUNCTION(Exec)
	void MansaTestEscalade(FName Option = TEXT("A"), bool bQuitterALaFin = false);

	/** Active / désactive le contexte d'escalade (appelé par le futur composant d'escalade). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Input")
	void SetEscaladeContextActive(bool bActive);

	/** Active / désactive le contexte de combat (appelé au début / à la fin d'un combat). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Input")
	void SetCombatContextActive(bool bActive);

	/** Active le contexte clavier AZERTY ou QWERTY selon les réglages du joueur (Auto = détection). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Input")
	void AppliquerDispositionClavier();

	/** Prompts utiles maintenant (« Grimper », « Lâcher »...), lus par le HUD. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Input")
	void GetPromptsContextuels(TArray<FMansaPrompt>& OutPrompts) const;

	/** Prompts actuels en texte (« [Espace] Grimper »), pour les logs et les tests. */
	FString DecrirePrompts(EMansaPeripherique Peripherique) const;

	/** Toutes les actions assignées (debug, futur menu de remappage). */
	TArray<const UInputAction*> GetToutesLesActions() const;

	/** Sens du chasseur actif (V / R3). */
	UFUNCTION(BlueprintPure, Category="Mansa|Input")
	bool EstSensDuChasseurActif() const { return bSensDuChasseur; }

protected:

	virtual void BeginPlay() override;

	/** Recalcule l'allure (marche / jogging / course) et la vitesse max. */
	void MettreAJourAllure();

	// État des entrées d'allure
	bool bSprintMaintenu = false;
	bool bSprintBascule = false;
	bool bMarcheMaintenue = false;
	float MagnitudeStick = 0.f;

	// Test automatique des franchissements (commande console)
	void EtapeTestFranchissement();
	TArray<TWeakObjectPtr<AActor>> ObstaclesDeTest;
	int32 IndexTestFranchissement = 0;
	FTimerHandle TimerTestFranchissement;

	// Test automatique de l'escalade (commande console)
	void EtapeTestEscalade();
	void GlissementTestEscalade();
	TWeakObjectPtr<UMansaPriseComponent> CibleGlissement;
	float TempsGlissement = 0.f;
	void FinTestEscalade(bool bSucces, const FString& Raison);
	TArray<FString> EtapesTestEscalade;
	int32 IndexTestEscalade = 0;
	bool bQuitterApresTestEscalade = false;
	FTimerHandle TimerTestEscalade;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Ajoute IMC_Exploration quand un PlayerController local prend le contrôle. */
	virtual void NotifyControllerChanged() override;

	/** Ajoute ou retire un contexte sur le sous-système Enhanced Input du joueur local. */
	void SetContextActive(UInputMappingContext* Context, int32 Priority, bool bActive);

	/** Lie une action seulement si elle est assignée (évite les liaisons nulles). */
	void BindIfValid(UEnhancedInputComponent* Input, UInputAction* Action, ETriggerEvent Event, void (AMansaCharacter::*Func)(const FInputActionValue&));

	/** Affiche + logue un message de debug d'entrée. */
	void DebugInput(const FString& Message) const;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// Gestionnaires des actions (A4 : simples messages tant que le gameplay n'existe pas)
	void OnJumpStarted(const FInputActionValue& Value);
	void OnJumpCompleted(const FInputActionValue& Value);
	void OnMoveCompleted(const FInputActionValue& Value);
	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);
	void OnSprintToggle(const FInputActionValue& Value);
	void OnWalkStarted(const FInputActionValue& Value);
	void OnWalkCompleted(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnAttackLight(const FInputActionValue& Value);
	void OnAttackHeavy(const FInputActionValue& Value);
	void OnDodge(const FInputActionValue& Value);
	void OnLockOn(const FInputActionValue& Value);
	void OnDropHold(const FInputActionValue& Value);
	void OnPause(const FInputActionValue& Value);
	void OnCrouchToggle(const FInputActionValue& Value);
	void OnParry(const FInputActionValue& Value);
	void OnHunterSense(const FInputActionValue& Value);
	void OnMap(const FInputActionValue& Value);
	void OnJournal(const FInputActionValue& Value);
	void OnInventory(const FInputActionValue& Value);

	/** Déclare les contextes de Sira aux réglages Enhanced Input du joueur (remappage, bloc H). */
	void EnregistrerContextesRemappables();

	bool bSensDuChasseur = false;

	/** Escalade : bascule du contexte d'entrée et du profil caméra. */
	UFUNCTION()
	void OnEtatEscaladeChange(EMansaEtatEscalade Ancien, EMansaEtatEscalade Nouveau);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns CameraProfiles subobject **/
	FORCEINLINE UMansaCameraProfileComponent* GetCameraProfiles() const { return CameraProfiles; }

	/** Returns Traversal subobject **/
	FORCEINLINE UMansaTraversalComponent* GetTraversal() const { return Traversal; }

	/** Returns Climbing subobject **/
	FORCEINLINE UMansaClimbingComponent* GetClimbing() const { return Climbing; }

	/** Returns Parcours subobject **/
	FORCEINLINE UMansaParcoursComponent* GetParcours() const { return Parcours; }
};
