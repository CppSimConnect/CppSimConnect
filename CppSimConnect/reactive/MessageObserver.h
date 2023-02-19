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
#include <functional>
#include <vector>
#include <atomic>

#include "Callbacks.h"


namespace CppSimConnect {
	namespace Reactive {

		template <typename Terr>
		static inline Terr errorFromPtr(std::exception_ptr exc) {
			try {
				std::rethrow_exception(exc);
			}
			catch (const Terr& e) {
				return e;
			}
			catch (...) {
				throw;
			}
		}

		template <typename Tmsg>
		class _MessageObserver {
		public:
			_MessageObserver() {
				_completed.clear();
			}
			~_MessageObserver() {
				onCompleted();
			}
			_MessageObserver(_MessageObserver<Tmsg>&&) = delete;
			_MessageObserver(const _MessageObserver<Tmsg>& other) : _error(other._error), _onNext(other._onNext), _onError(other._onError), _onCompleted(other._onCompleted) {
				if (other.completed()) {
					_completed.test_and_set();
				}
			}
			_MessageObserver<Tmsg>& operator=(_MessageObserver<Tmsg>&&) = delete;
			_MessageObserver<Tmsg>& operator=(const _MessageObserver<Tmsg>&) = default;

		private:
			std::atomic_flag _completed;
			std::exception_ptr _error;
			CallbackList<Tmsg> _onNext;
			CallbackList<std::exception_ptr> _onError;
			CallbackList<> _onCompleted;

		public:
			using NextAction = CallbackList<Tmsg>::Callback;
			using ErrorAction = CallbackList<std::exception_ptr>::Callback;
			using CleanupAction = CallbackList<>::Callback;

			inline _MessageObserver<Tmsg>& withOnNext(NextAction action) {
				_onNext += action;
				return *this;
			}

			inline _MessageObserver<Tmsg>& withOnError(ErrorAction action) {
				_onError += action;
				if (completed() && (_error != nullptr)) {
					action(_error);
				}
				return *this;
			}

			inline _MessageObserver<Tmsg>& withOnComplete(CleanupAction action) {
				_onCompleted += action;
				if (completed()) {
					action();
				}
				return *this;
			}

			inline _MessageObserver<Tmsg>& operator+=(const _MessageObserver<Tmsg>& other) {
				_onNext += other._onNext;
				_onCompleted += other._onCompleted;
				_onError += other._onError;
				return *this;
			}

			virtual void onNext(const Tmsg& msg) {
				if (!_completed.test()) {
					try {
						_onNext(msg);
					}
					catch (...) {
						onError(std::current_exception());
					}
				}
			}

			virtual void onCompleted() {
				if (!_completed.test_and_set()) {
					try {
						_onCompleted();
					}
					catch (...) {
						onError(std::current_exception());
					}
				}
			}

			virtual void onError(std::exception_ptr err) {
				if (_completed.test_and_set()) {
					return;
				}
				_error = err;
				_onError(err);
				_onCompleted();
			}

			inline bool completed() const { return _completed.test(); }
			inline std::exception_ptr error() const { return _error; }
		};

		template <typename Tmsg, typename Tobs>
		class MessageObserver {
		protected:
			std::shared_ptr<Tobs> _obs;

		public:
			inline const Tobs& obs() const { return *_obs; }
			inline Tobs& obs() { return *_obs; }

		public:
			using NextAction = CallbackList<Tmsg>::Callback;
			using ErrorAction = CallbackList<std::exception_ptr>::Callback;
			using CleanupAction = CallbackList<>::Callback;

			MessageObserver() : _obs(std::make_shared<Tobs>()) {}
			~MessageObserver() = default;
			MessageObserver(MessageObserver<Tmsg, Tobs>&&) = default;
			MessageObserver(const MessageObserver<Tmsg, Tobs>&) = default;
			MessageObserver<Tmsg, Tobs>& operator=(MessageObserver<Tmsg, Tobs>&&) = default;
			MessageObserver<Tmsg, Tobs>& operator=(const MessageObserver<Tmsg, Tobs>&) = default;

			inline const MessageObserver<Tmsg, Tobs>& withOnNext(NextAction action) const {
				_obs->withOnNext(action);
				return *this;
			}

			inline const MessageObserver<Tmsg, Tobs>& withOnError(ErrorAction action) const {
				_obs->withOnError(action);
				return *this;
			}

			inline const MessageObserver<Tmsg, Tobs>& withOnComplete(CleanupAction action) const {
				_obs->withOnComplete(action);
				return *this;
			}

			inline const MessageObserver<Tmsg, Tobs>& operator+=(const MessageObserver<Tmsg, Tobs>& other) const {
				(*_obs) += *(other._obs);
				return *this;
			}

			inline void onNext(const Tmsg& msg) const {
				_obs->onNext(msg);
			}

			inline void onCompleted() const {
				_obs->onCompleted();
			}

			inline void onError(std::exception_ptr err) const {
				_obs->onError(err);
			}


			inline const MessageObserver<Tmsg, Tobs>& subscribe(NextAction onNext) const {
				return withOnNext(onNext);
			}
			inline const MessageObserver<Tmsg, Tobs>& subscribe(NextAction onNext, ErrorAction onError) const {
				return withOnNext(onNext).withOnError(onError);
			}
			inline const MessageObserver<Tmsg, Tobs>& subscribe(NextAction onNext, CleanupAction onCompleted) const {
				return withOnNext(onNext).withOnComplete(onCompleted);
			}
			inline const MessageObserver<Tmsg, Tobs>& subscribe(NextAction onNext, ErrorAction onError, CleanupAction onCompleted) const {
				return withOnNext(onNext).withOnError(onError).withOnComplete(onCompleted);
			}

			inline std::exception_ptr error() const noexcept { return obs().error(); }
			template <typename Terr>
			inline Terr error() const { return errorFromPtr<Terr>(obs().error()); }
			inline bool completed() const noexcept { return obs().completed(); }
		};

	}

}