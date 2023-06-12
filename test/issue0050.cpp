/* Regression testing
(C) 2022 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Oct 2022


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

#include "status-code/iostream_support.hpp"
#include "status-code/nested_status_code.hpp"
#include "status-code/system_error2.hpp"

#include <cstdio>
#include <iostream>

#define CHECK(expr)                                                                                                                                            \
  if(!(expr))                                                                                                                                                  \
  {                                                                                                                                                            \
    fprintf(stderr, #expr " failed at line %d\n", __LINE__);                                                                                                   \
    retcode = 1;                                                                                                                                               \
  }

int main()
{
  using namespace SYSTEM_ERROR2_NAMESPACE;
  int retcode = 0;
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  auto do_evil = [](posix_code c) -> system_code
  {
    auto p = make_nested_status_code(c);
    system_code c1 = std::move(p);
    return p;  // moved from, so empty
  };
  auto sc = do_evil(posix_code(int(std::errc::bad_file_descriptor)));
  auto msg = sc.message();
  std::cout << msg << std::endl;
  CHECK(strstr(msg.c_str(), "(empty)") != nullptr);
  return retcode;
}