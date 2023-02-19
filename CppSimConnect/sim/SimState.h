/*
 * Copyright (c) 2022. Bert Laverman
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

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#pragma warning(disable:4245)
#include <windows.h>
#include <SimConnect.h>
#pragma warning(default:4245)

#include <deque>

#include "../Logger.h"

#include "../exceptions/SimException.h"
#include "../reactive/StreamResult.h"


namespace CppSimConnect {

	struct ExceptionInfo {
		DWORD exceptionId;
		DWORD parmIndex;
	};

	using ExceptionCallback = std::function<void(unsigned exceptionId, std::string const& msg, unsigned parmIndex)>;
	struct ExceptionHandler {
		DWORD sendID;
		ExceptionCallback handler;
	};

	using RecvObserver = Reactive::StreamResult<SIMCONNECT_RECV*>;

	class SimState {
		Logger _logger;
		std::mutex _simConnectMutex;		// SimConnect is not thread-safe

		HANDLE _handle{ nullptr };

		unsigned _maxSenders{ 16 };
		std::deque<ExceptionHandler> _onExcept;
		std::map<DWORD, ExceptionInfo> _earlyErrors;
		std::mutex _onExceptMutex;

		std::map<DWORD, RecvObserver> _messageObservers;
		std::mutex _observerMutex;

		inline DWORD sendIdOf(std::function<HRESULT()> call) {
			std::lock_guard<std::mutex> lock(_simConnectMutex);

			DWORD sendId{ 0 };
			HRESULT hr = call();

			if (SUCCEEDED(hr)) {
				if (FAILED(SimConnect_GetLastSentPacketID(_handle, &sendId))) {
					_logger.error("Failed to retrieve SendID for call.");
				}
			}
			return SUCCEEDED(hr) ? sendId : hr;
		}

	public:
		SimState(const Logger& logger) : _logger(logger) {}
		~SimState() = default;

		inline HANDLE handle() const noexcept { return _handle; }

		void addExceptionHandler(DWORD sendID, ExceptionCallback handler);
		void onExcept(DWORD sendID, unsigned exceptionId, unsigned parmIndex);

		inline RecvObserver registerRequestResultObserver(DWORD reqId) {
			std::lock_guard<std::mutex> lock(_observerMutex);

			auto result = _messageObservers[reqId];
			_logger.debug("Register result handler for RequestID {} (now {} registration(s)).", reqId, _messageObservers.size());
			return result;
		}
		inline void deRegisterRequestResultObserver(DWORD reqId) {
			std::lock_guard<std::mutex> lock(_observerMutex);

			auto it = _messageObservers.find(reqId);
			if (it != _messageObservers.end()) {
				_messageObservers.erase(it);
				_logger.debug("Deregistered result handler for RequestID {} (now {} registration(s)).", reqId, _messageObservers.size());
			}
		}

		static void cppSimConnect_handleMessage(SIMCONNECT_RECV* msgPtr, DWORD msgLen, void* context) noexcept;

		inline RecvObserver simRequestSimState(DWORD reqId, const std::string& stateName) {
			RecvObserver obs = registerRequestResultObserver(reqId);
			addExceptionHandler(
				sendIdOf([handle = handle(), reqId, stateName]() { return SimConnect_RequestSystemState(handle, reqId, stateName.c_str()); }),
				[&obs](unsigned exceptionId, std::string const& msg, unsigned parmIndex) { obs.onError(std::make_exception_ptr(SimException(exceptionId, msg, parmIndex))); });

			return obs;
		}

		void dispatchRequestData(DWORD reqId, SIMCONNECT_RECV* msg) {
			RecvObserver obs;
			{
				std::lock_guard<std::mutex> lock(_observerMutex);

				auto obsIt = _messageObservers.find(reqId);
				if (obsIt != _messageObservers.end()) {
					obs = obsIt->second;
				}
				else {
					_logger.warn("Received data for unknown request id {}.", reqId);
				}
			}
			obs.onNext(msg);
		}

		friend class SimConnect;
	};
};