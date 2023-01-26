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

#include "pch.h"

#include "../CppSimConnect/Callbacks.h"

using CppSimConnect::CallbackResult;

TEST(TestCallbacks, testEmptyCallbacks) {

	CppSimConnect::CallbackList<std::string> callbacks;

	callbacks("Hi there!");
}

TEST(TestCallbacks, testCallbacks) {

	CppSimConnect::CallbackList<int> callbacks;

	int total{ 0 };
	callbacks.add([&total](auto i) {
		total += i;
		});
	callbacks(1);

	ASSERT_EQ(total, 1);

	callbacks(1);

	ASSERT_EQ(total, 2);

	callbacks.add([&total](auto i) { total += 2 * i; });

	ASSERT_EQ(total, 2);
	callbacks(2);
	ASSERT_EQ(total, 8);
}


TEST(TestCallbacks, testShortcutCallbacks) {

	CppSimConnect::ShortcutCallbackList<int> callbacks;

	int total{ 0 };
	callbacks.add([&total](auto i) {
		if (total > 5) return CallbackResult::Abort;
		total += i;
		return CallbackResult::Ok;
		});

	auto result = callbacks(2);
	ASSERT_EQ(total, 2);
	ASSERT_EQ(result, CallbackResult::Ok);

	result = callbacks(2);
	ASSERT_EQ(total, 4);
	ASSERT_EQ(result, CallbackResult::Ok);

	result = callbacks(2);
	ASSERT_EQ(total, 6);
	ASSERT_EQ(result, CallbackResult::Ok);

	result = callbacks(2);
	ASSERT_EQ(total, 6);
	ASSERT_EQ(result, CallbackResult::Abort);
}


TEST(TestCallbacks, testCleanableCallbacks) {

	CppSimConnect::CleanableCallbackList<int> callbacks;

	callbacks.add([](auto i) {
		if (i > 2) return CallbackResult::Done;
	return CallbackResult::Ok;
		});
	callbacks.add([](auto i) {
		if (i > 4) return CallbackResult::Done;
	return CallbackResult::Ok;
		});
	callbacks.add([](auto i) {
		if (i > 5) return CallbackResult::Done;
	return CallbackResult::Ok;
		});
	callbacks.add([](auto i) {
		if (i > 5) return CallbackResult::Done;
	return CallbackResult::Ok;
		});

	ASSERT_EQ(callbacks.size(), 4);

	auto result = callbacks(1);
	ASSERT_EQ(callbacks.size(), 4);
	ASSERT_EQ(result, CallbackResult::Ok);

	result = callbacks(2);
	ASSERT_EQ(callbacks.size(), 4);
	ASSERT_EQ(result, CallbackResult::Ok);

	result = callbacks(3);
	ASSERT_EQ(callbacks.size(), 3);
	ASSERT_EQ(result, CallbackResult::Done);

	result = callbacks(4);
	ASSERT_EQ(callbacks.size(), 3);
	ASSERT_EQ(result, CallbackResult::Ok);

	result = callbacks(5);
	ASSERT_EQ(callbacks.size(), 2);
	ASSERT_EQ(result, CallbackResult::Done);

	result = callbacks(6);
	ASSERT_EQ(callbacks.size(), 0);
	ASSERT_EQ(result, CallbackResult::Done);
}


TEST(TestCallbacks, testCleanableShortcutCallbacks) {

	CppSimConnect::CleanableShortcutCallbackList<int> callbacks;

	callbacks.add([](auto i) {
		if (i % 2 == 1) return CallbackResult::Abort;
		return CallbackResult::Ok;
		});
	callbacks.add([](auto i) {
		if (i > 2) return CallbackResult::AbortDone;
	return CallbackResult::Ok;
		});
	callbacks.add([](auto i) {
		if (i > 4) return CallbackResult::Done;
	return CallbackResult::Ok;
		});
	callbacks.add([](auto i) {
		if (i > 5) return CallbackResult::AbortDone;
	return CallbackResult::Ok;
		});
	callbacks.add([](auto i) {
		if (i > 5) return CallbackResult::Done;
	return CallbackResult::Ok;
		});

	ASSERT_EQ(callbacks.size(), 5);

	auto result = callbacks(1);
	ASSERT_EQ(callbacks.size(), 5);
	ASSERT_EQ(result, CallbackResult::Abort);

	result = callbacks(2);
	ASSERT_EQ(callbacks.size(), 5);
	ASSERT_EQ(result, CallbackResult::Ok);

	result = callbacks(3);
	ASSERT_EQ(callbacks.size(), 5);
	ASSERT_EQ(result, CallbackResult::Abort);

	result = callbacks(4);
	ASSERT_EQ(callbacks.size(), 4);
	ASSERT_EQ(result, CallbackResult::AbortDone);

	result = callbacks(5);
	ASSERT_EQ(callbacks.size(), 4);
	ASSERT_EQ(result, CallbackResult::Abort);

	result = callbacks(6);
	ASSERT_EQ(callbacks.size(), 2);
	ASSERT_EQ(result, CallbackResult::AbortDone);

	result = callbacks(6);
	ASSERT_EQ(callbacks.size(), 1);
	ASSERT_EQ(result, CallbackResult::Done);
}
