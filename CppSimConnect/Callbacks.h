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

#include <functional>
#include <mutex>
#include <vector>

namespace CppSimConnect {

	/**
	 * <summary></summary>
	 */
	template <typename... Tparm>
	class CallbackList {
		using Callback = std::function<void(Tparm...)>;
		using CallbackVector = std::vector<Callback>;

		CallbackVector _callbacks;

	public:
		inline auto size() const noexcept { return _callbacks.size(); }
		inline void add(Callback cb) { _callbacks.emplace_back(cb); }
		inline void operator+=(Callback cb) { _callbacks.emplace_back(cb); }
		inline void operator()(Tparm... parms) {
			for (const Callback& cb : _callbacks)
			{
				cb(parms...);
			}
		}
	};

	enum class CallbackResult {
		Ok,			// Conitune processing
		Done,		// Continue, but I'm done
		Abort,		// Abort processing
		AbortDone,
	};

	template <typename... Tparm>
	class ShortcutCallbackList {
		using Callback = std::function<CallbackResult(Tparm...)>;
		using CallbackVector = std::vector<Callback>;

		CallbackVector _callbacks;

	public:
		inline auto size() const noexcept { return _callbacks.size(); }
		inline void add(Callback cb) { _callbacks.emplace_back(cb); }
		inline void operator+=(Callback cb) { _callbacks.emplace_back(cb); }
		inline CallbackResult operator()(Tparm... parms) {
			for (const Callback& cb : _callbacks)
			{
				switch (cb(parms...)) {
				case CallbackResult::Abort:
				case CallbackResult::AbortDone:
					return CallbackResult::Abort;
				}
			}
			return CallbackResult::Ok;
		}
	};

	template <typename... Tparm>
	class CleanableCallbackList {
		using Callback = std::function<CallbackResult(Tparm...)>;
		using CallbackVector = std::vector<std::pair<unsigned, Callback>>;

		CallbackVector _callbacks;
		std::atomic_uint _nextId{ 0 };
		std::mutex _mutex;

	public:
		inline auto size() const noexcept { return _callbacks.size(); }
		inline unsigned add(Callback cb) { auto id = _nextId++; _callbacks.emplace_back(id, cb); return id; }
		inline void operator+=(Callback cb) { auto id = _nextId++; _callbacks.emplace_back(id, cb); }
		inline void operator-=(unsigned id) {
			std::lock_guard<std::mutex> lock(_mutex);
			auto end = std::remove_if(_callbacks.begin(), _callbacks.end(), [id](const auto& it) { return id == it.first; });
			_callbacks.erase(end, _callbacks.end());
		}
		CallbackResult operator()(Tparm... parms) {
			CallbackResult result{ CallbackResult::Ok };

			CallbackVector callbacks(_callbacks.size());
			{
				std::lock_guard<std::mutex> lock(_mutex);
				callbacks = _callbacks;
			}
			std::set<unsigned> idsToGo;
			for (auto pair : callbacks)
			{
				switch (pair.second(parms...)) {
				case CallbackResult::Done:
				case CallbackResult::AbortDone:
					idsToGo.emplace(pair.first);
					result = CallbackResult::Done;
					break;
				}
			}
			{
				std::lock_guard<std::mutex> lock(_mutex);
				auto end = std::remove_if(_callbacks.begin(), _callbacks.end(), [&idsToGo](const auto& it) { return idsToGo.contains(it.first); });
				_callbacks.erase(end, _callbacks.end());
			}
			return result;
		}
	};

	template <typename... Tparm>
	class CleanableShortcutCallbackList {
		using Callback = std::function<CallbackResult(Tparm...)>;
		using CallbackVector = std::vector<std::pair<unsigned, Callback>>;

		CallbackVector _callbacks;
		std::atomic_uint _nextId{ 0 };
		std::mutex _mutex;

	public:
		inline auto size() const noexcept { return _callbacks.size(); }
		inline unsigned add(Callback cb) { auto id = _nextId++; _callbacks.emplace_back(id, cb); return id; }
		inline void operator+=(Callback cb) { auto id = _nextId++; _callbacks.emplace_back(id, cb); }
		inline void operator-=(unsigned id) {
			std::lock_guard<std::mutex> lock(_mutex);
			auto end = std::remove_if(_callbacks.begin(), _callbacks.end(), [id](const auto& it) { return id == it.first; });
			_callbacks.erase(end, _callbacks.end());
		}
		CallbackResult operator()(Tparm... parms) {
			CallbackResult result{ CallbackResult::Ok };

			CallbackVector callbacks(_callbacks.size());
			{
				std::lock_guard<std::mutex> lock(_mutex);
				callbacks = _callbacks;
			}
			std::set<unsigned> idsToGo;
			for (auto pair : callbacks)
			{
				bool abort = false;
				switch (pair.second(parms...)) {
				case CallbackResult::Done:
					idsToGo.emplace(pair.first);
					result = CallbackResult::Done;
					break;

				case CallbackResult::AbortDone:
					idsToGo.emplace(pair.first);
					result = CallbackResult::AbortDone;
					abort = true;
					break;

				case CallbackResult::Abort:
					result = CallbackResult::Abort;
					abort = true;
					break;
				}
				if (abort) {
					break;
				}
			}
			{
				std::lock_guard<std::mutex> lock(_mutex);
				auto end = std::remove_if(_callbacks.begin(), _callbacks.end(), [&idsToGo](const auto& it) { return idsToGo.contains(it.first); });
				_callbacks.erase(end, _callbacks.end());
			}
			return result;
		}
	};

}