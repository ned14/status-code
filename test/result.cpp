/* Proposed SG14 status_code testing
(C) 2015-2020 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Jan 2020


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

#include "status-code/result.hpp"

#if(__cplusplus >= 201703L || _HAS_CXX17) && __has_include(<variant>)

#include <cstdio>
#include <iostream>
#include <string>

/* Most of this test suite was ported over from Boost.Outcome's
experimental-core-result-status.cpp
*/

#define BOOST_CHECK(expr)                                                                                                                                                                                                                                                                                                      \
  if(!(expr))                                                                                                                                                                                                                                                                                                                  \
  {                                                                                                                                                                                                                                                                                                                            \
    fprintf(stderr, #expr " failed at line %d\n", __LINE__);                                                                                                                                                                                                                                                                   \
    retcode = 1;                                                                                                                                                                                                                                                                                                               \
  }
#define BOOST_CHECK_THROW(expr, etype)                                                                                                                                                                                                                                                                                         \
  try                                                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    expr;                                                                                                                                                                                                                                                                                                                      \
    fprintf(stderr, #expr " failed to throw " #etype " at line %d\n", __LINE__);                                                                                                                                                                                                                                               \
    retcode = 1;                                                                                                                                                                                                                                                                                                               \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(etype)                                                                                                                                                                                                                                                                                                                 \
  {                                                                                                                                                                                                                                                                                                                            \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(...)                                                                                                                                                                                                                                                                                                                   \
  {                                                                                                                                                                                                                                                                                                                            \
    fprintf(stderr, #expr " failed to throw " #etype " at line %d\n", __LINE__);                                                                                                                                                                                                                                               \
    retcode = 1;                                                                                                                                                                                                                                                                                                               \
  }
#define BOOST_CHECK_NO_THROW(expr)                                                                                                                                                                                                                                                                                             \
  try                                                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    expr;                                                                                                                                                                                                                                                                                                                      \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(...)                                                                                                                                                                                                                                                                                                                   \
  {                                                                                                                                                                                                                                                                                                                            \
    fprintf(stderr, #expr " failed due to throw at line %d\n", __LINE__);                                                                                                                                                                                                                                                      \
    retcode = 1;                                                                                                                                                                                                                                                                                                               \
  }

int main()
{
  using namespace SYSTEM_ERROR2_NAMESPACE;
  int retcode = 0;

  {  // errored int
    result<int> m(generic_code{errc::bad_address});
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), generic_error);
    BOOST_CHECK_NO_THROW(m.error());
  }
  {  // errored void
    result<void> m(generic_code{errc::bad_address});
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    // BOOST_CHECK_THROW(([&m]() -> void { return m.value(); }()), generic_error);
    BOOST_CHECK_NO_THROW(m.error());
  }
  {  // valued int
    result<int> m(5);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == 5);
    m.value() = 6;
    BOOST_CHECK(m.value() == 6);
  }
  {  // valued bool
    result<bool> m(false);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == false);
    m.value() = true;
    BOOST_CHECK(m.value() == true);
  }
  {  // moves do not clear state
    result<std::string> m("niall");
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == "niall");
    m.value() = "NIALL";
    BOOST_CHECK(m.value() == "NIALL");
    auto temp(std::move(m).value());
    BOOST_CHECK(temp == "NIALL");
    BOOST_CHECK(m.value().empty());  // NOLINT
  }
  {  // valued void
    result<void> m(in_place_type<void>);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_NO_THROW(m.value());  // works, but type returned is unusable
  }
  {  // errored
    error ec(errc::no_link);
    result<int> m(ec.clone());
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), generic_error);
    BOOST_CHECK(m.error() == ec);
  }
  //  if(false)  // NOLINT
  //  {          // void, void is permitted, but is not constructible
  //    result<void, void> *m = nullptr;
  //    m->value();
  //    m->error();
  //  }

  {
    // Deliberately define non-trivial operations
    struct udt
    {
      int _v{0};
      udt() = default;
      udt(udt &&o) noexcept
          : _v(o._v)
      {
      }
      udt(const udt &o)  // NOLINT
          : _v(o._v)
      {
      }
      udt &operator=(udt &&o) noexcept
      {
        _v = o._v;
        return *this;
      }
      udt &operator=(const udt &o)  // NOLINT
      {
        _v = o._v;
        return *this;
      }
      ~udt() { _v = 0; }
    };
    // No default construction, no copy nor move
    struct udt2
    {
      udt2() = delete;
      udt2(udt2 &&) = delete;
      udt2(const udt2 &) = delete;
      udt2 &operator=(udt2 &&) = delete;
      udt2 &operator=(const udt2 &) = delete;
      explicit udt2(int /*unused*/) {}
      ~udt2() = default;
    };
    // Can only be constructed via multiple args
    struct udt3
    {
      udt3() = delete;
      udt3(udt3 &&) = delete;
      udt3(const udt3 &) = delete;
      udt3 &operator=(udt3 &&) = delete;
      udt3 &operator=(const udt3 &) = delete;
      explicit udt3(int /*unused*/, const char * /*unused*/, std::nullptr_t /*unused*/) {}
      ~udt3() = default;
    };

    result<int> a(5);
    result<int> b(generic_code{errc::invalid_argument});
    std::cout << sizeof(a) << std::endl;  // 32 bytes
    if(false)                             // NOLINT
    {
      b.assume_value();
      a.assume_error();
    }
#ifdef __cpp_exceptions
    try
    {
      b.value();
      std::cerr << "fail" << std::endl;
      std::terminate();
    }
    catch(const generic_error &e)
    {
      BOOST_CHECK(!strcmp(e.what(), b.error().message().c_str()));
    }
#endif
    static_assert(!std::is_default_constructible<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(a)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_trivially_destructible<decltype(a)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(a)>::value, "");

    // Test void compiles
    result<void> c(in_place_type<void>);

    // Test a standard udt compiles
    result<udt> d(in_place_type<udt>);
    static_assert(!std::is_default_constructible<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(d)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(d)>::value, "");
    static_assert(std::is_move_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_move_assignable<decltype(d)>::value, "");
    static_assert(std::is_nothrow_move_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_destructible<decltype(d)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(d)>::value, "");

    // Test a highly pathological udt compiles
    result<udt2> e(in_place_type<udt2>, 5);
    // result<udt2> e2(e);
    static_assert(!std::is_default_constructible<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(e)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_move_assignable<decltype(e)>::value, "");
    static_assert(!std::is_trivially_move_assignable<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_move_assignable<decltype(e)>::value, "");

    // Test a udt which can only be constructed in place compiles
    result<udt3> g(in_place_type<udt3>, 5, static_cast<const char *>("niall"), nullptr);
    // Does converting inplace construction also work?
    result<udt3> h(5, static_cast<const char *>("niall"), nullptr);
    result<udt3> i(generic_code{errc::not_enough_memory});
    BOOST_CHECK(h.has_value());
    BOOST_CHECK(i.has_error());
  }

  // Test direct use of error code enum works
  {
    /*constexpr*/ result<int> a(5), b(errc::invalid_argument);
    BOOST_CHECK(a.value() == 5);
    BOOST_CHECK(b.error() == errc::invalid_argument);
  }

#if 0
#ifdef __cpp_exceptions
  // Test payload facility
  {
    const char *niall = "niall";
    result<int, status_code_payload> b{payload{errc::invalid_argument, niall}};
    try
    {
      b.value();
      BOOST_CHECK(false);
    }
    catch(const payload_exception &e)
    {
      BOOST_CHECK(!strcmp(e.what(), niall));
    }
    catch(...)
    {
      BOOST_CHECK(false);
    }
  }
#endif
#endif

  return retcode;
}

#else
int main(void)
{
  return 0;
}
#endif
