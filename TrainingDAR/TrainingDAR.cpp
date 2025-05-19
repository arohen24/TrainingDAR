#include "pch.h"
#include "TrainingDAR.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h" // Include the settings window header
#include "bakkesmod/plugin/bakkesmodplugin.h" // Include this if not already in pch.h

#include "IMGUI/imgui.h" 
#include "IMGUI/imgui_internal.h" // Might be needed for some advanced ImGui features

#define DAR_OFF 0
#define DAR_LEFT 1
#define DAR_RIGHT -1

BAKKESMOD_PLUGIN(TrainingDAR, "Training DAR", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

std::string TrainingDAR::GetPluginName()
{
	return "Training DAR Settings"; // This is the title that will appear in the BakkesMod menu
}

void TrainingDAR::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

void TrainingDAR::RenderSettings()
{
	// Example: Get the current value of your cvar_mode CVar
	// Use the global cvarManager or the one passed to onLoad/onUnload
	CVarWrapper cvar_mode_wrapper = cvarManager->getCvar("cdar_mode");
	int current_dar_mode = cvar_mode_wrapper.getIntValue(); // Get the current integer value

	CVarWrapper cvar_sensitivity_wrapper = cvarManager->getCvar("cdar_sensitivity");
	float current_sensitivity = cvar_sensitivity_wrapper.getFloatValue(); // Get the current float value

	// Use ImGui functions to create UI elements

	// Example: A simple text label
	ImGui::TextUnformatted("Select Conditional DAR Mode:");

	// Example: Radio buttons for the DAR mode
	// ImGui::RadioButton takes label, value, and a pointer to the variable to modify
	if (ImGui::RadioButton("Off", &current_dar_mode, DAR_OFF)) {
		// If the radio button is clicked, update the CVar
		cvar_mode_wrapper.setValue(current_dar_mode);
	}
	ImGui::SameLine(); // Place the next element on the same line
	if (ImGui::RadioButton("Left", &current_dar_mode, DAR_LEFT)) {
		cvar_mode_wrapper.setValue(current_dar_mode);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Right", &current_dar_mode, DAR_RIGHT)) {
		cvar_mode_wrapper.setValue(current_dar_mode);
	}

	// Example: You could add more settings here
	// ImGui::Checkbox("Enable Feature X", &your_bool_cvar);
	// ImGui::SliderFloat("Sensitivity", &your_float_cvar, 0.1f, 1.0f);
	// if (ImGui::Button("Reset Settings")) { /* ... */ }

	ImGui::Spacing();

	ImGui::TextUnformatted("DAR Sensitivity Threshold:");
	// ImGui::SliderFloat takes label, pointer to float variable, min value, max value
	if (ImGui::SliderFloat("##SensitivitySlider", &current_sensitivity, 0.0f, 1.0f, "%.2f")) { // Using ## hides the label, %.2f formats the value
		// If the slider value changes, update the CVar
		cvar_sensitivity_wrapper.setValue(current_sensitivity);
	}

	// Note: You don't need to call ImGui::Begin() or ImGui::End() here.
	// BakkesMod handles the main window for you.
}

void TrainingDAR::onLoad()
{
	_globalCvarManager = cvarManager;

	LOG("[onLoad] Hello");

	cvarManager->registerCvar("cdar_mode", "0", "[TrainingDAR] mode of conditional DAR", true, true, -1, true, 1);
	cvarManager->registerCvar("cdar_sensitivity", "0.5", "[TrainingDAR] Sensitivity threshold for conditional DAR", true, true, 0.0f, true, 1.0f);

	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput",
		bind(&TrainingDAR::conditionalDAR, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
	);
}

void TrainingDAR::onUnload()
{
	LOG("[onUnload] Bye");
	gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
}

void TrainingDAR::conditionalDAR(CarWrapper cw, void* params, std::string funcName)
{
	if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining()) { return; }

	ControllerInput* ci = (ControllerInput*)params;

	CVarWrapper cvar_mode_wrapper = cvarManager->getCvar("cdar_mode");
	int current_dar_mode = cvar_mode_wrapper.getIntValue();

	CVarWrapper cvar_sensitivity_wrapper = cvarManager->getCvar("cdar_sensitivity");
	float sensitivity_threshold = cvar_sensitivity_wrapper.getFloatValue();

	float target_roll = 0.0f;
	bool mod_roll_needed = abs(ci->Pitch) != 0 || abs(ci->Yaw) != 0;
	if (mod_roll_needed) {
		float pitch_yaw_magnitude = std::sqrt(ci->Pitch * ci->Pitch + ci->Yaw * ci->Yaw);
		if (pitch_yaw_magnitude >= sensitivity_threshold) {
			target_roll = 1.0f;
		} else {
			target_roll = pitch_yaw_magnitude / sensitivity_threshold;
		}
	}

	switch (current_dar_mode) {
	case DAR_LEFT:
		if (mod_roll_needed) {
			LOG("[conditionalDAR] DAR_LEFT Pitch: {}, Yaw: {}", ci->Pitch, ci->Yaw);
			ci->Roll = -target_roll;
		}
		break;
	case DAR_RIGHT:
		if (mod_roll_needed) {
			LOG("[conditionalDAR] DAR_RIGHT Pitch: {}, Yaw: {}", ci->Pitch, ci->Yaw);
			ci->Roll = target_roll;
		}
		break;
	case DAR_OFF:
	default:
		break;
	}
}
