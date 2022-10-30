#include "pch.h"
#include "TrainingDAR.h"

#define DAR_OFF 0
#define DAR_LEFT 1
#define DAR_RIGHT 2

BAKKESMOD_PLUGIN(TrainingDAR, "Training DAR", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
int cdar_mode = DAR_LEFT;

void TrainingDAR::onLoad()
{
	_globalCvarManager = cvarManager;
	//cvarManager->log("Plugin loaded!");
	LOG("[onLoad] Hello");

	cvarManager->registerCvar("cdar_mode", "0", "[TrainingDAR] mode of conditional DAR", true, true, -1, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			cdar_mode = cvar.getIntValue();
		});

	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput",
		bind(&TrainingDAR::conditionalDAR, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
	);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	cvarManager->log("the cvar with name: " + cvarName + " changed");
	//	cvarManager->log("the new value is:" + newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&TrainingDAR::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&TrainingDAR::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	cvarManager->log("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&TrainingDAR::YourPluginMethod, this);
}

void TrainingDAR::onUnload()
{
	LOG("[onUnload] Bye");
}

void TrainingDAR::conditionalDAR(CarWrapper cw, void* params, std::string funcName)
{
	LOG("[conditionalDAR] start");

	if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining()) { return; }

	ControllerInput* ci = (ControllerInput*)params;
	switch (cdar_mode) {
	case DAR_LEFT:
		if (abs(ci->Pitch) != 0 or abs(ci->Yaw) != 0) {
			ci->Roll = -1.0f;
		}
		break;
	case DAR_RIGHT:
		if (abs(ci->Pitch) != 0 or abs(ci->Yaw) != 0) {
			ci->Roll = 1.0f;
		}
		break;
	}

	LOG("[conditionalDAR] end");
}
