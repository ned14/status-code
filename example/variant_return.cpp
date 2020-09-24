/* Example use of status_code with outcome
(C) 2020 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Sept 2020


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
(See accompanying file Licence.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

/* There are many T or E variant return solutions available in the C++
ecosystem e.g. expected<T, E>, LEAF, Outcome. These allow the return
of a T, for success, or an E, for failure, from a function. This enables
a reasonable simulacrum of any future deterministic exceptions which
may get added to standard C++.

This example uses Experimental.Outcome which bundles an internal
copy of the system_error2 library. Outcome ships in a standalone
edition, and in Boost since v1.70. Both work well with C++ exceptions
globally disabled (though other bugs in Boost outside Outcome may
frustrate you if you use the Boost edition).

You can retrieve a single file integration of standalone Outcome
bundled with this library from:

https://raw.githubusercontent.com/ned14/outcome/develop/single-header/outcome-experimental.hpp

You may also wish to study the Outcome tutorial at:

https://ned14.github.io/outcome/tutorial/
*/
#include "outcome-experimental.hpp"

#include <iostream>
#include <string>

// Outcome's namespace permutes to avoid ABI collisions due to the unstable ABI.
namespace outcome_e = OUTCOME_V2_NAMESPACE::experimental;

/* Bind a local result<T> type from Outcome.Experimental's status_result<>.
status_result<> within Outcome is a specialised alias of outcome::basic_result<>
for E types which are system_error2's system_code. As a general rule, you don't
need to think much past just binding outcome_e::status_result into your local
namespace as your local result<T> type.
*/
template<class T> using result = outcome_e::status_result<T>;

result<std::string> moveMountain(bool input)
{
  if (input == true)
  {
    // Outcome's result will auto construct success from anything convertible to T (std::string)
    // If it's ambiguous which, use outcome_e::success(T) to disambiguate.
    return "worked";
  }
  else
  {
    // Types whitelisted as convertible to E cause auto construction of failure.
    // If it's ambiguous which, use outcome_e::failure(E) to disambiguate.
    return outcome_e::errc::no_such_file_or_directory;
  };
}

int main(int argc, char *argv[])
{
  if(auto r = moveMountain(argc > 1))
  {
    std::cout << r.value() << std::endl;;
  }
  else
  {
    // We need to use .c_str() because Outcome's pregenerated "outcome-experimental.hpp"
    // does not #include iostream nor iosfwd. If you want iostream integration,
    // #include "iostream_support.hpp" in this library.
    std::cerr << "error happened " << r.error().message().c_str() << std::endl;
  }
  return 0;
}
