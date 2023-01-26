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
#include <vector>
#include <atomic>


namespace CppSimConnect {
	namespace Reactive {


		template <typename Tmsg, typename Terr>
		class MessageObserver {
		public:
			using NextAction = std::function<void(const Tmsg&)>;
			using ErrorAction = std::function<void(const Terr&)>;
			using CleanupAction = std::function<void()>;

			MessageObserver() {
				_completed.clear();
			}
			~MessageObserver() {
				onCompleted();
			}
			MessageObserver(const MessageObserver&) = default;
			MessageObserver(MessageObserver&&) = default;
			MessageObserver& operator=(const MessageObserver&) = default;
			MessageObserver& operator=(MessageObserver&&) = default;

		private:
			std::atomic_flag _completed;
			Terr _error;
			NextAction _onNext;
			ErrorAction _onError;
			CleanupAction _onComplete;

		public:
			inline void withOnNext(NextAction action) noexcept {
				_onNext = action;
			}

			inline void withOnError(ErrorAction action) noexcept {
				_onError = action;
			}

			inline void withOnComplete(CleanupAction action) noexcept {
				_onComplete = action;
			}

			void onNext(const Tmsg& msg) {
				if (!_completed.test() && _onNext) {
					_onNext(msg);
				}
			}

			void onCompleted() {
				if (!_completed.test_and_set() && _onComplete) {
					_onComplete();
				}
			}

			void onError(const Terr& err) {
				if (_completed.test_and_set()) {
					return;
				}
				_error = err;
				if (_onError) {
					_onError(_error);
				}
			}

			inline const Terr& error() const noexcept { return _error; }
			inline bool completed() const noexcept { return _completed.test(); }
		};
	}
}