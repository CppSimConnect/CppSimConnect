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


namespace CppSimConnect {

	struct ExceptionHandler {
		DWORD sendID;
		std::function<void(std::string const& msg, unsigned parmIndex)> handler;
	};


	class SimState {
		std::mutex _simConnectMutex;		// SimConnect is not thread-safe

		HANDLE _handle{ nullptr };

		unsigned _maxSenders{ 16 };
		std::deque<ExceptionHandler> _onExcept;
		std::mutex _onExceptMutex;
		void addExceptionHandler(DWORD sendID, std::function<void(std::string const& msg, unsigned parmIndex)> handler);
		void onExcept(DWORD sendID, std::string const& msg, unsigned parmIndex);

		static void cppSimConnect_MSFS_handleMessage(SIMCONNECT_RECV* msgPtr, DWORD msgLen, void* context) noexcept;

		friend class SimConnect;
	};
};