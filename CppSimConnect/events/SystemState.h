/*
 * Copyright (c) 2023. Bert Laverman
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <array>
#include <string>

namespace CppSimConnect {

	enum class SystemState {
		AircraftLoaded,						// Currently loaded aircraft
		DialogMode,							// Does the user have a dialog open?
		FlightLoaded,						// Currently loaded flight
		FlightPlan,							// Currently loaded flightplan
		Sim,								// Is the simulator running?
	};

	enum class SystemEvent {
		AircraftLoaded,						// User changed the aircraft
		Crashed,							// The user's aircraft crashed
		CrashReset,							// The crash cut-scene has finished
		CustomMissionActionExecuted,		// Deprecated
		DialogMode,							// SystemState
		FlightLoaded,						// A flight has started or finished (default flight was loaded)
		FlightSaved,						// The current flight was correctly saved
		FlightPlan,							// SystemState
		FlightPlanActivated,				// A flightplan has been activated
		FlightPlanDeactivated,				// A flightplan has been de-activated
		Heartbeat_1sec,
		Heartbeat_4sec,
		Heartbeat_6Hz,
		Heartbeat_Frame,
		Heartbeat_PauseFrame,
		ObjectAdded,						// An AI object was added
		ObjectRemoved,						// An AI object was removed
		Pause,								// The flight was paused or unpaused
		Pause_EX1,							// A more detailed pause event
		Paused,								// The flight was paused
		PositionChanged,					// The user's aircraft changed position through a dialog
		Sim,								// The simulator's state changed
		SimStart,							// The simulator started running
		SimStop,							// The simulator stopped running
		Sound,								// Sound was switched on or off
		Unpaused,							// The flight was unpaused
		View,								// The user's aircraft view was changed
		WeatherModeChanged,					// Deprecated
	};

	inline std::array<std::string, 28> const SystemEventNames{
		"AircraftLoaded",					// User changed the aircraft
		"Crashed",							// The user's aircraft crashed
		"CrashReset",						// The crash cut-scene has finished
		"CustomMissionActionExecuted",		// Deprecated
		"DialogMode",						// SystemState
		"FlightLoaded",						// A flight has started or finished (default flight was loaded)
		"FlightSaved",						// The current flight was correctly saved
		"FlightPlan",						// SystemState
		"FlightPlanActivated",				// A flightplan has been activated
		"FlightPlanDeactivated",			// A flightplan has been de-activated
		"Heartbeat_1sec",
		"Heartbeat_4sec",
		"Heartbeat_6Hz",
		"Heartbeat_Frame",
		"Heartbeat_PauseFrame",
		"ObjectAdded",						// An AI object was added
		"ObjectRemoved",					// An AI object was removed
		"Pause",							// The flight was paused or unpaused
		"Pause_EX1",						// A more detailed pause event
		"Paused",							// The flight was paused
		"PositionChanged",					// The user's aircraft changed position through a dialog
		"Sim",								// The simulator's state changed
		"SimStart",							// The simulator started running
		"SimStop",							// The simulator stopped running
		"Sound",							// Sound was switched on or off
		"Unpaused",							// The flight was unpaused
		"View",								// The user's aircraft view was changed
		"WeatherModeChanged",				// Deprecated
	};
}