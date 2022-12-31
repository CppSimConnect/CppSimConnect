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


using CppSimConnect::LogLevel;
using CppSimConnect::SimConnect;


void SimConnect::createState()
{
    if (haveState()) {
        releaseState();
    }
    _state = new SimState;
}

void SimConnect::releaseState()
{
    delete _state;
    _state = nullptr;
}

bool SimConnect::simConnect()
{
    if (_state) {
        if (_logger && (_loggingThreshold >= LogLevel::Warn)) {
            _logger(LogLevel::Warn, "Forcing SimConnect_Close() to clean up old handle.");
        }
        simDisconnect();
    }
    HANDLE h;
    HRESULT result = SimConnect_Open(&h, _clientName.c_str(), nullptr, 0, nullptr, 0);
    if (SUCCEEDED(result)) {
        createState();
        _state->_handle = h;
    }
    else if (!_autoConnect && _logger && (LogLevel::Error >= _loggingThreshold)) {
        _logger(LogLevel::Error, std::format("Connection to simulator failed. (0x{:08x})", result));
    }
    return SUCCEEDED(result);
}

bool SimConnect::simDisconnect()
{
    if (!_state) {
        return false; // Already disconnected
    }
    if (_state->_handle == nullptr) {
        releaseState();
        if (_logger && (LogLevel::Warn >= _loggingThreshold)) {
            _logger(LogLevel::Warn, "Not connected, but cleaning up state.");
        }
        return false; // Already disconnected.. assuming we just forgot to clen up?
    }
    HANDLE h{ _state->_handle };
    _state->_handle = nullptr;

    HRESULT result = SimConnect_Close(h);
    if (FAILED(result) && _logger && (LogLevel::Error >= _loggingThreshold)) {
        _logger(LogLevel::Error, "Failed to disconnect from simulator.");
    }
    return SUCCEEDED(result);
}

void SimConnect::simDispatch()
{
    HRESULT dpResult = SimConnect_CallDispatch(_state->_handle, &SimState::cppSimConnect_MSFS_handleMessage, this);
    if (FAILED(dpResult) && _logger && (LogLevel::Error >= _loggingThreshold)) {
        _logger(LogLevel::Error, std::format("Failed to start message dispatcher. (0x{:08x})", dpResult));
    }
}

void SimConnect::simDrainDispatchQueue()
{
    SIMCONNECT_RECV* msgPtr;
    DWORD msgLen;
    while (SUCCEEDED(SimConnect_GetNextDispatch(_state->_handle, &msgPtr, &msgLen))) {
        if ((msgPtr == nullptr) || (msgLen < sizeof(SIMCONNECT_RECV)) || (msgPtr->dwID == SIMCONNECT_RECV_ID_NULL)) {
            break;
        }
        SimState::cppSimConnect_MSFS_handleMessage(msgPtr, msgLen, this);
    }
}
