#include "BobbyRE.h"
#include "config.h"

bool jumpStarted = false;
config settings;

void manualLoadSystem(RE::TESObjectREFR* ship) 
{
	using func_getParentLocation_t = RE::BGSLocation* (RE::TESObjectREFR*);
	REL::Relocation<func_getParentLocation_t>getParentLocation{ REL::ID(63412) };

	using func_unkfunc_t = void* (RE::TESObjectREFR**, RE::BGSLocation*, RE::BGSLocation*);
	REL::Relocation<func_unkfunc_t>unkfunc{ REL::ID(64046) };

	using func_unkfunc2_t = void* ();
	REL::Relocation<func_unkfunc2_t>unkfunc2{ REL::ID(62173) };

	using func_unkfunc3_t = void* (void*, RE::TESObjectREFR*, RE::BGSLocation*, RE::BGSLocation*, bool);
	REL::Relocation<func_unkfunc3_t>unkfunc3{ REL::ID(72938) };

	using func_unkfunc4_t = void* (void*, uint32_t, void*);
	REL::Relocation<func_unkfunc4_t>unkfunc4{ REL::ID(72962) };

	using func_loadSystem_t = int(RE::TESObjectREFR*, RE::TESObjectCELL*, bool, double);
	REL::Relocation<func_loadSystem_t>loadSystem{ REL::ID(102641) };

	RE::BGSLocation* prevLocation = getParentLocation(ship);

	loadSystem(ship, ship->parentCell, 0, 0);

	RE::BGSLocation* newLocation = getParentLocation(ship);
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

	using func_updateDiscoveredStatus_t = void*(RE::Actor *, RE::BGSLocation *);
	REL::Relocation<func_updateDiscoveredStatus_t>updatePlanetDiscoveryStatus{ REL::ID(102968) };
	updatePlanetDiscoveryStatus(BobbyRE::Spaceship::GetPilot(ship), newLocation);

	REL::Relocation<uintptr_t*>unkGlobal{ REL::ID(938414) };

	using func_updateStarDiscoveryStatus_t = void* (void*, uint32_t, bool);
	REL::Relocation<func_updateStarDiscoveryStatus_t>updateStarDiscoveryStatus{ REL::ID(102650) };

	BobbyRE::starInfo* currentStarInfo = *(BobbyRE::starInfo**)(*unkGlobal.get() + 0x58);

	updateStarDiscoveryStatus(nullptr, currentStarInfo->FormID, 1);
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

		if (pilot->formID == RE::PlayerCharacter::GetSingleton()->formID 
			|| ship->HasKeyword((RE::BGSKeyword*)RE::TESForm::LookupByID(0x101da7))) //jade swan keyword
		{
			REX::INFO("Grav jump event");
			REX::INFO("State: {}", event.aeState);
			REX::INFO("Jump destination: {}", location);

			jumpStarted = true;

			if (event.aeState == 2) {
				manualLoadSystem(ship);
				REX::INFO("Manual jump success");
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}
};

namespace hooks 
{
	using func_playerMoveTo_t = void(RE::PlayerCharacter*, void *, RE::TESObjectCELL*, RE::TESWorldSpace*, float*, void*);
	func_playerMoveTo_t *original_playerMoveTo;

	void hook_playerMoveTo(RE::PlayerCharacter* player, void *a2, RE::TESObjectCELL* cell, RE::TESWorldSpace* worldspace, float* a5, void* a6) 
	{
		if (jumpStarted) 
		{
			RE::TESObjectREFR* ship = RE::PlayerCharacter::GetSingleton()->GetSpaceship();
			manualLoadSystem(ship);
			jumpStarted = false;
		}
		else 
		{
			original_playerMoveTo(player, a2, cell, worldspace, a5, a6);
		}
	}

	void install() 
	{
		//moveTo papyrus call
		uintptr_t addr = REL::Relocation<uintptr_t>( REL::ID(118183)).address();
		uintptr_t MoveTo = addr + 0xbb6;

		REL::Trampoline &tramp = REL::GetTrampoline();
		tramp.create(64);

		//GRAV LANES SUPPORT
		original_playerMoveTo = (func_playerMoveTo_t *)tramp.write_call<5>(MoveTo, hook_playerMoveTo);
	}
}

void OnMessage(SFSE::MessagingInterface::Message* message) 
{
	if (message->type == SFSE::MessagingInterface::kPostDataLoad) 
	{
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


		if (settings.GravLanesSupport) 
		{
			hooks::install();
		}	
	}
}