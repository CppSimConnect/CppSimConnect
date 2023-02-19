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

#include "pch.h"

#include "../CppSimConnect/reactive/MessageObserver.h"
#include "../CppSimConnect/reactive/MessageResult.h"
#include "../CppSimConnect/reactive/StreamResult.h"


using CppSimConnect::Reactive::MessageResult;

TEST(TestMessageResult, testNoListeners) {
	MessageResult<std::string> messageObserver;

	ASSERT_FALSE(messageObserver.completed()) << "A new observer is not completed.\n";

	messageObserver.onNext("Hi there!");
	ASSERT_TRUE(messageObserver.completed()) << "Sending a single message should complete a MessageResult\n";
}

TEST(TestMessageResult, testOnNext) {
	MessageResult<std::string> messageObserver;

	bool haveMessage{ false };
	messageObserver.withOnNext([&haveMessage](std::string msg) { std::cerr << "Received '" << msg << "'\n"; haveMessage = true; });

	ASSERT_FALSE(messageObserver.completed()) << "A new observer is not completed.\n";
	ASSERT_FALSE(haveMessage) << "Initially we have no message.\n";

	messageObserver.onNext("Hi there!");
	ASSERT_TRUE(haveMessage) << "After sending a message, we should have registered it.\n";
	ASSERT_TRUE(messageObserver.completed()) << "Sending a single message should complete a MessageResult\n";
}

TEST(TestMessageResult, testOnCompleted) {
	MessageResult<std::string> messageObserver;

	bool haveMessage{ false }, areDone{ false };
	messageObserver
		.withOnNext([&haveMessage](std::string msg) { std::cout << "Received '" << msg << "'\n"; haveMessage = true; })
		.withOnComplete([&areDone]() { std::cout << "Done!\n"; areDone = true; });

	ASSERT_FALSE(messageObserver.completed()) << "A new observer is not completed.\n";
	ASSERT_FALSE(haveMessage) << "Initially we have no message.\n";
	ASSERT_FALSE(areDone) << "Initially we are not done.\n";

	messageObserver.onNext("Hi there!");
	ASSERT_TRUE(haveMessage) << "After sending a message, we should have registered it.\n";
	ASSERT_TRUE(areDone) << "After sending a message, we should be done.\n";
	ASSERT_TRUE(messageObserver.completed()) << "Sending a single message should complete a MessageResult\n";
}

class TestError : public std::exception {
	std::string _msg;
public:
	TestError(std::string msg) : _msg(msg) {};
	~TestError() = default;
	TestError(TestError&&) = default;
	TestError(const TestError&) = default;
	TestError& operator=(TestError&&) = default;
	TestError& operator=(const TestError&) = default;

	virtual const char* what() const override { return _msg.c_str(); }
};

TEST(TestMessageResult, testOnError) {
	MessageResult<std::string> messageObserver;

	bool haveMessage{ false }, areDone{ false };
	std::string error;

	messageObserver
		.withOnNext([&haveMessage](std::string msg) { std::cout << "Received '" << msg << "'\n"; haveMessage = true; throw TestError("HELP!"); })
		.withOnComplete([&areDone]() { std::cout << "Done!\n"; areDone = true; })
		.withOnError([&error](auto ptr) { std::cout << "Error!\n"; error = CppSimConnect::Reactive::errorFromPtr<TestError>(ptr).what(); });

	ASSERT_FALSE(messageObserver.completed()) << "A new observer is not completed.\n";
	ASSERT_FALSE(haveMessage) << "Initially we have no message.\n";
	ASSERT_FALSE(areDone) << "Initially we are not done.\n";
	ASSERT_EQ(error, "") << "Initially no error\n.";

	messageObserver.onNext("Hi there!");
	ASSERT_TRUE(haveMessage) << "After sending a message, we should have registered it.\n";
	ASSERT_TRUE(areDone) << "After sending a message, we should be done.\n";
	ASSERT_TRUE(messageObserver.completed()) << "Sending a single message should complete a MessageResult\n";
	ASSERT_NE(error, "") << "We should have an error now.\n";
	//const char* errMesg = messageObserver.error<TestError>().what();
	ASSERT_EQ(error, std::string("HELP!")) << "The error should be \"HELP!\".\n";
}
