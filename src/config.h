#include "INIReader.h"

struct config 
{
	void load() 
	{
		INIReader reader("Data\\SFSE\\plugins\\SGJ_Config.ini");
		if (reader.ParseError() != 0) 
		{
			REX::WARN("Failed to read config file, using default");
			return;
		}
		this->DisableJumpCam = reader.GetBoolean("Config", "DisableJumpCam", true);
		this->GravLanesSupport = reader.GetBoolean("Compatibility", "GravLanesSupport", false);
	}

	bool DisableJumpCam = 1;
	bool GravLanesSupport = 0;
};