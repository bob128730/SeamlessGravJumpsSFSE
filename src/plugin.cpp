#include "BobbyRE.h"
#include "config.h"

bool jumpStarted = false;
bool jumpComplete = false;
config settings;
RE::BGSLocation* prevLocation;
RE::BGSLocation* newLocation;

void updateDiscoveryInfo(RE::TESObjectREFR* ship) 
{

	using func_updateDiscoveredStatus_t = void* (RE::Actor*, RE::BGSLocation*);
	REL::Relocation<func_updateDiscoveredStatus_t>updatePlanetDiscoveryStatus{ REL::ID(102968) };
	updatePlanetDiscoveryStatus(BobbyRE::Spaceship::GetPilot(ship), newLocation);

	REL::Relocation<uintptr_t*>unkGlobal{ REL::ID(938414) };

	using func_updateStarDiscoveryStatus_t = void* (void*, uint32_t, bool);
	REL::Relocation<func_updateStarDiscoveryStatus_t>updateStarDiscoveryStatus{ REL::ID(102650) };

	BobbyRE::starInfo* currentStarInfo = *(BobbyRE::starInfo**)(*unkGlobal.get() + 0x58);

	updateStarDiscoveryStatus(nullptr, currentStarInfo->FormID, 1);
}

void manualLoadSystem(RE::TESObjectREFR* ship) 
{
	using func_getParentLocation_t = RE::BGSLocation* (RE::TESObjectREFR*);
	REL::Relocation<func_getParentLocation_t>getParentLocation{ REL::ID(63412) };

	using func_loadSystem_t = int(RE::TESObjectREFR*, RE::TESObjectCELL*, bool, double);
	REL::Relocation<func_loadSystem_t>loadSystem{ REL::ID(102641) };

	prevLocation = getParentLocation(ship);

	loadSystem(ship, ship->parentCell, 0, 0);

	newLocation = getParentLocation(ship);

	using func_unkfunc_t = void* (RE::TESObjectREFR**, RE::BGSLocation*, RE::BGSLocation*);
	REL::Relocation<func_unkfunc_t>unkfunc{ REL::ID(64046) };

	using func_unkfunc2_t = void* ();
	REL::Relocation<func_unkfunc2_t>unkfunc2{ REL::ID(62173) };

	using func_unkfunc3_t = void* (void*, RE::TESObjectREFR*, RE::BGSLocation*, RE::BGSLocation*, bool);
	REL::Relocation<func_unkfunc3_t>unkfunc3{ REL::ID(72938) };

	using func_unkfunc4_t = void* (void*, uint32_t, void*);
	REL::Relocation<func_unkfunc4_t>unkfunc4{ REL::ID(72962) };

	if (prevLocation != newLocation)
	{
		_InterlockedExchangeAdd64((volatile long long*)&ship->refCount, 0x80000200001);
		unkfunc(&ship, prevLocation, newLocation);
		ship->DecRefCount();
		void* v206 = unkfunc2();
		int v456[8];
		void* v207 = unkfunc3(v456, ship, prevLocation, newLocation, 0);

		unkfunc4(v206, 4, v207);
	}
}

