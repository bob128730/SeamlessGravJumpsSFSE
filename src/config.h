#include "INIReader.h"

const char* defaultConfigFile =
"[Config]\n\
DisableJumpCam = true\n\n\
[Compatibility]\n\
GravLanesSupport = false\n\0";

struct config 
{
	void load() 
	{
		FILE* file;
		const char* path = "Data\\SFSE\\plugins\\SGJ_Config.ini";

   		file = fopen(path, "r");

		if (!file)
		{
			REX::INFO("No config file, creating with defaults");
			file = fopen(path, "w+");
			fprintf(file, "%s", defaultConfigFile);
			fclose(file);

			file = fopen(path, "r");
		}
		if (file)
		{
			INIReader reader(file);
			if (reader.ParseError() != 0)
			{
				REX::WARN("Failed to read config file, using default");
				return;
			}
			this->DisableJumpCam = reader.GetBoolean("Config", "DisableJumpCam", true);
			this->GravLanesSupport = reader.GetBoolean("Compatibility", "GravLanesSupport", false);

			fclose(file);
		}

		REX::INFO("Config:");
		REX::INFO("DisableJumpCam: {}", this->DisableJumpCam);
		REX::INFO("GravLanesSupport: {}", this->GravLanesSupport);
		REX::INFO("-----------------------------------------------------------------");
	}

	bool DisableJumpCam = 1;
	bool GravLanesSupport = 0;
};