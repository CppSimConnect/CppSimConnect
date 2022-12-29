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

#include <string>

namespace CppSimConnect::messages {

	struct AppInfo {
		std::string appName;
		std::string appVersionMajor;
		std::string appVersionMinor;
		std::string appBuildMajor;
		std::string appBuildMinor;

		std::string scVersionMajor;
		std::string scVersionMinor;
		std::string scBuildMajor;
		std::string scBuildMinor;

		void operator=(SIMCONNECT_RECV_OPEN const& msg) {
			appName = msg.szApplicationName;

			appVersionMajor = std::to_string(msg.dwApplicationVersionMajor);
			appVersionMinor = std::to_string(msg.dwApplicationVersionMinor);
			appBuildMajor = std::to_string(msg.dwApplicationBuildMajor);
			appBuildMinor = std::to_string(msg.dwApplicationBuildMinor);
			scVersionMajor = std::to_string(msg.dwSimConnectVersionMajor);
			scVersionMinor = std::to_string(msg.dwSimConnectVersionMinor);
			scBuildMajor = std::to_string(msg.dwSimConnectBuildMajor);
			scBuildMinor = std::to_string(msg.dwSimConnectBuildMinor);
		}
	};
}