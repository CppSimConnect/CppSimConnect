/*
 * Copyright (c) 2021, 2022. Bert Laverman
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

#include "pch.h"

#include <array>
#include <string>

#include "sim/SimState.h"


using CppSimConnect::LogLevel;
using CppSimConnect::SimConnect;
using CppSimConnect::SimState;


static void copyAppInfo(CppSimConnect::messages::AppInfo& info, SIMCONNECT_RECV_OPEN const& msg) {
    info.appName = msg.szApplicationName;

    info.appVersionMajor = std::to_string(msg.dwApplicationVersionMajor);
    info.appVersionMinor = std::to_string(msg.dwApplicationVersionMinor);
    info.appBuildMajor = std::to_string(msg.dwApplicationBuildMajor);
    info.appBuildMinor = std::to_string(msg.dwApplicationBuildMinor);
    info.scVersionMajor = std::to_string(msg.dwSimConnectVersionMajor);
    info.scVersionMinor = std::to_string(msg.dwSimConnectVersionMinor);
    info.scBuildMajor = std::to_string(msg.dwSimConnectBuildMajor);
    info.scBuildMinor = std::to_string(msg.dwSimConnectBuildMinor);
}


void SimState::cppSimConnect_handleMessage(SIMCONNECT_RECV* msgPtr, DWORD msgLen, void* context) noexcept
{
    if ((msgPtr == nullptr) || (msgLen < sizeof(SIMCONNECT_RECV)) || (msgPtr->dwID == SIMCONNECT_RECV_ID_NULL)) {
        return;
    }
    CppSimConnect::SimConnect& sim{ *static_cast<SimConnect*>(context) };
    if (!sim._state) {
        sim._logger.error("Received message from simulator but we have no valid connection.");
        return;
    }

    switch (msgPtr->dwID) {

    case SIMCONNECT_RECV_ID_EXCEPTION:
    {
        SIMCONNECT_RECV_EXCEPTION& msg{ *static_cast<SIMCONNECT_RECV_EXCEPTION*>(msgPtr) };
        sim._state->onExcept(msg.dwSendID, msg.dwException, msg.dwIndex);
    }
    break;

    case SIMCONNECT_RECV_ID_OPEN:
    {
        auto msg = *static_cast<SIMCONNECT_RECV_OPEN*>(msgPtr);
        copyAppInfo(sim._appInfo, msg);
        sim._logger.info("Connected to '{}'", sim._appInfo.appName);
        sim.notifyOpen();
    }
    break;

    case SIMCONNECT_RECV_ID_QUIT:
        sim.notifyClose();
        break;

    case SIMCONNECT_RECV_ID_SYSTEM_STATE:
    {
        SIMCONNECT_RECV_SYSTEM_STATE& msg{ *static_cast<SIMCONNECT_RECV_SYSTEM_STATE*>(msgPtr) };
        sim._logger.debug("System state received: {} ({})", msg.dwRequestID, msg.dwInteger);
        sim._state->dispatchRequestData(msg.dwRequestID, msgPtr);
    }
    break;

    default:
        sim._logger.warn("Unknown message type {}", msgPtr->dwID);
    }
}
