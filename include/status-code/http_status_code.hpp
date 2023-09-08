/* Proposed SG14 status_code
(C) 2022 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Jun 2022


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

#ifndef SYSTEM_ERROR2_HTTP_STATUS_CODE_HPP
#define SYSTEM_ERROR2_HTTP_STATUS_CODE_HPP

#include "status_code.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

class _http_status_code_domain;
//! A HTTP status code.
using http_status_code = status_code<_http_status_code_domain>;

namespace mixins
{
  template <class Base> struct mixin<Base, _http_status_code_domain> : public Base
  {
    using Base::Base;

    //! True if the HTTP status code is informational
    inline bool is_http_informational() const noexcept;
    //! True if the HTTP status code is successful
    inline bool is_http_success() const noexcept;
    //! True if the HTTP status code is redirection
    inline bool is_http_redirection() const noexcept;
    //! True if the HTTP status code is client error
    inline bool is_http_client_error() const noexcept;
    //! True if the HTTP status code is server error
    inline bool is_http_server_error() const noexcept;
  };
}  // namespace mixins

/*! The implementation of the domain for HTTP status codes.
 */
class _http_status_code_domain : public status_code_domain
{
  template <class DomainType> friend class status_code;
  using _base = status_code_domain;

public:
  //! The value type of the HTTP code, which is an `int`
  using value_type = int;
  using _base::string_ref;

  //! Default constructor
  constexpr explicit _http_status_code_domain(typename _base::unique_id_type id = 0xbdb4cde88378a333ull) noexcept
      : _base(id)
  {
  }
  _http_status_code_domain(const _http_status_code_domain &) = default;
  _http_status_code_domain(_http_status_code_domain &&) = default;
  _http_status_code_domain &operator=(const _http_status_code_domain &) = default;
  _http_status_code_domain &operator=(_http_status_code_domain &&) = default;
  ~_http_status_code_domain() = default;

  //! Constexpr singleton getter. Returns constexpr http_status_code_domain variable.
  static inline constexpr const _http_status_code_domain &get();

  virtual string_ref name() const noexcept override { return string_ref("HTTP status domain"); }  // NOLINT