class GravJumpEventSink : public RE::BSTEventSink<BobbyRE::Spaceship::GravJumpEvent> 
{
	RE::BSEventNotifyControl ProcessEvent(const BobbyRE::Spaceship::GravJumpEvent& event, RE::BSTEventSource<BobbyRE::Spaceship::GravJumpEvent>* a_source)
	{
		RE::TESObjectREFR* ship = event.source->AsReference();

		RE::Actor* pilot = BobbyRE::Spaceship::GetPilot(ship);

		const char* location = "";

		if (event.Location)
		{
			location = event.Location->formEditorID.c_str();
		}

		if (pilot->formID == RE::PlayerCharacter::GetSingleton()->formID)
		{
			REX::INFO("Grav jump event");
			REX::INFO("State: {}", event.aeState);
			REX::INFO("Jump destination: {}", location);

			jumpStarted = true;

			if (event.aeState == 2)
			{
				jumpComplete = true;
			}
		}
		else if (ship->HasKeyword((RE::BGSKeyword*)RE::TESForm::LookupByID(0x101da7))) //jade swan keyword
		{
			if (event.aeState == 2) 
			{
				manualLoadSystem(ship);
				updateDiscoveryInfo(ship);
				jumpStarted = false;
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}
};

namespace hooks 
{
	using func_playerMoveTo_t = void(RE::PlayerCharacter*, void *, RE::TESObjectCELL*, RE::TESWorldSpace*, float*, void*);
	func_playerMoveTo_t *original_playerMoveTo;

	using func_shipHudHide_t = void();
	func_shipHudHide_t* original_shipHudHide;

	using func_playerShipUpdate_t = void(RE::TESObjectREFR*, float);
	func_playerShipUpdate_t* original_playerShipUpdate;

	using func_registerForDistanceLessThanEvent_t = void*(void *, void *, void *, void *, bool, float, uint32_t);
	func_registerForDistanceLessThanEvent_t* original_registerForDistanceLessThanEvent;

	void hook_playerMoveTo(RE::PlayerCharacter* player, void *a2, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, float* a5, void* a6) 
	{
		if (jumpStarted) 
		{
			REX::INFO("Astrogate / Grav lanes grav jump called");
			jumpComplete = true;
			jumpStarted = false;
		}
		else 
		{
			REX::INFO("Non astrogate / grav lanes MoveTo called");
			original_playerMoveTo(player, a2, cell, worldspace, a5, a6);
		}
	}

	// For some reason when the mod is active grav lanes & astrogate can't show the ship hud after jumping. I have no idea why
	// So i'll just prevent them from hiding it for now.
	void hook_shipHudHide() 
	{
		if (jumpStarted)
			return;

		return original_shipHudHide();
	}

	//loading too fast seems to cause issues
	float timer = 0.15;
	void hook_playerShipUpdate(RE::TESObjectREFR* ship, float dt) 
	{
		if (jumpComplete) {
			timer -= dt;
			if (timer <= 0)
			{
				jumpComplete = false;
				timer = 0.15;
				RE::TESObjectREFR* ship = RE::PlayerCharacter::GetSingleton()->GetSpaceship();
				manualLoadSystem(ship);

				using func_attatchObjectToCell_t = double(RE::TESObjectCELL*, RE::TESObjectREFR*, bool, bool);
				REL::Relocation<func_attatchObjectToCell_t>attatchObjectToCell{ REL::ID(63034) };
				RE::TESObjectCELL* GalaxyCell = (RE::TESObjectCELL*)RE::TESObjectCELL::LookupByID(0x18343);

				attatchObjectToCell(GalaxyCell, ship, 0, 0);

				updateDiscoveryInfo(ship);

				REX::INFO("Manual jump success");
			}
		}
		original_playerShipUpdate(ship, dt);
	}

	// Astrogate calls this when using the arrive at star option, so we know to re enable the moveTo hook
	void* hook_registerForDistanceLessThanEvent(void* a1, void* a2, void* a3, void* a4, bool a5, float distance, uint32_t a7) 
	{
		if (distance == 375000000.0)
		{
			REX::INFO("Astrogate arrive at star option detected");
			jumpStarted = true;
		}
		return original_registerForDistanceLessThanEvent(a1, a2, a3, a4, a5, distance, a7);
	}

	void install() 
	{
		uintptr_t addr = REL::Relocation<uintptr_t>( REL::ID(118183)).address();
		uintptr_t addr2 = REL::Relocation<uintptr_t>(REL::ID(117400)).address();
		uintptr_t addr3 = REL::Relocation<uintptr_t>(REL::ID(97772)).address();
		uintptr_t addr4 = REL::Relocation<uintptr_t>(REL::ID(117757)).address();

		uintptr_t MoveToCall = addr + 0xbb6;
		uintptr_t shipHudHideCall = addr2 + 0x56;
		uintptr_t playerShipUpdateCall = addr3 + 0x68;
		uintptr_t registerForDistanceLessThanEventCall = addr4 + 0x1bd;

		REL::Trampoline &tramp = REL::GetTrampoline();
		tramp.create(64);

		original_playerShipUpdate = (func_playerShipUpdate_t*)tramp.write_call<5>(playerShipUpdateCall, hook_playerShipUpdate);

		if (settings.GravLanesSupport) {
			original_playerMoveTo = (func_playerMoveTo_t*)tramp.write_call<5>(MoveToCall, hook_playerMoveTo);
			original_shipHudHide = (func_shipHudHide_t*)tramp.write_call<5>(shipHudHideCall, hook_shipHudHide);
			original_registerForDistanceLessThanEvent = (func_registerForDistanceLessThanEvent_t*)tramp.write_call<5>(registerForDistanceLessThanEventCall, hook_registerForDistanceLessThanEvent);
		}
	}
}

void OnMessage(SFSE::MessagingInterface::Message* message) 
{
	if (message->type == SFSE::MessagingInterface::kPostDataLoad) 
	{
		REX::INFO("Init");
		settings.load();

		GravJumpEventSink* GravJumpSink = new GravJumpEventSink();
		using GetFn = RE::BSTGlobalEvent::EventSource<BobbyRE::Spaceship::GravJumpEvent>* (*)();
		auto GravJumpEvent_GetSource = REL::Relocation<GetFn>(REL::ID(93876));
		auto source = GravJumpEvent_GetSource();
		source->RegisterSink(GravJumpSink);

		//stop normal loading
		uintptr_t initiateGravJumpCompleted = REL::Relocation<uintptr_t>(REL::ID(119833)).address();
		void* addCellToLoaderCall = (void *)(initiateGravJumpCompleted + 0x262);

		int8_t nopcall[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

		DWORD OldProtect;
		VirtualProtect(addCellToLoaderCall, 5, PAGE_EXECUTE_READWRITE, &OldProtect);
		memcpy_s(addCellToLoaderCall, 5, nopcall, 5);
		VirtualProtect(addCellToLoaderCall, 5, OldProtect, &OldProtect);

		if (settings.DisableJumpCam) 
		{
			uintptr_t initiateGravJumpSequence = REL::Relocation<uintptr_t>(REL::ID(119839)).address();
			void* shouldStartGravCam = (void*)(initiateGravJumpSequence + 0x4e4);
			byte jmp = 0xEB;

			VirtualProtect(shouldStartGravCam, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
			memcpy_s(shouldStartGravCam, 1, &jmp, 1);
			VirtualProtect(shouldStartGravCam, 5, OldProtect, &OldProtect);
		}

		hooks::install();
	}
}