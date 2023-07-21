/* Regression testing
(C) 2023 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
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

#define DONT_DEFINE_MAIN 1
#include "../example/thrown_exception.cpp"

#define CHECK(expr)                                                                                                                                            \
  if(!(expr))                                                                                                                                                  \
  {                                                                                                                                                            \
    fprintf(stderr, #expr " failed at line %d\n", __LINE__);                                                                                                   \
    retcode = 1;                                                                                                                                               \
  }

void throw_something()
{
  throw std::logic_error("oops");
}

SYSTEM_ERROR2_NAMESPACE::system_code produce_error()
{
  using namespace SYSTEM_ERROR2_NAMESPACE;
  try
  {
    throw_something();
  }
  catch(std::exception &e)
  {
    printf("Exception has message %s\n", e.what());
    auto eptr = std::current_exception();
    thrown_exception_code tec = make_status_code(eptr);
    return tec;
  }
  return errc::success;
}

int main()
{
  int retcode = 0;
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  using namespace SYSTEM_ERROR2_NAMESPACE;
  auto sc = produce_error();
  auto msg = sc.message();
  CHECK(strcmp(msg.c_str(), "oops") == 0);
  printf("Thrown exception code has message %s\n", sc.message().c_str());
  return retcode;
}
