#include "plugin.h"

SFSE_PLUGIN_PRELOAD(const SFSE::PreLoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	return true;
}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse, { .trampoline = true, .trampolineSize = 128 });

	RE::UI::GetSingleton();
	SFSE::GetMessagingInterface()->RegisterListener(OnMessage);

	return true;
}
