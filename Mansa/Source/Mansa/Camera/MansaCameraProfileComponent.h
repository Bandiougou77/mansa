// MANSA — Profils de caméra (D1, GDD §4.5).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MansaDataTypes.h"
#include "MansaCameraProfileComponent.generated.h"

class UDataTable;
class USpringArmComponent;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMansaOnCameraProfileChanged, FName, NouveauProfil);

/**
 * Pilote le bras de caméra et la caméra du propriétaire d'après DT_CameraProfiles.
 * Les transitions entre profils sont interpolées (durée lue dans la ligne cible, 0,4 s par défaut).
 * Le Tick n'est actif QUE pendant une transition.
 */
UCLASS(ClassGroup=(Mansa), meta=(BlueprintSpawnableComponent))
class UMansaCameraProfileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMansaCameraProfileComponent();

	/** Table des profils (struct FMansaCameraProfileRow). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Camera")
	TObjectPtr<UDataTable> ProfileTable;

	/** Profil appliqué au démarrage (sans transition). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Camera")
	FName ProfilParDefaut = TEXT("Exploration");

	/** Valeurs de repli si la table ou la ligne manque (un warning est logué). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mansa|Camera")
	FMansaCameraProfileRow ProfilDeRepli;

	/** Passe au profil indiqué (nom de ligne de DT_CameraProfiles) avec transition interpolée. */
	UFUNCTION(BlueprintCallable, Category="Mansa|Camera")
	void SetCameraProfile(FName NomProfil, bool bInstantane = false);

	/** Relit la table et réapplique le profil courant (utile après une réimport en jeu). */
	UFUNCTION(BlueprintCallable, Category="Mansa|Camera")
	void RechargerProfils();

	UFUNCTION(BlueprintPure, Category="Mansa|Camera")
	FName GetProfilCourant() const { return ProfilCourant; }

	/** Diffusé quand une transition vers un nouveau profil commence. */
	UPROPERTY(BlueprintAssignable, Category="Mansa|Camera")
	FMansaOnCameraProfileChanged OnCameraProfileChanged;

	/** Le bras et la caméra pilotés (trouvés automatiquement sur le propriétaire si vides). */
	void SetCibles(USpringArmComponent* InBras, UCameraComponent* InCamera);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Cherche la ligne ; renvoie le repli (et logue) si absente. */
	FMansaCameraProfileRow TrouverProfil(FName NomProfil) const;

	/** Applique un état (bras, FOV, décalage) aux composants. */
	void Appliquer(const FMansaCameraProfileRow& Etat);

	/** État actuellement appliqué (pour démarrer une transition depuis n'importe où). */
	FMansaCameraProfileRow EtatCourant;
	FMansaCameraProfileRow EtatDepart;
	FMansaCameraProfileRow EtatCible;
	float DureeTransition = 0.f;
	float TempsEcoule = 0.f;
	FName ProfilCourant;

	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> Bras;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> Camera;
};
