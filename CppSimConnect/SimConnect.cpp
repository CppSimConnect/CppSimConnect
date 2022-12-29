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

std::map<std::string, std::unique_ptr<CppSimConnect::SimConnect>> CppSimConnect::SimConnect::clients;


void CppSimConnect::SimConnect::autoConnectHandler()
{
    while (isRunning())
    {
        if (isConnected())
        {
            stateChanged("Handling messages.");

            SIMCONNECT_RECV* msgPtr;
            DWORD msgLen;
            while (SUCCEEDED(SimConnect_GetNextDispatch(simHandle, &msgPtr, &msgLen))) {
                if ((msgPtr == nullptr) || (msgLen < sizeof(SIMCONNECT_RECV)) || (msgPtr->dwID == SIMCONNECT_RECV_ID_NULL)) {
                    break;
                }
                handleMessage(msgPtr, msgLen, this);
            }
            std::unique_lock lock(simConnectMutex);
            connection_cv.wait_for(lock, _messagePollerRetryPeriod, [this] { return !isRunning() || !isConnected(); });
        }
        else if (_autoConnect)
        {
            stateChanged("Starting auto-connect loop.");
            while (!isConnected())
            {
                if (!connect())
                {
                    std::unique_lock lock(simConnectMutex);
                    running_cv.wait_for(lock, _autoConnectRetryPeriod, [this] { return !isRunning() || !_autoConnect; });
                }
            }
        } else {
            stateChanged("Waiting till somebody connects.");
            std::unique_lock lock(simConnectMutex);
            running_cv.wait(lock, [this] { return !isRunning() || _autoConnect; });
        }
    }
}

void CppSimConnect::SimConnect::handleMessage(SIMCONNECT_RECV* msgPtr, DWORD msgLen, void* context) noexcept
{
    if ((msgPtr == nullptr) || (msgLen < sizeof(SIMCONNECT_RECV)) || (msgPtr->dwID == SIMCONNECT_RECV_ID_NULL)) {
        return;
    }
    CppSimConnect::SimConnect& sim{ *static_cast<CppSimConnect::SimConnect*>(context) };

    switch (msgPtr->dwID) {

    case SIMCONNECT_RECV_ID_EXCEPTION:
        {
            SIMCONNECT_RECV_EXCEPTION& msg{ *static_cast<SIMCONNECT_RECV_EXCEPTION*>(msgPtr) };
            sim.onExcept(SIMCONNECT_EXCEPTION(msg.dwException), msg.dwSendID, msg.dwIndex);
        }
        break;

    case SIMCONNECT_RECV_ID_OPEN:
        sim.appInfo = *static_cast<SIMCONNECT_RECV_OPEN*>(msgPtr);
        sim.receivedOpen();
        break;

    case SIMCONNECT_RECV_ID_QUIT:
        sim.receivedClose();
        break;

    }
}

[[nodiscard]]
bool CppSimConnect::SimConnect::connect() noexcept
{
    if (!isConnected()) { disconnect(); }

    HANDLE h;
    HRESULT result = SimConnect_Open(&h, _clientName.c_str(), nullptr, 0, nullptr, 0);
    if (SUCCEEDED(result)) {
        simHandle = h;
        connection_cv.notify_all();

        connected();

        HRESULT dpResult = SimConnect_CallDispatch(simHandle, &handleMessage, this);
        if (FAILED(dpResult) && _logger && (LogLevel::Error >= _loggingThreshold)) {
            _logger(LogLevel::Error, std::format("Failed to start message dispatcher. (0x{:08x})", dpResult));
        }
    }
    else if (!_autoConnect && _logger && (LogLevel::Error >= _loggingThreshold)) {
        _logger(LogLevel::Error, std::format("Connection to simulator failed. (0x{:08x})", result));
    }
    return SUCCEEDED(result);
}

void CppSimConnect::SimConnect::disconnect() noexcept
{
    if (simHandle == nullptr) {
        return;
    }
    HANDLE h{ simHandle };
    simHandle = nullptr;

    if (SUCCEEDED(SimConnect_Close(h))) {
        connection_cv.notify_all();

        disconnected();
    }
    else if (_logger && (LogLevel::Error >= _loggingThreshold)) {
        _logger(LogLevel::Error, "Failed to disconnect from simulator.");
    }
}

void CppSimConnect::SimConnect::onExcept(SIMCONNECT_EXCEPTION e, DWORD sendId, DWORD parmIndex) {
    //TODO
}