#pragma once

namespace BobbyRE 
{
	namespace Spaceship 
	{
		struct GravJumpEvent : RE::Spaceship::GravJumpEvent 
		{
			RE::NiPointer<RE::TESObjectREFR> source; //source ship
			int aeState; // State { Initiated = 0, AnimStarted = 1, Completed = 2, Failed = 3 }
			RE::NiPointer<RE::BGSLocation> Location; 
		};

		RE::Actor* GetPilot(RE::TESObjectREFR* ship) 
		{
			using func_t = decltype(&GetPilot);
			static REL::Relocation<func_t> func{ REL::ID(119876) };
			return func(ship);
		}
	}

	struct PositionPlayerEvent 
	{
		uint32_t unk;
		bool unk4;
		bool unk5;
	};

	struct MovePlayerRelevantSpaceComponentsOnViewChange 
	{
		RE::NiAVObject** node;
		const char* fullName;
		uint32_t sourceID;
		uint32_t destPlanetID;
		int param;
	};

	struct starInfo 
	{
		void *qword0;
		void *qword8;
		uint32_t dword10;
		uint32_t field_14;
		volatile signed __int64 refcount;
		void *qword20;
		void *qword28;
		void *qword30;
		void *BSService__Jobsite;
		void *qword40;
		void *qword48;
		uint32_t FormID;
	};
}