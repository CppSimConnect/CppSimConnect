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

#include <iostream>

#include "../CppSimConnect/reactive/MessageObserver.h"

#include <rpp/rpp.hpp>


void run1() {
	CppSimConnect::Reactive::MessageObserver<std::string, std::string> observer;

	observer.withOnNext([](auto msg) { std::cerr << "Message: '" << msg << "'\n"; });
	observer.withOnError([](auto err) { std::cerr << "Error: '" << err << "'\n"; });
	observer.withOnComplete([]() { std::cerr << "Completed!\n"; });

	std::cerr << "Run 1: count to three and complete.\n";
	observer.onNext("1");
	observer.onNext("2");
	observer.onNext("3");
	observer.onCompleted();

	observer.onNext("4");		// should be ignored
	observer.onCompleted();		// should be ignored
	observer.onError("error");	// should be ignored
}

void run2() {
	CppSimConnect::Reactive::MessageObserver<std::string, std::string> observer;

	observer.withOnNext([](auto msg) { std::cerr << "Message: '" << msg << "'\n"; });
	observer.withOnError([](auto err) { std::cerr << "Error: '" << err << "'\n"; });
	observer.withOnComplete([]() { std::cerr << "Completed!\n"; });

	std::cerr << "\n\nRun 2: count to three and return.\n";
	observer.onNext("1");
	observer.onNext("2");
	observer.onNext("3");
}

void run3() {
	CppSimConnect::Reactive::MessageObserver<std::string, std::string> observer;

	observer.withOnNext([](auto msg) { std::cerr << "Message: '" << msg << "'\n"; });
	observer.withOnError([](auto err) { std::cerr << "Error: '" << err << "'\n"; });
	observer.withOnComplete([]() { std::cerr << "Completed!\n"; });

	std::cerr << "\n\nRun 3: count to three and error.\n";
	observer.onNext("1");
	observer.onNext("2");
	observer.onNext("3");
	observer.onError("error");

	observer.onNext("4");		// should be ignored
	observer.onCompleted();		// should be ignored
	observer.onError("error2");	// should be ignored
}

void run4() {
	rpp::subjects::publish_subject<std::string> s;
	s.get_observable().subscribe([](auto msg) { std::cerr << "Message: '" << msg << "'\n"; }, [](auto err) { std::cerr << "Error:\n"; }, []() { std::cerr << "Completed!\n"; });

	std::cerr << "\n\nRun 4: ReactivePlusPlus\n";

	auto subscriber = s.get_subscriber();
	subscriber.on_next("1");
	subscriber.on_next("2");
	subscriber.on_next("3");
	subscriber.on_completed();

	subscriber.on_next("4");		// should be ignored
	subscriber.on_completed();		// should be ignored
	subscriber.on_error(std::make_exception_ptr(std::string("error")));
}

int main(int argc, char* argv[])
{
	run1();
	run2();
	run3();
	run4();
}