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

#include "../CppSimConnect.h"

#include <atomic>


namespace CppSimConnect {

	using EventID = unsigned;
	using GroupID = unsigned;

	class EventManager {
		EventManager(const EventManager&) = delete;
		EventManager(EventManager&&) = delete;
		EventManager& operator=(const EventManager&) = delete;
		EventManager& operator=(EventManager&&) = delete;

		SimConnect& _api;

		static std::mutex _eventLock;
		static EventID _nextID;
		static GroupID _nextGroup;
		static std::vector<std::string> _names;
		static std::map<std::string, EventID> _ids;
		template <typename T>

		EventID eventID(T&& eventName) {
			std::lock_guard<std::mutex> lock(_eventLock);
			std::string name{ std::forward<T>(eventName) };
			auto it{ _ids.find(name) };
			if (it == _ids.cend()) {
				_names.resize(_nextID + 1, name);
				_ids.emplace(std::move(name), _nextID);
				return _nextID++;
			}
			return it->second;
		}

	public:
		EventManager(SimConnect& api) : _api{ api } {}
		~EventManager() = default;

		
	};
}