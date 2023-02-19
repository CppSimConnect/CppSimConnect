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

#include <exception>
#include <stdexcept>

#include <string>
#include <format>


namespace CppSimConnect {

	class SimException : public std::exception {
		unsigned _exceptionId;
		std::string _exceptionMsg;
		unsigned _parmIndex;

		std::string _msg;

	public:
		SimException(unsigned exceptionId, std::string exceptionMsg, unsigned parmIndex) :
			_exceptionId{ exceptionId },
			_exceptionMsg(std::move(exceptionMsg)),
			_parmIndex{ parmIndex },
			_msg(std::format("SimConnect exception id {} for parameter {}: {}", _exceptionId, _parmIndex, _exceptionMsg))
		{
		}
		~SimException() = default;
		SimException(SimException&&) = default;
		SimException(const SimException&) = default;
		SimException& operator=(SimException&&) = default;
		SimException& operator=(const SimException&) = default;

		virtual const char* what() const noexcept override {
			return _msg.c_str();
		}

		inline unsigned exceptionId() const noexcept { return _exceptionId; }
		inline const std::string& exceptionMsg() const noexcept { return _exceptionMsg; }
		inline unsigned parmIndex() const noexcept { return _parmIndex; }
	};
}