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

#include <future>

#include "MessageObserver.h"


namespace CppSimConnect {
	namespace Reactive {

		template <typename Tmsg>
		class _MessageResult : public _MessageObserver<Tmsg> {
			std::promise<Tmsg> _future;

		public:
			_MessageResult() = default;
			~_MessageResult() = default;
			_MessageResult(_MessageResult<Tmsg>&&) = default;
			_MessageResult(const _MessageResult<Tmsg>& other) : MessageObserver<Tmsg>(other) {};
			_MessageResult<Tmsg>& operator=(_MessageResult<Tmsg>&&) = default;
			_MessageResult<Tmsg>& operator=(const _MessageResult<Tmsg>&) = default;

			virtual void onNext(const Tmsg& msg) override {
				try {
					_future.set_value(msg);
				}
				catch (std::future_error _) {
					//TODO
				}
				_MessageObserver<Tmsg>::onNext(msg);
				onCompleted();
			}

			virtual void onCompleted() override {
				_MessageObserver<Tmsg>::onCompleted();
			}

			virtual void onError(std::exception_ptr err) override {
				try {
					_future.set_exception(err);
				}
				catch (std::future_error _) {
					// TODO
				}
				_MessageObserver<Tmsg>::onError(err);
			}

			inline Tmsg get() {
				if (_MessageObserver<Tmsg>::completed() && (_MessageObserver<Tmsg>::error() != nullptr)) {
					std::rethrow_exception(_MessageObserver<Tmsg>::error());
				}
				return _future.get_future().get();
			}
		};

		template <typename Tmsg>
		class MessageResult : public virtual MessageObserver<Tmsg, _MessageResult<Tmsg>> {
		public:
			MessageResult() = default;
			~MessageResult() = default;
			MessageResult(MessageResult<Tmsg>&&) = default;
			MessageResult(const MessageResult<Tmsg>&) = default;
			MessageResult<Tmsg>& operator=(MessageResult<Tmsg>&&) = default;
			MessageResult<Tmsg>& operator=(const MessageResult<Tmsg>&) = default;

			Tmsg get() const {
				return MessageObserver<Tmsg, _MessageResult<Tmsg>>::_obs->get();
			}
		};
	}
}