/* Proposed SG14 status_code testing
(C) 2015-2020 Niall Douglas <http://www.nedproductions.biz/>
(C) 2023 Henrik Steffen Gaßmann
File Created: Apr 2023


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

#include "status-code/fmt_support.hpp"
#include "status-code/generic_code.hpp"

#if __cpp_lib_format >= 202106L

#include <cstdio>
#include <string>

/* Most of this test suite was ported over from Boost.Outcome's
experimental-core-result-status.cpp
*/

#define BOOST_CHECK(expr)                                                                                                                                      \
  if(!(expr))                                                                                                                                                  \
  {                                                                                                                                                            \
    fprintf(stderr, #expr " failed at line %d\n", __LINE__);                                                                                                   \
    retcode = 1;                                                                                                                                               \
  }
#define BOOST_CHECK_THROW(expr, etype)                                                                                                                         \
  try                                                                                                                                                          \
  {                                                                                                                                                            \
    expr;                                                                                                                                                      \
    fprintf(stderr, #expr " failed to throw " #etype " at line %d\n", __LINE__);                                                                               \
    retcode = 1;                                                                                                                                               \
  }                                                                                                                                                            \
  catch(etype)                                                                                                                                                 \
  {                                                                                                                                                            \
  }                                                                                                                                                            \
  catch(...)                                                                                                                                                   \
  {                                                                                                                                                            \
    fprintf(stderr, #expr " failed to throw " #etype " at line %d\n", __LINE__);                                                                               \
    retcode = 1;                                                                                                                                               \
  }
#define BOOST_CHECK_NO_THROW(expr)                                                                                                                             \
  try                                                                                                                                                          \
  {                                                                                                                                                            \
    expr;                                                                                                                                                      \
  }                                                                                                                                                            \
  catch(...)                                                                                                                                                   \
  {                                                                                                                                                            \
    fprintf(stderr, #expr " failed due to throw at line %d\n", __LINE__);                                                                                      \
    retcode = 1;                                                                                                                                               \
  }

int main()
{
  using namespace std::string_view_literals;
  using namespace SYSTEM_ERROR2_NAMESPACE;
  int retcode = 0;

  {  // format a string_ref
    generic_code c{errc::bad_address};
    BOOST_CHECK(std::format("{}", c.message()) == "Bad address"sv);
  }

  return retcode;
}

#else
int main()
{
  return 0;
}
#endif
