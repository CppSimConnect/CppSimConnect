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

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#pragma warning(disable:4245)
#include <windows.h>
#include <SimConnect.h>
#pragma warning(default:4245)

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <map>


#include "message.h"

namespace CppSimConnect {

	enum class LogLevel {
		Trace,
		Debug,
		Info,
		Warn,
		Error,
		Fatal
	};

	enum class FlightSimType {
		Unknown,
		Prepar3Dv5,
		MSFS2020
	};

	class SimConnect {
	public:
		class Builder;
		friend class Builder;

		~SimConnect() {
			disconnect();
			stop();
		};

		SimConnect(Builder const& builder) :
			_clientName{ builder._clientName },
			_autoConnect{ builder._autoConnect },
			_autoConnectRetryPeriod{ builder._autoConnectRetryPeriod },
			_messagePollerRetryPeriod{ builder._messagePollerRetryPeriod },
			_stopOnDisconnect{ builder._stopOnDisconnect },
			_loggingThreshold{ builder._loggingThreshold },
			_logger{ builder._logger }
		{
			if (builder._startRunning) {
				start();
			}
		};

		SimConnect(SimConnect const&) = delete;
		SimConnect(SimConnect&&) = delete;
		SimConnect& operator=(SimConnect const&) = delete;
		SimConnect& operator=(SimConnect&&) = delete;

	private:
		std::atomic_bool running;
		bool _stopOnDisconnect;
		HANDLE simHandle{ nullptr };
		FlightSimType connectedSim{ FlightSimType::Unknown };

		std::atomic_bool _autoConnect;
		std::chrono::milliseconds _autoConnectRetryPeriod;
		std::chrono::milliseconds _messagePollerRetryPeriod;

		void autoConnectHandler();
		std::jthread autoConnector;

		std::mutex simConnectMutex;
		std::condition_variable running_cv;
		std::condition_variable connection_cv;

		std::jthread messageDispatcher;

		static std::map<std::string, std::unique_ptr<SimConnect>> clients;

		std::string _clientName;
		messages::AppInfo appInfo;

		LogLevel _loggingThreshold{ LogLevel::Info };
		std::function<void(LogLevel level, std::string)> _logger;

		std::vector<std::function<void(std::string const& msg)>> stateLoggers;
		std::vector<std::function<void()>> onConnectHandlers;
		std::vector<std::function<void(messages::AppInfo const& appInfo)>> onOpenHandlers;
		std::vector<std::function<void()>> onCloseHandlers;
		std::vector<std::function<void()>> onDisconnectHandlers;

		// Message handling
		void onExcept(SIMCONNECT_EXCEPTION e, DWORD sendId, DWORD parmIndex);

		static void handleMessage(SIMCONNECT_RECV* msgPtr, DWORD msgLen, void* context) noexcept;

	public:

		bool isRunning() const { return running; }
		void start() noexcept {
			if (!running.exchange(true)) {
				std::lock_guard lock(simConnectMutex);
				autoConnector = std::jthread(std::mem_fn(&SimConnect::autoConnectHandler), this);
			}
			running_cv.notify_all();
		}
		void stop() noexcept {
			running = false;
			running_cv.notify_all();
		}

		HANDLE const& handle() const noexcept { return simHandle; }
		bool isConnected() const noexcept { return handle() != nullptr; }

		[[nodiscard]]
		bool connect() noexcept;
		void disconnect() noexcept;

		bool autoConnect() const noexcept { return _autoConnect; }
		void autoConnect(bool autoConnect = true) {
			_autoConnect = autoConnect;
			connection_cv.notify_all();
		}

		std::chrono::milliseconds autoConnectRetryPeriods() const noexcept { return _autoConnectRetryPeriod; }
		template <typename Repr>
		void autoConnectRetryPeriods(std::chrono::duration<Repr> period) { _autoConnectRetryPeriod = period; }

		std::chrono::milliseconds messagePollerRetryPeriod() const { return _messagePollerRetryPeriod; }
		template <typename Repr>
		void messagePollerRetryPeriod(std::chrono::duration<Repr> period) { _messagePollerRetryPeriod = period; }

		// Callbacks
		void addStateLogger(std::function<void(std::string const& msg)>&& cb) { stateLoggers.emplace_back(cb); }
		void stateChanged(std::string const& msg) const { for (auto const& cb : stateLoggers) { cb(msg); } }

		void onConnect(std::function<void()>&& cb) { onConnectHandlers.emplace_back(cb); }
		void connected() const { for (auto const& cb : onConnectHandlers) { cb(); } }

		void onDisconnect(std::function<void()>&& cb) { onDisconnectHandlers.emplace_back(cb); }
		void disconnected() const { for (auto const& cb : onDisconnectHandlers) { cb(); } }

		void onOpen(std::function<void(messages::AppInfo const& appInfo)>&& cb) { onOpenHandlers.emplace_back(cb); }
		void receivedOpen() const { for (auto const& cb : onOpenHandlers) { cb(appInfo); } }

		void onClose(std::function<void()>&& cb) { onCloseHandlers.emplace_back(cb); }
		void receivedClose() const { for (auto const& cb : onCloseHandlers) { cb(); } }

		/**
		 * <summary></summary>
		 */
		class Builder {
			friend class SimConnect;

		private:
			std::string _clientName;
			bool _startRunning{ false };
			bool _autoConnect{ false };
			std::chrono::milliseconds _autoConnectRetryPeriod{ 5000 };
			std::chrono::milliseconds _messagePollerRetryPeriod{ 100 };
			bool _stopOnDisconnect{ true };

			LogLevel _loggingThreshold{ LogLevel::Info };
			std::function<void(LogLevel level, std::string)> _logger;

		public:
			Builder() = default;
			~Builder() = default;
			Builder(Builder const&) = default;
			Builder(Builder&&) = default;
			Builder& operator=(Builder const&) = default;
			Builder& operator=(Builder&&) = default;

			Builder& withName(std::string name) {
				_clientName = std::move(name);
				return *this;
			}
			Builder& startRunning() {
				_startRunning = true;
				return *this;
			}
			Builder& startStopped() {
				_startRunning = false;
				return *this;
			}
			Builder& withAutoConnect() {
				_autoConnect = true;
				return *this;
			}
			Builder& withoutAutoConnect() {
				_autoConnect = false;
				return *this;
			}
			template <typename Repr>
			Builder& withAutoConnectRetryPeriod(std::chrono::duration<Repr> period) {
				_autoConnectRetryPeriod = period;
				return *this;
			}
			template <typename Repr>
			Builder& withMessagePollerRetryPeriod(std::chrono::duration<Repr> period) {
				_messagePollerRetryPeriod = period;
				return *this;
			}
			Builder& stopOnDisconnect() {
				_stopOnDisconnect = true;
				return *this;
			}
			Builder& ignoreDisconnect() {
				_stopOnDisconnect = false;
				return *this;
			}

			Builder& withLogThreshold(LogLevel threshold) {
				_loggingThreshold = threshold;
				return *this;
			}
			Builder& withLogger(std::function<void(LogLevel, std::string)> logger) {
				_logger = logger;
				return *this;
			}

			SimConnect& build() {
				auto result = std::make_unique<SimConnect>(*this);
				SimConnect::clients[_clientName] = std::move(result);
				return *SimConnect::clients.at(_clientName);
			}
		};

	};

}