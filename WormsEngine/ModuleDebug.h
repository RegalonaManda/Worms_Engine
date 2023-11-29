#pragma once
#include "Module.h"
#include "Globals.h"

enum class DeltaTimeScheme
{
	FIXED,
	VARIABLE,
	SEMIFIXED,
	UNKNOWN
};

class ModuleDebug : public Module
{
public:
	ModuleDebug(bool start_enabled = true);
	~ModuleDebug();

	bool Start();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

public:
	bool gravityEnabled;
	bool aerodynamicDragEnabled;
	bool hydrodynamicDragEnabled;
	bool hydrodynamicBuoyancyEnabled;
	bool pause;
	bool debug;
	bool debugSpeed;
	double FPS;

	DeltaTimeScheme deltaTime;


	char radiusText[10] = { "\0" };
	char angleText[11] = { "\0" };

};
