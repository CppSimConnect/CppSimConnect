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

#include "../CppSimConnect/SimConnect.h"

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

    sim.start();

    while (true) {
        std::cerr << "Waiting..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));

    }
}
