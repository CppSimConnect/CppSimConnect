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

#include "../pch.h"

#include "ClientEvent.h"


using CppSimConnect::ClientEvent;

/*
std::mutex ClientEvent::_eventLock;
ClientEvent::EventID ClientEvent::_nextID{ 0 };
std::vector<std::string> ClientEvent::_names;
std::map<std::string, ClientEvent::EventID> ClientEvent::_ids;
*/