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

#include "../pch.h"

#include "../reactive/MessageResult.h"


using CppSimConnect::SimConnect;
using CppSimConnect::Reactive::MessageResult;


static std::array<std::string, 5> const SystemStateNames {
	"AircraftLoaded",
	"DialogMode",
	"FlightLoaded",
	"FlightPlan",
	"Sim",
};

inline static consteval const std::string& SystemStateName(CppSimConnect::SystemState s) {
	return SystemStateNames[static_cast<size_t>(s)];
}

std::string SimConnect::systemStateName(SystemState state) const noexcept {
	return SystemStateNames[static_cast<size_t>(state)];
}

MessageResult<std::string> SimConnect::requestAircraftLoaded() {
	return simRequestSystemStateString(SystemStateName(SystemState::AircraftLoaded));
}

MessageResult<bool> SimConnect::requestSimInDialogMode() {
	return simRequestSystemStateBool(SystemStateName(SystemState::DialogMode));
}

MessageResult<std::string> SimConnect::requestFlightLoaded() {
	return simRequestSystemStateString(SystemStateName(SystemState::FlightLoaded));
}

MessageResult<std::string> SimConnect::requestFlightPlan() {
	return simRequestSystemStateString(SystemStateName(SystemState::FlightPlan));
}

MessageResult<bool> SimConnect::requestUserFlying() {
	return simRequestSystemStateBool(SystemStateName(SystemState::Sim));
}
