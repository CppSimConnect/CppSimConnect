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

#include <iostream>

#include "../CppSimConnect/CppSimConnect.h"
#include "../CppSimConnect/exceptions/NotConnected.h"

using CppSimConnect::SimConnect;

int main()
{
    auto& sim = SimConnect::Builder()
        .withName("CppSimConnectTester")
        .withAutoConnect()
        .withAutoConnectRetryPeriod(std::chrono::seconds(1))
        .withMessagePollerRetryPeriod(std::chrono::seconds(1))
        .startStopped()
        .stopOnDisconnect()
        .withLogThreshold(CppSimConnect::LogLevel::Debug)
        .withLogger([]([[maybe_unused]]auto level, auto msg) { std::cerr << msg << std::endl; })
        .build();

    sim.onOpen([](auto appInfo) {
        std::cerr
            << "Connected to " << appInfo.appName
            << " version " << appInfo.appVersionMajor << "." << appInfo.appVersionMinor
            << " (build " << appInfo.appBuildMajor << "." << appInfo.appBuildMinor << ")"
            << " using SimConnect version " << appInfo.scVersionMajor << "." << appInfo.scVersionMinor
            << " (build " << appInfo.scBuildMajor << "." << appInfo.scBuildMinor << ")"
            << std::endl;
    });
    sim.onClose([] {
        std::cerr
            << "Simulator shutting down!"
            << std::endl;
    });
    sim.onConnect([] {
        std::cerr
            << "Connected!"
            << std::endl;
    });
    sim.onDisconnect([] {
        std::cerr
            << "Disconnected!"
            << std::endl;
        });
    sim.addStateLogger([](auto msg) {
        std::cerr << msg << std::endl;
        });

    auto deadline = std::chrono::steady_clock::now() + std::chrono::minutes(5);

    sim.start();

    while (std::chrono::steady_clock::now() < deadline) {
        std::string airFile("<unknown>");
        try {
            airFile = sim.currentAircraftAirFile();
            if (sim.isSimInDialogMode()) {
                std::cerr << "Sim is currently in a dialog.\n";
            }
            else {
                std::cerr << "Sim is running\n";
            }
            if (sim.isUserFlying()) {
                std::cerr << "The user is currently flying.\n";
            }
            else {
                std::cerr << "The user is currently doing dialog stuff.\n";
            }
        }
        catch (CppSimConnect::NotConnected&) {
            std::cerr << "[Not connected, no airfile available]\n";
        }
        std::cerr << "Waiting 5 seconds (aircraftLoaded = '" << airFile << "')" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    std::cerr << "Shutting down" << std::endl;
    sim.stop();
}
