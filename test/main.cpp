/* Proposed SG14 status_code testing
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Feb 2018


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

#include "iostream_support.hpp"
#include "system_error2.hpp"

#include <cstdio>
#include <cstring>  // for strlen
#include <memory>
#include <string>

#define CHECK(expr)                                                                                                                                                                                                                                                                                                            \
  if(!(expr))                                                                                                                                                                                                                                                                                                                  \
  {                                                                                                                                                                                                                                                                                                                            \
    fprintf(stderr, #expr " failed at line %d\n", __LINE__);                                                                                                                                                                                                                                                                   \
    retcode = 1;                                                                                                                                                                                                                                                                                                               \
  }

// An error coding with multiple success values
enum class Code : size_t
{
  success1,
  goaway,
  success2,
  error2
};
inline std::ostream &operator<<(std::ostream &s, Code v)
{
  return s << static_cast<size_t>(v);
}
class Code_domain_impl;
using StatusCode = system_error2::status_code<Code_domain_impl>;
// Category for Code
class Code_domain_impl : public system_error2::status_code_domain
{
  using _base = system_error2::status_code_domain;

public:
  using value_type = Code;

  // Custom string_ref using a shared_ptr
  class string_ref : public _base::string_ref
  {
  public:
    using shared_ptr_type = std::shared_ptr<std::string>;

  protected:
    virtual void _copy(_base::string_ref *dest) const & override final { new(static_cast<string_ref *>(dest)) string_ref(_begin, _end, _state); }
    virtual void _move(_base::string_ref *dest) && noexcept override final { new(static_cast<string_ref *>(dest)) string_ref(_begin, _end, _state); }
  public:
    string_ref() { new(reinterpret_cast<shared_ptr_type *>(this->_state)) shared_ptr_type(); }
    // Allow explicit cast up
    explicit string_ref(_base::string_ref v) { static_cast<string_ref &&>(v)._move(this); }
    // Construct from a C string literal, holding ref counted copy of string
    explicit string_ref(const char *str)
    {
      static_assert(sizeof(shared_ptr_type) <= sizeof(this->_state), "A shared_ptr does not fit into status_code's state");
      auto len = strlen(str);
      auto p = std::make_shared<std::string>(str, len);
      new(reinterpret_cast<shared_ptr_type *>(this->_state)) shared_ptr_type(p);
      this->_begin = p->data();
      this->_end = p->data() + p->size();
    }
    // Construct from a set of data
    string_ref(pointer begin, pointer end, void *const state[])
        : _base::string_ref(begin, end, nullptr, nullptr)
    {
      // Increase the ref count
      new(reinterpret_cast<shared_ptr_type *>(_state)) shared_ptr_type(*reinterpret_cast<const shared_ptr_type *>(state));
    }
    virtual ~string_ref()
    {
      // Decrease the ref count
      auto *p = reinterpret_cast<shared_ptr_type *>(_state);
      p->~shared_ptr_type();
    }
  };
  constexpr Code_domain_impl()
      : _base(0x430f120194fc06c7)
  {
  }
  static inline constexpr const Code_domain_impl *get();
  virtual _base::string_ref name() const noexcept override final
  {
    static string_ref v("Code_category_impl");
    return v;
  }
  virtual bool _failure(const system_error2::status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    return (static_cast<size_t>(static_cast<const StatusCode &>(code).value()) & 1) != 0;
  }
  virtual bool _equivalent(const system_error2::status_code<void> &code1, const system_error2::status_code<void> &code2) const noexcept override final
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const StatusCode &>(code1);
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const StatusCode &>(code2);
      return c1.value() == c2.value();
    }
    // If the other category is generic
    if(code2.domain() == system_error2::generic_code_domain)
    {
      const auto &c2 = static_cast<const system_error2::generic_code &>(code2);
      switch(c1.value())
      {
      case Code::success1:
      case Code::success2:
        return static_cast<system_error2::errc>(c2.value()) == system_error2::errc::success;
      case Code::goaway:
        switch(static_cast<system_error2::errc>(c2.value()))
        {
        case system_error2::errc::permission_denied:
        case system_error2::errc::operation_not_permitted:
          return true;
        default:
          return false;
        }
      }
    }
    return false;
  }
  virtual system_error2::generic_code _generic_code(const system_error2::status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c1 = static_cast<const StatusCode &>(code);
    switch(c1.value())
    {
    case Code::success1:
    case Code::success2:
      return system_error2::generic_code(system_error2::errc::success);
    case Code::goaway:
      return system_error2::generic_code(system_error2::errc::permission_denied);
    case Code::error2:
      return {};
    }
    return {};
  }
  virtual _base::string_ref _message(const system_error2::status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c1 = static_cast<const StatusCode &>(code);
    switch(c1.value())
    {
    case Code::success1:
    {
      static string_ref v("success1");
      return v;
    }
    case Code::goaway:
    {
      static string_ref v("goaway");
      return v;
    }
    case Code::success2:
    {
      static string_ref v("success2");
      return v;
    }
    case Code::error2:
    {
      static string_ref v("error2");
      return v;
    }
    }
    return string_ref{};
  }
  virtual void _throw_exception(const system_error2::status_code<void> &code) const override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const StatusCode &>(code);
    throw system_error2::status_error<Code_domain_impl>(c);
  }
};
constexpr Code_domain_impl Code_domain;
inline constexpr const Code_domain_impl *Code_domain_impl::get()
{
  return &Code_domain;
}


int main()
{
  using namespace SYSTEM_ERROR2_NAMESPACE;
  int retcode = 0;

  constexpr generic_code empty1, success1(errc::success), failure1(errc::permission_denied);
  CHECK(empty1.empty());
  CHECK(!success1.empty());
  CHECK(!failure1.empty());
  CHECK(success1.success());
  CHECK(failure1.failure());
  printf("generic_code empty has value %d (%s) is success %d is failure %d\n", empty1.value(), empty1.message().c_str(), empty1.success(), empty1.failure());
  printf("generic_code success has value %d (%s) is success %d is failure %d\n", success1.value(), success1.message().c_str(), success1.success(), success1.failure());
  printf("generic_code failure has value %d (%s) is success %d is failure %d\n", failure1.value(), failure1.message().c_str(), failure1.success(), failure1.failure());

  constexpr StatusCode empty2, success2(Code::success1), failure2(Code::goaway);
  CHECK(success2.success());
  CHECK(failure2.failure());
  printf("\nStatusCode empty has value %zu (%s) is success %d is failure %d\n", empty2.value(), empty2.message().c_str(), empty2.success(), empty2.failure());
  printf("StatusCode success has value %zu (%s) is success %d is failure %d\n", success2.value(), success2.message().c_str(), success2.success(), success2.failure());
  printf("StatusCode failure has value %zu (%s) is success %d is failure %d\n", failure2.value(), failure2.message().c_str(), failure2.success(), failure2.failure());

  printf("\n(empty1 == empty2) = %d\n", empty1 == empty2);        // True, empty ec's always compare equal no matter the type
  printf("(success1 == success2) = %d\n", success1 == success2);  // True, success maps onto success
  printf("(success1 == failure2) = %d\n", success1 == failure2);  // False, success does not map onto failure
  printf("(failure1 == success2) = %d\n", failure1 == success2);  // False, failure does not map onto success
  printf("(failure1 == failure2) = %d\n", failure1 == failure2);  // True, filename_too_long maps onto nospace
  CHECK(empty1 == empty2);
  CHECK(success1 == success2);
  CHECK(success1 != failure2);
  CHECK(failure1 != success2);
  CHECK(failure1 == failure2);


  // Test status code erasure
  status_code<erased<int>> success3(success1), failure3(failure1);
  CHECK(success3.success());
  CHECK(success3.domain() == success1.domain());
  CHECK(failure3.failure());
  CHECK(failure3.domain() == failure1.domain());
  printf("\nerased<int> success has value %d (%s) is success %d is failure %d\n", success3.value(), success3.message().c_str(), success3.success(), success3.failure());
  printf("erased<int> failure has value %d (%s) is success %d is failure %d\n", failure3.value(), failure3.message().c_str(), failure3.success(), failure3.failure());
  generic_code success4(success3), failure4(failure3);
  CHECK(success4.value() == success1.value());
  CHECK(success4.domain() == success1.domain());
  CHECK(failure4.value() == failure1.value());
  CHECK(failure4.domain() == failure1.domain());

  // ostream printers
  std::cout << "\ngeneric_code failure: " << failure1 << std::endl;
  std::cout << "StatusCode failure: " << failure2 << std::endl;
  std::cout << "erased<int> failure: " << failure3 << std::endl;

#ifdef _WIN32
  // Test win32_code
  constexpr win32_code success5(0 /*ERROR_SUCCESS*/), failure5(0x5 /*ERROR_ACCESS_DENIED*/);
  CHECK(success5.success());
  CHECK(failure5.failure());
  printf("\nWin32 code success has value %zu (%s) is success %d is failure %d\n", success5.value(), success5.message().c_str(), success5.success(), success5.failure());
  printf("Win32 code failure has value %zu (%s) is success %d is failure %d\n", failure5.value(), failure5.message().c_str(), failure5.success(), failure5.failure());
  CHECK(success5 == errc::success);
  CHECK(failure5 == errc::permission_denied);
  CHECK(failure5 == failure1);
  CHECK(failure5 == failure2);
  system_code success6(success5), failure6(failure5);
  CHECK(success6 == errc::success);
  CHECK(failure6 == errc::permission_denied);
  CHECK(failure6 == failure1);
  CHECK(failure6 == failure2);

  // Test nt_code
  constexpr nt_code success7(1 /* positive */), failure7(0xC0000022 /*STATUS_ACCESS_DENIED*/);
  CHECK(success7.success());
  CHECK(failure7.failure());
  printf("\nNT code success has value %zu (%s) is success %d is failure %d\n", success7.value(), success7.message().c_str(), success7.success(), success7.failure());
  printf("NT code warning has value %zu (%s) is success %d is failure %d\n", failure7.value(), failure7.message().c_str(), failure7.success(), failure7.failure());
  CHECK(success7 == errc::success);
  CHECK(failure7 == errc::permission_denied);
  CHECK(failure7 == failure1);
  CHECK(failure7 == failure2);
  CHECK(failure7 == failure5);
  system_code success8(success7), failure8(failure7);
  CHECK(success8 == errc::success);
  CHECK(failure8 == errc::permission_denied);
  CHECK(failure8 == failure1);
  CHECK(failure8 == failure2);
  CHECK(failure8 == failure5);
#endif

  // Test posix_code
  constexpr posix_code success9(0), failure9(EACCES);
  CHECK(success9.success());
  CHECK(failure9.failure());
  printf("\nPOSIX code success has value %zu (%s) is success %d is failure %d\n", success9.value(), success9.message().c_str(), success9.success(), success9.failure());
  printf("POSIX code failure has value %zu (%s) is success %d is failure %d\n", failure9.value(), failure9.message().c_str(), failure9.success(), failure9.failure());
  CHECK(success9 == errc::success);
  CHECK(failure9 == errc::permission_denied);
  CHECK(failure9 == failure1);
  CHECK(failure9 == failure2);
  system_code success10(success9), failure10(failure9);
  CHECK(success10 == errc::success);
  CHECK(failure10 == errc::permission_denied);
  CHECK(failure10 == failure1);
  CHECK(failure10 == failure2);

  return retcode;
}