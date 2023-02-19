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

#include <string>

#include "../exceptions/NotConnected.h"

#include "../sim/SimState.h"

#include "../reactive/MessageResult.h"

using CppSimConnect::SimConnect;
using CppSimConnect::Reactive::MessageResult;


MessageResult<std::string> SimConnect::simRequestSystemStateString(const std::string& stateName) {
	MessageResult<std::string> result;

	if (connected()) {
		auto reqId = nextReqId();
		_logger.debug("Requesting string value for '{}' with RequestID {}", stateName, reqId);

		RecvObserver obs(_state->simRequestSimState(reqId, stateName));
		result.withOnComplete([reqId,this]() { _state->deRegisterRequestResultObserver(reqId); });
		obs.subscribe([result](SIMCONNECT_RECV* msg) {
			std::string value(static_cast<SIMCONNECT_RECV_SYSTEM_STATE*>(msg)->szString);
			result.onNext(value);
		});
	}
	else {
		result.onError(std::make_exception_ptr(NotConnected()));
	}
	return result;
}

MessageResult<bool> SimConnect::simRequestSystemStateBool(const std::string& stateName) {
	MessageResult<bool> result;

	if (connected()) {
		auto reqId = nextReqId();
		_logger.debug("Requesting boolean value for '{}' with RequestID {}", stateName, reqId);

		RecvObserver obs(_state->simRequestSimState(reqId, stateName));
		result.withOnComplete([reqId, this]() { _state->deRegisterRequestResultObserver(reqId); });
		obs.subscribe([result](SIMCONNECT_RECV* msg) {
			bool value{ (static_cast<SIMCONNECT_RECV_SYSTEM_STATE*>(msg)->dwInteger) != 0 };
			result.onNext(value);
		});
	}
	else {
		result.onError(std::make_exception_ptr(NotConnected()));
	}
	return result;
}