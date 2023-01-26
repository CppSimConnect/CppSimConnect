/*
 * Copyright (c) 2023. Bert Laverman
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

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "../EventManager.h"

namespace CppSimConnect {

	class ClientEvent {
	public:
		using EventID = unsigned int;

	private:

		std::weak_ptr<EventManager> _api;
		EventID _id;
		std::string _name;

		ClientEvent(ClientEvent&&) = delete;
		ClientEvent& operator=(ClientEvent&&) = delete;

	public:
//		ClientEvent(std::string eventName) : _name{ _name }, _id{ eventID(eventName) } {}
//		ClientEvent(std::weak_ptr<SimConnect> api, std::string eventName) : _api{ api }, _id { eventID(eventName) } {}
//
//		ClientEvent(const ClientEvent& event) = default;
//		~ClientEvent() = default;
//		ClientEvent& operator=(const ClientEvent&) = default;
//
//		inline EventID id() const noexcept { return _id; }
//		inline const std::string& name() const noexcept { return EventManager::_names.at(_id); }
	};
}