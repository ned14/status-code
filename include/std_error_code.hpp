/* Proposed SG14 status_code
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Aug 2018


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

#ifndef SYSTEM_ERROR2_STD_ERROR_CODE_HPP
#define SYSTEM_ERROR2_STD_ERROR_CODE_HPP

#include "posix_code.hpp"

#if defined(_WIN32) || defined(STANDARDESE_IS_IN_THE_HOUSE)
#include "win32_code.hpp"
#endif

#include <system_error>

SYSTEM_ERROR2_NAMESPACE_BEGIN

template <class error_code_type> class _error_code_domain;
//! A wrapper of `std::error_code`.
using std_error_code = status_code<_error_code_domain<std::error_code>>;

/*! The implementation of the domain for `std::error_code` error codes.
*/
template <class error_code_type> class _error_code_domain : public status_code_domain
{
  template <class DomainType> friend class status_code;
  using _base = status_code_domain;
  using _status_code = status_code<_error_code_domain>;

  static _base::string_ref _make_string_ref(error_code_type c) noexcept
  {
    try
    {
      std::string msg = c.message();
      auto *p = static_cast<char *>(malloc(msg.size() + 1));  // NOLINT
      if(p == nullptr)
      {
        return _base::string_ref("failed to allocate message");
      }
      memcpy(p, msg.c_str(), msg.size() + 1);
      return _base::atomic_refcounted_string_ref(p, msg.size());
    }
    catch(...)
    {
      return _base::string_ref("failed to allocate message");
    }
  }

public:
  //! The value type of the `std::error_code` code, which is the `std::error_code`
  using value_type = error_code_type;
  using _base::string_ref;

  //! Default constructor
  constexpr _error_code_domain() noexcept : _base(0x223a160d20de97b4) {}
  _error_code_domain(const _error_code_domain &) = default;
  _error_code_domain(_error_code_domain &&) = default;
  _error_code_domain &operator=(const _error_code_domain &) = default;
  _error_code_domain &operator=(_error_code_domain &&) = default;
  ~_error_code_domain() = default;

  //! Constexpr singleton getter. Returns constexpr error_code_domain variable.
  static inline constexpr const _error_code_domain &get();

  virtual string_ref name() const noexcept override { return string_ref("error_code compatibility domain"); }  // NOLINT
protected:
  virtual bool _do_failure(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    return static_cast<const _status_code &>(code).value().value() != 0;  // NOLINT
  }
  virtual bool _do_equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final  // NOLINT
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const _status_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const _status_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    if(code2.domain() == generic_code_domain)
    {
      const auto &c2 = static_cast<const generic_code &>(code2);  // NOLINT
      if(static_cast<int>(c2.value()) == static_cast<int>(c1.value().default_error_condition().value()))
      {
        return true;
      }
    }
    return false;
  }
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const _status_code &>(code);  // NOLINT
    return generic_code(static_cast<errc>(c.value().default_error_condition().value()));
  }
  virtual string_ref _do_message(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const _status_code &>(code);  // NOLINT
    return _make_string_ref(c.value());
  }
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(STANDARDESE_IS_IN_THE_HOUSE)
  SYSTEM_ERROR2_NORETURN virtual void _do_throw_exception(const status_code<void> &code) const override  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const _status_code &>(code);  // NOLINT
    throw std::system_error(c.value());
  }
#endif
};
//! A constexpr source variable for the `std::error_code` code domain. Returned by `_error_code_domain<error_code_type>::get()`.
constexpr _error_code_domain<std::error_code> std_error_code_domain;
template <class error_code_type> inline constexpr const _error_code_domain<error_code_type> &_error_code_domain<error_code_type>::get()
{
  return std_error_code_domain;
}
// Enable implicit construction of `std_error_code` from `std::error_code`.
inline std_error_code make_status_code(std::error_code c) noexcept
{
  return std_error_code(in_place, c);
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
