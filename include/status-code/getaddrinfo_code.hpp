/* Proposed SG14 status_code
(C) 2020 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
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

#ifndef SYSTEM_ERROR2_GETADDRINFO_CODE_HPP
#define SYSTEM_ERROR2_GETADDRINFO_CODE_HPP

#include "quick_status_code_from_enum.hpp"

#ifdef _WIN32
#error Not available for Microsoft Windows
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

SYSTEM_ERROR2_NAMESPACE_BEGIN

class _getaddrinfo_code_domain;
//! A getaddrinfo error code, those returned by `getaddrinfo()`.
using getaddrinfo_code = status_code<_getaddrinfo_code_domain>;
//! A specialisation of `status_error` for the `getaddrinfo()` error code domain.
using getaddrinfo_error = status_error<_getaddrinfo_code_domain>;

/*! The implementation of the domain for `getaddrinfo()` error codes, those returned by `getaddrinfo()`.
 */
class _getaddrinfo_code_domain : public status_code_domain
{
  template <class DomainType> friend class status_code;
  using _base = status_code_domain;

public:
  //! The value type of the `getaddrinfo()` code, which is an `int`
  using value_type = int;
  using _base::string_ref;

  //! Default constructor
  constexpr explicit _getaddrinfo_code_domain(typename _base::unique_id_type id = 0x5b24b2de470ff7b6) noexcept
      : _base(id)
  {
  }
  _getaddrinfo_code_domain(const _getaddrinfo_code_domain &) = default;
  _getaddrinfo_code_domain(_getaddrinfo_code_domain &&) = default;
  _getaddrinfo_code_domain &operator=(const _getaddrinfo_code_domain &) = default;
  _getaddrinfo_code_domain &operator=(_getaddrinfo_code_domain &&) = default;
  ~_getaddrinfo_code_domain() = default;

  //! Constexpr singleton getter. Returns constexpr getaddrinfo_code_domain variable.
  static inline constexpr const _getaddrinfo_code_domain &get();

protected:
  virtual int _do_name(_vtable_name_args &args) const noexcept override
  {
    args.ret = string_ref("getaddrinfo() domain");
    return 0;
  }  // NOLINT
  virtual void _do_payload_info(_vtable_payload_info_args &args) const noexcept override
  {
    args.ret = {sizeof(value_type), sizeof(status_code_domain *) + sizeof(value_type),
                (alignof(value_type) > alignof(status_code_domain *)) ? alignof(value_type) :
                                                                        alignof(status_code_domain *)};
  }
  virtual bool _do_failure(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);                                   // NOLINT
    return static_cast<const getaddrinfo_code &>(code).value() != 0;  // NOLINT
  }
  virtual bool _do_equivalent(const status_code<void> &code1,
                              const status_code<void> &code2) const noexcept override  // NOLINT
  {
    assert(code1.domain() == *this);                                // NOLINT
    const auto &c1 = static_cast<const getaddrinfo_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const getaddrinfo_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    return false;
  }
  virtual void _do_generic_code(_vtable_generic_code_args &args) const noexcept override
  {
    assert(args.code.domain() == *this);                               // NOLINT
    const auto &c = static_cast<const getaddrinfo_code &>(args.code);  // NOLINT
    switch(c.value())
    {
#ifdef EAI_ADDRFAMILY
    case EAI_ADDRFAMILY:
      args.ret = errc::no_such_device_or_address;
      return;
#endif
    case EAI_FAIL:
      args.ret = errc::io_error;
      return;
    case EAI_MEMORY:
      args.ret = errc::not_enough_memory;
      return;
#ifdef EAI_NODATA
    case EAI_NODATA:
      args.ret = errc::no_such_device_or_address;
      return;
#endif
    case EAI_NONAME:
      args.ret = errc::no_such_device_or_address;
      return;
#ifdef EAI_OVERFLOW
    case EAI_OVERFLOW:
      args.ret = errc::argument_list_too_long;
      return;
#endif
    case EAI_BADFLAGS:  // fallthrough
    case EAI_SERVICE:
      args.ret = errc::invalid_argument;
      return;
    case EAI_FAMILY:  // fallthrough
    case EAI_SOCKTYPE:
      args.ret = errc::operation_not_supported;
      return;
    case EAI_AGAIN:  // fallthrough
    case EAI_SYSTEM:
      args.ret = errc::resource_unavailable_try_again;
      return;
    default:
      args.ret = errc::unknown;
      return;
    }
  }
  virtual int _do_message(_vtable_message_args &args) const noexcept override
  {
    assert(args.code.domain() == *this);                               // NOLINT
    const auto &c = static_cast<const getaddrinfo_code &>(args.code);  // NOLINT
    args.ret = string_ref(gai_strerror(c.value()));
    return 0;
  }
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(STANDARDESE_IS_IN_THE_HOUSE)
  SYSTEM_ERROR2_NORETURN virtual void _do_throw_exception(const status_code<void> &code) const override  // NOLINT
  {
    assert(code.domain() == *this);                               // NOLINT
    const auto &c = static_cast<const getaddrinfo_code &>(code);  // NOLINT
    throw status_error<_getaddrinfo_code_domain>(c);
  }
#endif
};
//! A constexpr source variable for the `getaddrinfo()` code domain, which is that of `getaddrinfo()`. Returned by
//! `_getaddrinfo_code_domain::get()`.
constexpr _getaddrinfo_code_domain getaddrinfo_code_domain;
inline constexpr const _getaddrinfo_code_domain &_getaddrinfo_code_domain::get()
{
  return getaddrinfo_code_domain;
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
