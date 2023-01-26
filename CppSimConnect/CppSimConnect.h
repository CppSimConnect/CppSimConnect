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

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <map>

#include "Logger.h"

#include "AppInfo.h"

#include "events/SystemState.h"


namespace CppSimConnect {

	enum class FlightSimType {
		Unknown,
		Prepar3Dv5,
		MSFS2020
	};

	class SimState;

	class SimConnect {
	public:
		class Builder;
		friend class Builder;

		friend class SimState;

		SimConnect(Builder const& builder);

		~SimConnect();

		SimConnect(SimConnect const&) = delete;
		SimConnect(SimConnect&&) = delete;
		SimConnect& operator=(SimConnect const&) = delete;
		SimConnect& operator=(SimConnect&&) = delete;

		std::weak_ptr<SimConnect> weakThis() const noexcept { return _clients.at(_clientName); }

	private:
		std::unique_ptr<SimState> _state;

		bool _stopOnDisconnect;
		std::mutex _simConnector;

		std::atomic_bool _running;
		std::condition_variable _running_cv;

		std::atomic_bool _connected;
		std::condition_variable _connection_cv;

		std::jthread _messageDispatcher;

		// Client connections
		static std::map<std::string, std::shared_ptr<SimConnect>> _clients;
		std::string _clientName;
		messages::AppInfo _appInfo;
		FlightSimType _connectedSim{ FlightSimType::Unknown };

		// Connecting to the simulator
		std::atomic_bool _autoConnect;
		std::chrono::milliseconds _autoConnectRetryPeriod;
		std::chrono::milliseconds _messagePollerRetryPeriod;

		void autoConnectHandler();
		std::jthread _autoConnector;

		LogLevel _loggingThreshold{ LogLevel::Info };
		LogSink _sink;
		Logger _logger;

		std::vector<std::function<void(std::string const& msg)>> stateLoggers;
		void notifyStateChanged(std::string const& msg) const { for (auto const& cb : stateLoggers) { cb(msg); } }

		std::vector<std::function<void()>> onConnectHandlers;
		void notifyConnected() const { for (auto const& cb : onConnectHandlers) { cb(); } }

		std::vector<std::function<void(messages::AppInfo const& appInfo)>> onOpenHandlers;
		void notifyOpen() const { for (auto const& cb : onOpenHandlers) { cb(_appInfo); } }
		std::vector<std::function<void()>> onCloseHandlers;
		void notifyClose() const { for (auto const& cb : onCloseHandlers) { cb(); } }
		std::vector<std::function<void()>> onDisconnectHandlers;
		void notifyDisconnected() const { for (auto const& cb : onDisconnectHandlers) { cb(); } }

		bool simConnect(bool byAutoConnect = false);
		bool simDisconnect() noexcept;
		void simDispatch() noexcept;
		void simDrainDispatchQueue() noexcept;


	public:
		// SimConnect state

		bool running() const { return _running; }
		void start() noexcept;
		void stop() noexcept {
			_logger.debug("SimConnect::stop()");

			_running = false;
			_running_cv.notify_all();
		}

		inline bool connected() const noexcept { return _connected; }

		[[nodiscard]]
		bool connect(bool byAutoConnect = false);
		void disconnect() noexcept;

		inline bool autoConnect() const noexcept { return _autoConnect; }
		void autoConnect(bool autoConnect = true) {
			_autoConnect = autoConnect;
			_connection_cv.notify_all();
		}

		inline std::chrono::milliseconds autoConnectRetryPeriods() const noexcept { return _autoConnectRetryPeriod; }
		template <typename Repr>
		void autoConnectRetryPeriods(std::chrono::duration<Repr> period) { _autoConnectRetryPeriod = period; }

		inline std::chrono::milliseconds messagePollerRetryPeriod() const { return _messagePollerRetryPeriod; }
		template <typename Repr>
		void messagePollerRetryPeriod(std::chrono::duration<Repr> period) { _messagePollerRetryPeriod = period; }

		// Simulator state
		
		void RequestSystemState();
		void SubscribeToSystemState();

		// Callbacks
		void addStateLogger(std::function<void(std::string const& msg)>&& cb) { stateLoggers.emplace_back(cb); }
		void onConnect(std::function<void()>&& cb) { onConnectHandlers.emplace_back(cb); }
		void onDisconnect(std::function<void()>&& cb) { onDisconnectHandlers.emplace_back(cb); }
		void onOpen(std::function<void(messages::AppInfo const& appInfo)>&& cb) { onOpenHandlers.emplace_back(cb); }
		void onClose(std::function<void()>&& cb) { onCloseHandlers.emplace_back(cb); }

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
				auto result = std::make_shared<SimConnect>(*this);
				SimConnect::_clients[_clientName] = std::move(result);
				return *SimConnect::_clients.at(_clientName);
			}
		};

	};

}