// MANSA — Acteur prise d'escalade.

#include "MansaPriseEscalade.h"
#include "MansaPriseComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

AMansaPriseEscalade::AMansaPriseEscalade()
{
	PrimaryActorTick.bCanEverTick = false;

	Racine = CreateDefaultSubobject<USceneComponent>(TEXT("Racine"));
	Racine->SetMobility(EComponentMobility::Movable);
	RootComponent = Racine;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Racine);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	Prise = CreateDefaultSubobject<UMansaPriseComponent>(TEXT("Prise"));

#if WITH_EDITORONLY_DATA
	FlecheNormale = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("FlecheNormale"));
	if (FlecheNormale)
	{
		FlecheNormale->SetupAttachment(Racine);
		FlecheNormale->ArrowSize = 0.5f;
		FlecheNormale->SetArrowColor(FLinearColor(1.f, 0.6f, 0.1f));
	}
#endif
}
