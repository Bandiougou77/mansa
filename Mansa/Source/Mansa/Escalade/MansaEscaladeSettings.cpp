// MANSA — Réglages projet de l'escalade et du parcours.

#include "MansaEscaladeSettings.h"
#include "Engine/DataTable.h"

#define LOCTEXT_NAMESPACE "MansaEscalade"

UMansaEscaladeSettings::UMansaEscaladeSettings()
{
	// Valeurs par défaut : écrasées par Config/DefaultGame.ini si présent.
	TablePrisesEscalade = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Mansa/Data/DT_PrisesEscalade.DT_PrisesEscalade")));
	OrdreCheckpoints = { TEXT("CP1"), TEXT("CP2"), TEXT("CP_Combat") };
	RepliqueRattrapage = LOCTEXT("RepliqueRattrapage", "« Non, non... Sira n'était pas un oiseau. Reprenons. »");
}

#undef LOCTEXT_NAMESPACE
