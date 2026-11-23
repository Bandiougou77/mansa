// MANSA — Acteur prise d'escalade (parent de BP_PriseEscalade).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MansaPriseEscalade.generated.h"

class UStaticMeshComponent;
class UArrowComponent;
class UMansaPriseComponent;

/**
 * Prise d'escalade posée à la main ou par le générateur de greybox.
 * Convention : la RACINE ne porte que le Yaw (sa flèche avant = normale du mur, vers l'extérieur) ;
 * le mesh enfant porte la forme (rotation / échelle libres, ex. cylindre couché pour un toron).
 * Tout est mobile pour que la BancoFragile puisse trembler.
 */
UCLASS(Blueprintable)
class AMansaPriseEscalade : public AActor
{
	GENERATED_BODY()

public:
	AMansaPriseEscalade();

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	UMansaPriseComponent* GetPrise() const { return Prise; }

	UFUNCTION(BlueprintPure, Category="Mansa|Escalade")
	UStaticMeshComponent* GetMesh() const { return Mesh; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> Racine;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UMansaPriseComponent> Prise;

#if WITH_EDITORONLY_DATA
	/** Normale de la prise, visible dans l'éditeur seulement. */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UArrowComponent> FlecheNormale;
#endif
};
