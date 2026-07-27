#pragma once

namespace BobbyRE 
{
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