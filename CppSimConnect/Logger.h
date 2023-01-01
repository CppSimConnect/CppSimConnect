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
#include <format>
#include <optional>
#include <functional>

namespace CppSimConnect {

	enum class LogLevel {
		Trace,
		Debug,
		Info,
		Warn,
		Error,
		Fatal
	};

	using LogSink = std::function<void(LogLevel, const std::string&)>;

	class Logger {
		friend class LeveledLogger;

		std::string _name;
		LogLevel _threshold;
		LogSink &_logger;

	public:
		Logger(std::string name, LogSink &logger, LogLevel threshold = LogLevel::Info) : _name(std::move(name)), _logger{ logger }, _threshold(threshold) {}
		~Logger() = default;
		Logger(Logger const&) = default;
		Logger(Logger&&) = default;
		Logger& operator=(Logger const&) = default;
		Logger& operator=(Logger&&) = default;

		inline void trace(const char* msg) {
			if (_logger && _threshold <= LogLevel::Trace) _logger(LogLevel::Trace, msg);
		};
		inline void trace(std::string msg) {
			if (_logger && _threshold <= LogLevel::Trace) _logger(LogLevel::Trace, msg);
		};
		template <typename ...Targs>
		inline void trace(const std::_Fmt_string<Targs...> fmt, Targs... args) {
			if (_logger && _threshold <= LogLevel::Trace) _logger(LogLevel::Trace, std::format(fmt, std::forward<Targs>(args)...));
		}

		inline void debug(const char* msg) {
			if (_logger && _threshold <= LogLevel::Debug) _logger(LogLevel::Debug, msg);
		};
		inline void debug(std::string msg) {
			if (_logger && _threshold <= LogLevel::Debug) _logger(LogLevel::Debug, msg);
		};
		template <typename ...Targs>
		inline void debug(const std::_Fmt_string<Targs...> fmt, Targs... args) {
			if (_logger && _threshold <= LogLevel::Debug) _logger(LogLevel::Debug, std::format(fmt, std::forward<Targs>(args)...));
		}

		inline void info(const char* msg) {
			if (_logger && _threshold <= LogLevel::Info) _logger(LogLevel::Info, msg);
		};
		inline void info(std::string msg) {
			if (_logger && _threshold <= LogLevel::Info) _logger(LogLevel::Info, msg);
		};
		template <typename ...Targs>
		inline void info(const std::_Fmt_string<Targs...> fmt, Targs... args) {
			if (_logger && _threshold <= LogLevel::Info) _logger(LogLevel::Info, std::format(fmt, std::forward<Targs>(args)...));
		}

		inline void warn(const char* msg) {
			if (_logger && _threshold <= LogLevel::Warn) _logger(LogLevel::Warn, msg);
		};
		inline void warn(std::string msg) {
			if (_logger && _threshold <= LogLevel::Warn) _logger(LogLevel::Warn, msg);
		};
		template <typename ...Targs>
		inline void warn(const std::_Fmt_string<Targs...> fmt, Targs... args) {
			if (_logger && _threshold <= LogLevel::Warn) _logger(LogLevel::Warn, std::format(fmt, std::forward<Targs>(args)...));
		}

		inline void error(const char* msg) {
			if (_logger && _threshold <= LogLevel::Error) _logger(LogLevel::Error, msg);
		};
		inline void error(std::string msg) {
			if (_logger && _threshold <= LogLevel::Error) _logger(LogLevel::Error, msg);
		};
		template <typename... Targs>
		inline void error(const std::_Fmt_string<Targs...> fmt, Targs... args) {
			if (_logger && _threshold <= LogLevel::Error) _logger(LogLevel::Error, std::format(fmt, std::forward<Targs>(args)...));
		}

		inline void fatal(const char* msg) {
			if (_logger && _threshold <= LogLevel::Fatal) _logger(LogLevel::Fatal, msg);
		};
		inline void fatal(std::string msg) {
			if (_logger && _threshold <= LogLevel::Fatal) _logger(LogLevel::Fatal, msg);
		};
		template <typename ...Targs>
		inline void fatal(const std::_Fmt_string<Targs...> fmt, Targs... args) {
			if (_logger && _threshold <= LogLevel::Fatal) _logger(LogLevel::Fatal, std::format(fmt, std::forward<Targs>(args)...));
		}

	};

}