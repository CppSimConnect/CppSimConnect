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

std::map<std::string, std::unique_ptr<CppSimConnect::SimConnect>> CppSimConnect::SimConnect::clients;


void SimConnect::start() noexcept {
    if (_logger && (_loggingThreshold >= LogLevel::Debug)) {
        _logger(LogLevel::Debug, "SimConnect::start()");
    }
    if (!_running.exchange(true)) {
        std::lock_guard lock(_simConnector);
        notifyStateChanged("Starting connector-thread.");
        autoConnector = std::jthread(std::mem_fn(&SimConnect::autoConnectHandler), this);
    }
    running_cv.notify_all();
}

void CppSimConnect::SimConnect::autoConnectHandler()
{
    while (running())
    {
        if (connected())
        {
            notifyStateChanged("Handling messages.");

            simDrainDispatchQueue();

            std::unique_lock lock(_simConnector);
            connection_cv.wait_for(lock, _messagePollerRetryPeriod, [this] { return !running() || !connected(); });
        }
        else if (_autoConnect)
        {
            notifyStateChanged("Starting auto-connect loop.");
            while (running() && !connected())
            {
                if (!connect())
                {
                    std::unique_lock lock(_simConnector);
                    running_cv.wait_for(lock, _autoConnectRetryPeriod, [this] { return !running() || !_autoConnect; });
                }
            }
        } else {
            notifyStateChanged("Waiting for connect.");
            std::unique_lock lock(_simConnector);
            running_cv.wait(lock, [this] { return !running() || _autoConnect; });
        }
    }
}

[[nodiscard]]
bool SimConnect::connect() noexcept
{
    if (!connected()) { disconnect(); }

    bool result = simConnect();
    if (result) {
        connection_cv.notify_all();

        notifyConnected();

        simDispatch();
    }
    return result;
}

void CppSimConnect::SimConnect::disconnect() noexcept
{
    if (simDisconnect()) {
        connection_cv.notify_all();

        notifyDisconnected();
    }
}
