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

using CppSimConnect::LogLevel;
using CppSimConnect::SimConnect;
using CppSimConnect::SimState;


constexpr int cppSimConnect_NumExceptions = SIMCONNECT_EXCEPTION_OBJECT_SCHEDULE + 1;
static const std::array<std::string, cppSimConnect_NumExceptions> cppSimConnect_Exceptions = {
    "None",                                 // Unused
    "Error",
    "Size mismatch",
    "Unrecognized ID",
    "Unopened",                             // Unused
    "SimConnect version mismatch",
    "Too many groups",
    "Unknown event name",
    "Too many event names",
    "Duplicate event ID",
    "Too many maps",
    "Too many objects",
    "Too many request IDs",
    "Weather: Invalid port",                // Deprecated
    "Weather: Invalid METAR",               // Deprecated
    "Weather: Unable to get observation",   // Deprecated
    "Weather: Unable to create station",    // Deprecated
    "Weather: Unable to remove station",    // Deprecated
    "Invalid data type",
    "Invalid data size",
    "Data error",
    "Invalid array",
    "Create object failed",
    "Load flightplan failed",
    "Invalid operation for object type",
    "AI: Illegal operation",
    "Already subscribed",
    "Invalid enum",                         // Unknown datatype
    "Data definition error",                // Requesting variable string data
    "Duplicate ID",
    "Unknown datum ID",
    "Out of bounds",
    "Client data area already created",
    "AI: Outside of reality bubble",
    "AI: Object container error",
    "AI: Creation failed",
    "AI: ATC error",
    "AI: Scheduling error"
};

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


void SimState::cppSimConnect_MSFS_handleMessage(SIMCONNECT_RECV* msgPtr, DWORD msgLen, void* context) noexcept
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
        sim._state->onExcept(msg.dwSendID, cppSimConnect_Exceptions[msg.dwException], msg.dwIndex);
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

    }
}

void SimState::addExceptionHandler(DWORD sendID, std::function<void(std::string const& msg, unsigned parmIndex)> handler)
{
    std::lock_guard<std::mutex> lock(_onExceptMutex);
    while (_onExcept.size() >= _maxSenders) {
        _onExcept.pop_front();
    }
    _onExcept.push_back({sendID, handler});
}

void SimState::onExcept(DWORD sendID, std::string const& msg, unsigned parmIndex)
{
    std::function<void(std::string const& msg, unsigned parmIndex)> handler;
    {
        std::lock_guard<std::mutex> lock(_onExceptMutex);

        for (const auto& exceptionHandler : _onExcept) {
            if (sendID == exceptionHandler.sendID) {
                handler = exceptionHandler.handler;
                break;
            }
        }
    }
    if (handler) {
        handler(msg, parmIndex);
    }
}