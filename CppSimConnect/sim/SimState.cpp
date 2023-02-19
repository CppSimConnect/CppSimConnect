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

#include "../pch.h"

#include "SimState.h"

using CppSimConnect::LogLevel;
using CppSimConnect::SimConnect;
using CppSimConnect::SimState;


bool SimConnect::simConnect(bool byAutoConnect)
{
    if (_state) {
        _logger.warn("Forcing SimConnect_Close() to clean up old handle.");
        simDisconnect();
    }
    HANDLE h;
    HRESULT result = SimConnect_Open(&h, _clientName.c_str(), nullptr, 0, nullptr, 0);
    if (SUCCEEDED(result)) {
        _state = std::make_unique<SimState>(_logger);
        _state->_handle = h;
    }
    else if (!byAutoConnect) {
        long long bigInt = static_cast<unsigned long>(result);
        _logger.error("Connection to simulator failed. ({:x})", bigInt);
    }
    return SUCCEEDED(result);
}

bool SimConnect::simDisconnect() noexcept
{
    if (!_state) {
        return false; // Already disconnected
    }
    if (_state->_handle == nullptr) {
        _logger.warn("Not connected, but cleaning up state.");
        _state.reset(nullptr);

        return false; // Already disconnected.. assuming we just forgot to clean up?
    }
    HANDLE h{ _state->_handle };
    _state->_handle = nullptr;

    HRESULT result = SimConnect_Close(h);
    if (FAILED(result)) {
        _logger.error("Failed to disconnect from simulator.");
    }
    return SUCCEEDED(result);
}

void SimConnect::simDispatch() noexcept
{
    HRESULT dpResult = SimConnect_CallDispatch(_state->_handle, &SimState::cppSimConnect_handleMessage, this);
    if (FAILED(dpResult)) {
        _logger.error("Failed to start message dispatcher. (0x{:08x})", dpResult);
    }
}

void SimConnect::simDrainDispatchQueue() noexcept
{
    SIMCONNECT_RECV* msgPtr;
    DWORD msgLen;
    while (SUCCEEDED(SimConnect_GetNextDispatch(_state->_handle, &msgPtr, &msgLen))) {
        if ((msgPtr == nullptr) || (msgLen < sizeof(SIMCONNECT_RECV)) || (msgPtr->dwID == SIMCONNECT_RECV_ID_NULL)) {
            break;
        }
        SimState::cppSimConnect_handleMessage(msgPtr, msgLen, this);
    }
}



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

void SimState::addExceptionHandler(DWORD sendID, ExceptionCallback handler) {
    ExceptionInfo earlyError{ SIMCONNECT_EXCEPTION_NONE, 0 };
    bool haveError{ false };
    {
        std::lock_guard<std::mutex> lock(_onExceptMutex);

        auto it = _earlyErrors.find(sendID);
        if (it != _earlyErrors.end()) {
            earlyError = it->second;
            haveError = true;
            _earlyErrors.erase(it);
        }
        else {
            while (_onExcept.size() > _maxSenders) _onExcept.pop_front();
            _onExcept.push_back({ sendID, handler });
        }
    }
    if (haveError) {
        handler(earlyError.exceptionId, cppSimConnect_Exceptions[earlyError.exceptionId], earlyError.parmIndex);
    }
}

void SimState::onExcept(DWORD sendID, unsigned exceptionId, unsigned parmIndex)
{
    ExceptionCallback handler;
    {
        std::lock_guard<std::mutex> lock(_onExceptMutex);

        for (const auto& exceptionHandler : _onExcept) {
            if (sendID == exceptionHandler.sendID) {
                handler = exceptionHandler.handler;
                break;
            }
        }
        if (!handler) {
            _earlyErrors[sendID] = { exceptionId, parmIndex };
        }
    }
    if (handler) {
        handler(exceptionId, cppSimConnect_Exceptions[exceptionId], parmIndex);
    }
}