// MANSA — Profils de caméra (D1, GDD §4.5).

#include "MansaCameraProfileComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Mansa.h"

UMansaCameraProfileComponent::UMansaCameraProfileComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Le tick ne sert qu'aux transitions : il est activé à la demande.
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMansaCameraProfileComponent::SetCibles(USpringArmComponent* InBras, UCameraComponent* InCamera)
{
	Bras = InBras;
	Camera = InCamera;
}

void UMansaCameraProfileComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		if (!Bras) { Bras = Owner->FindComponentByClass<USpringArmComponent>(); }
		if (!Camera) { Camera = Owner->FindComponentByClass<UCameraComponent>(); }
	}
	if (!Bras || !Camera)
	{
		UE_LOG(LogMansa, Warning, TEXT("%s : bras de caméra ou caméra introuvable, profils inactifs."), *GetNameSafe(GetOwner()));
		return;
	}

	SetCameraProfile(ProfilParDefaut, /*bInstantane*/ true);
}

FMansaCameraProfileRow UMansaCameraProfileComponent::TrouverProfil(FName NomProfil) const
{
	if (!ProfileTable)
	{
		UE_LOG(LogMansa, Warning, TEXT("%s : ProfileTable (DT_CameraProfiles) non assignée, profil de repli utilisé."), *GetNameSafe(GetOwner()));
		return ProfilDeRepli;
	}
	if (const FMansaCameraProfileRow* Ligne = ProfileTable->FindRow<FMansaCameraProfileRow>(NomProfil, TEXT("MansaCameraProfile"), /*bWarnIfRowMissing*/ false))
	{
		return *Ligne;
	}
	UE_LOG(LogMansa, Warning, TEXT("%s : profil caméra '%s' absent de %s, profil de repli utilisé."), *GetNameSafe(GetOwner()), *NomProfil.ToString(), *GetNameSafe(ProfileTable));
	return ProfilDeRepli;
}

void UMansaCameraProfileComponent::SetCameraProfile(FName NomProfil, bool bInstantane)
{
	if (!Bras || !Camera)
	{
		return;
	}

	EtatCible = TrouverProfil(NomProfil);
	ProfilCourant = NomProfil;
	OnCameraProfileChanged.Broadcast(NomProfil);
	UE_LOG(LogMansa, Log, TEXT("Caméra : vers le profil '%s' (bras %.0f, FOV %.0f, %.2f s)"), *NomProfil.ToString(), EtatCible.LongueurBras, EtatCible.FOV, bInstantane ? 0.f : EtatCible.DureeTransition);

	if (bInstantane || EtatCible.DureeTransition <= KINDA_SMALL_NUMBER)
	{
		Appliquer(EtatCible);
		SetComponentTickEnabled(false);
		return;
	}

	// La transition part de l'état courant, même si une autre transition était en cours.
	EtatDepart = EtatCourant;
	DureeTransition = EtatCible.DureeTransition;
	TempsEcoule = 0.f;
	SetComponentTickEnabled(true);
}

void UMansaCameraProfileComponent::RechargerProfils()
{
	SetCameraProfile(ProfilCourant.IsNone() ? ProfilParDefaut : ProfilCourant, true);
}

void UMansaCameraProfileComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TempsEcoule += DeltaTime;
	const float Alpha = FMath::Clamp(TempsEcoule / DureeTransition, 0.f, 1.f);
	// Courbe douce (ease in/out) pour éviter l'à-coup au départ et à l'arrivée.
	const float A = FMath::SmoothStep(0.f, 1.f, Alpha);

	FMansaCameraProfileRow Etat = EtatCible;
	Etat.LongueurBras = FMath::Lerp(EtatDepart.LongueurBras, EtatCible.LongueurBras, A);
	Etat.FOV = FMath::Lerp(EtatDepart.FOV, EtatCible.FOV, A);
	Etat.DecalageCamera = FMath::Lerp(EtatDepart.DecalageCamera, EtatCible.DecalageCamera, A);
	Appliquer(Etat);

	if (Alpha >= 1.f)
	{
		SetComponentTickEnabled(false);
		UE_LOG(LogMansa, Log, TEXT("Caméra : profil '%s' atteint (bras %.0f, FOV %.0f)"), *ProfilCourant.ToString(), Etat.LongueurBras, Etat.FOV);
	}
}

void UMansaCameraProfileComponent::Appliquer(const FMansaCameraProfileRow& Etat)
{
	EtatCourant = Etat;
	Bras->TargetArmLength = Etat.LongueurBras;
	Bras->SocketOffset = Etat.DecalageCamera;
	Camera->SetFieldOfView(Etat.FOV);
}
