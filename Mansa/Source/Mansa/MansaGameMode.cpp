// Copyright Epic Games, Inc. All Rights Reserved.

#include "MansaGameMode.h"
#include "MansaHUD.h"

AMansaGameMode::AMansaGameMode()
{
	// HUD de la slice (prompts contextuels) ; BP_MansaGameMode en hérite.
	HUDClass = AMansaHUD::StaticClass();
}