  virtual payload_info_t payload_info() const noexcept override
  {
    return {sizeof(value_type), sizeof(status_code_domain *) + sizeof(value_type),
            (alignof(value_type) > alignof(status_code_domain *)) ? alignof(value_type) : alignof(status_code_domain *)};
  }

protected:
  virtual bool _do_failure(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);                                     // NOLINT
    return static_cast<const http_status_code &>(code).value() >= 400;  // NOLINT
  }
  virtual bool _do_equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override  // NOLINT
  {
    assert(code1.domain() == *this);                                // NOLINT
    const auto &c1 = static_cast<const http_status_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const http_status_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    return false;
  }
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);                               // NOLINT
    const auto &c = static_cast<const http_status_code &>(code);  // NOLINT
    switch(c.value())
    {
    case 102:
    case 202:
      return errc::operation_in_progress;
    case 400:
      return errc::invalid_argument;
    case 401:
      return errc::operation_not_permitted;
    case 403:
      return errc::permission_denied;
    case 404:
    case 410:
      return errc::no_such_file_or_directory;
    case 405:
    case 418:
      return errc::operation_not_supported;
    case 406:
      return errc::protocol_not_supported;
    case 408:
      return errc::timed_out;
    case 413:
      return errc::result_out_of_range;
    case 501:
      return errc::not_supported;
    case 503:
      return errc::resource_unavailable_try_again;
    case 504:
      return errc::timed_out;
    case 507:
      return errc::no_space_on_device;
    default:
      return errc::unknown;
    }
  }
  virtual string_ref _do_message(const status_code<void> &code) const noexcept override  // NOLINT
  {
    assert(code.domain() == *this);                               // NOLINT
    const auto &c = static_cast<const http_status_code &>(code);  // NOLINT
    return string_ref(
    [&]() -> const char *
    {
      switch(c.value())
      {
      case 100:
        return "Continue";
      case 101:
        return "Switching Protocols";
      case 102:
        return "Processing";
      case 103:
        return "Early Hints";
      case 200:
        return "OK";
      case 201:
        return "Created";
      case 202:
        return "Accepted";
      case 203:
        return "Non-Authoritative Information";
      case 204:
        return "No Content";
      case 205:
        return "Reset Content";
      case 206:
        return "Partial Content";
      case 207:
        return "Multi-Status";
      case 208:
        return "Already Reported";
      case 209:
        return "IM Used";
      case 300:
        return "Multiple Choices";
      case 301:
        return "Moved Permanently";
      case 302:
        return "Found";
      case 303:
        return "See Other";
      case 304:
        return "Not Modified";
      case 305:
        return "Use Proxy";
      case 306:
        return "Switch Proxy";
      case 307:
        return "Temporary Redirect";
      case 308:
        return "Permanent Redirect";
      case 400:
        return "Bad Request";
      case 401:
        return "Unauthorized";
      case 402:
        return "Payment Required";
      case 403:
        return "Forbidden";
      case 404:
        return "Not Found";
      case 405:
        return "Method Not Allowed";
      case 406:
        return "Not Acceptable";
      case 407:
        return "Proxy Authentication Required";
      case 408:
        return "Request Timeout";
      case 409:
        return "Conflict";
      case 410:
        return "Gone";
      case 411:
        return "Length Required";
      case 412:
        return "Precondition Failed";
      case 413:
        return "Payload Too Large";
      case 414:
        return "URI Too Long";
      case 415:
        return "Unsupported Media Type";
      case 416:
        return "Range Not Satisfiable";
      case 417:
        return "Expectation Failed";
      case 418:
        return "I'm a teapot";
      case 421:
        return "Misdirected Request";
      case 422:
        return "Unprocessable Entity";
      case 423:
        return "Locked";
      case 424:
        return "Failed Dependency";
      case 425:
        return "Too Early";
      case 426:
        return "Upgrade Required";
      case 428:
        return "Precondition Required";
      case 429:
        return "Too Many Requests";
      case 431:
        return "Request Header Fields Too Large";
      case 451:
        return "Unavailable For Legal Reasons";
      case 500:
        return "Internal Server Error";
      case 501:
        return "Not Implemented";
      case 502:
        return "Bad Gateway";
      case 503:
        return "Service Unavailable";
      case 504:
        return "Gateway Timeout";
      case 505:
        return "HTTP Version Not Supported";
      case 506:
        return "Variant Also Negotiates";
      case 507:
        return "Insufficient Storage";
      case 508:
        return "Loop Detected";
      case 510:
        return "Not Extended";
      case 511:
        return "Network Authentication Required";
      default:
        return "Unknown";
      }
    }());
  }
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(STANDARDESE_IS_IN_THE_HOUSE)
  SYSTEM_ERROR2_NORETURN virtual void _do_throw_exception(const status_code<void> &code) const override  // NOLINT
  {
    assert(code.domain() == *this);                               // NOLINT
    const auto &c = static_cast<const http_status_code &>(code);  // NOLINT
    throw status_error<_http_status_code_domain>(c);
  }
#endif
};
//! A constexpr source variable for the `getaddrinfo()` code domain, which is that of `getaddrinfo()`. Returned by `_http_status_code_domain::get()`.
constexpr _http_status_code_domain http_status_code_domain;
inline constexpr const _http_status_code_domain &_http_status_code_domain::get()
{
  return http_status_code_domain;
}

namespace mixins
{
  template <class Base> inline bool mixin<Base, _http_status_code_domain>::is_http_informational() const noexcept
  {
    const auto &c = static_cast<const http_status_code *>(this)->value();
    return c >= 100 && c < 200;
  }
  template <class Base> inline bool mixin<Base, _http_status_code_domain>::is_http_success() const noexcept
  {
    const auto &c = static_cast<const http_status_code *>(this)->value();
    return c >= 200 && c < 300;
  }
  template <class Base> inline bool mixin<Base, _http_status_code_domain>::is_http_redirection() const noexcept
  {
    const auto &c = static_cast<const http_status_code *>(this)->value();
    return c >= 300 && c < 400;
  }
  template <class Base> inline bool mixin<Base, _http_status_code_domain>::is_http_client_error() const noexcept
  {
    const auto &c = static_cast<const http_status_code *>(this)->value();
    return c >= 400 && c < 500;
  }
  template <class Base> inline bool mixin<Base, _http_status_code_domain>::is_http_server_error() const noexcept
  {
    const auto &c = static_cast<const http_status_code *>(this)->value();
    return c >= 500 && c < 600;
  }
}  // namespace mixins


SYSTEM_ERROR2_NAMESPACE_END

#endif
