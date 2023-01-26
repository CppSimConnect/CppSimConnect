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

// These need to go here to hide the SimState class.

SimConnect::SimConnect(SimConnect::Builder const& builder) :
    _clientName{ builder._clientName },
    _autoConnect{ builder._autoConnect },
    _autoConnectRetryPeriod{ builder._autoConnectRetryPeriod },
    _messagePollerRetryPeriod{ builder._messagePollerRetryPeriod },
    _stopOnDisconnect{ builder._stopOnDisconnect },
    _loggingThreshold{ builder._loggingThreshold },
    _sink{ builder._logger },
    _logger{ "SimConnect", _sink, _loggingThreshold }
{
    if (builder._startRunning) {
        start();
    }
}

SimConnect::~SimConnect() {
    disconnect();
    stop();
}

std::map<std::string, std::shared_ptr<CppSimConnect::SimConnect>> CppSimConnect::SimConnect::_clients;


void SimConnect::start() noexcept {
    _logger.debug("SimConnect::start()");
    if (!_running.exchange(true)) {
        std::lock_guard lock(_simConnector);
        notifyStateChanged("Starting connector-thread.");
        _autoConnector = std::jthread(std::mem_fn(&SimConnect::autoConnectHandler), this);
    }
    _running_cv.notify_all();
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
            _connection_cv.wait_for(lock, _messagePollerRetryPeriod, [this] { return !running() || !connected(); });
        }
        else if (_autoConnect)
        {
            notifyStateChanged("Starting auto-connect loop.");
            while (running() && !connected())
            {
                if (!connect(true))
                {
                    std::unique_lock lock(_simConnector);
                    _running_cv.wait_for(lock, _autoConnectRetryPeriod, [this] { return !running() || !_autoConnect; });
                }
            }
        } else {
            notifyStateChanged("Waiting for connect.");
            std::unique_lock lock(_simConnector);
            _running_cv.wait(lock, [this] { return !running() || _autoConnect; });
        }
    }
}

[[nodiscard]]
bool SimConnect::connect(bool byAutoConnect)
{
    if (!connected()) { disconnect(); }

    bool result = simConnect(byAutoConnect);
    if (result) {
        _connected = true;
        _connection_cv.notify_all();

        notifyConnected();

        simDispatch();
    }
    return result;
}

void CppSimConnect::SimConnect::disconnect() noexcept
{
    if (simDisconnect()) {
        _connected = false;
        _connection_cv.notify_all();

        notifyDisconnected();
    }
}
