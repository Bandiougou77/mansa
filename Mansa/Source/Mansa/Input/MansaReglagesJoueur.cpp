// MANSA — Réglages du joueur liés aux contrôles et au HUD (A6).

#include "MansaReglagesJoueur.h"
#include "Mansa.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UMansaReglagesJoueur* UMansaReglagesJoueur::Get()
{
	return GetMutableDefault<UMansaReglagesJoueur>();
}

EMansaDispositionClavier UMansaReglagesJoueur::GetDispositionEffective() const
{
	return DispositionClavier == EMansaDispositionClavier::Auto ? DetecterDispositionSysteme() : DispositionClavier;
}

EMansaDispositionClavier UMansaReglagesJoueur::DetecterDispositionSysteme()
{
#if PLATFORM_WINDOWS
	// On regarde la lettre produite par la touche PHYSIQUE à droite de Tab (code de balayage 0x10) :
	// « A » sur un clavier AZERTY (France, Belgique), « Q » sur QWERTY et QWERTZ.
	const HKL Disposition = ::GetKeyboardLayout(0);
	const UINT Vk = ::MapVirtualKeyExW(0x10, MAPVK_VSC_TO_VK, Disposition);
	return Vk == 'A' ? EMansaDispositionClavier::AZERTY : EMansaDispositionClavier::QWERTY;
#else
	return EMansaDispositionClavier::QWERTY;
#endif
}

void UMansaReglagesJoueur::Sauvegarder()
{
	SaveConfig();
	UE_LOG(LogMansa, Log, TEXT("Réglages joueur sauvegardés (%s)"), *GetClass()->GetConfigName());
}
