/* Proposed SG14 status_code
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

#ifndef SYSTEM_ERROR2_ERROR_HPP
#define SYSTEM_ERROR2_ERROR_HPP

#include "system_code.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

/*! An erased `system_code` which is always a failure. The closest equivalent to
`std::error_code`, except it cannot be null and cannot be modified.

This refines `system_code` into an `error` object meeting the requirements of
[P0709 Zero-overhead deterministic exceptions](https://wg21.link/P0709).

Differences from `system_code`:

- Always a failure (this is checked at construction, and if not the case,
the program is terminated as this is a logic error)
- No default construction.
- No empty state possible.
- Is immutable.

As with `system_code`, it remains guaranteed to be two CPU registers in size,
and trivially copyable.
*/
class error : public system_code
{
  using system_code::clear;
  using system_code::empty;
  using system_code::success;
  using system_code::failure;

public:
  //! The type of the erased error code.
  using system_code::value_type;
  //! The type of a reference to a message string.
  using system_code::string_ref;

  //! Default construction not permitted.
  error() = delete;
  //! Copy constructor.
  error(const error &) = default;
  //! Move constructor.
  error(error &&) = default;
  //! Copy assignment.
  error &operator=(const error &) = default;
  //! Move assignment.
  error &operator=(error &&) = default;
  ~error() = default;

  /*! Implicit copy construction from any other status code if its value type is trivially copyable and it would fit into our storage.

  The input is checked to ensure it is a failure, if not then `SYSTEM_ERROR2_FATAL()` is called which by default calls `std::terminate()`.
  */
  template <class DomainType,  //
            typename std::enable_if<detail::type_erasure_is_safe<value_type, typename DomainType::value_type>::value, bool>::type = true>
  error(const status_code<DomainType> &v) noexcept : system_code(v)  // NOLINT
  {
    if(!v.failure())
    {
      SYSTEM_ERROR2_FATAL("error constructed from a status code which is not a failure");
    }
  }
  /*! Implicit construction from any type where an ADL discovered `make_status_code(T &&)` returns a `status_code`
  whose value type is trivially copyable and it would fit into our storage.

  The input is checked to ensure it is a failure, if not then `SYSTEM_ERROR2_FATAL()` is called which by default calls `std::terminate()`.
  */
  template <class T,                                                                           //
            class _IfMakeStatusCode = decltype(make_status_code(std::declval<T>())),           //
            typename std::enable_if<!std::is_same<typename std::decay<T>::type, error>::value  //
                                    && is_status_code<_IfMakeStatusCode>::value                //
                                    && detail::type_erasure_is_safe<value_type, typename _IfMakeStatusCode::value_type>::value,
                                    bool>::type = true>
  error(T &&v) noexcept(noexcept(make_status_code(std::declval<T>())))  // NOLINT
  : error(make_status_code(static_cast<T &&>(v)))
  {
  }
};

#ifndef NDEBUG
static_assert(sizeof(error) == 2 * sizeof(void *), "error is not exactly two pointers in size!");
static_assert(std::is_trivially_copyable<error>::value, "error is not trivially copyable!");
#endif

//! True if the status code's are semantically equal via `equivalent()`.
template <class DomainType> inline bool operator==(const status_code<DomainType> &a, const error &b) noexcept
{
  return a.equivalent(static_cast<const system_code &>(b));
}
//! True if the status code's are not semantically equal via `equivalent()`.
template <class DomainType> inline bool operator!=(const status_code<DomainType> &a, const error &b) noexcept
{
  return !a.equivalent(static_cast<const system_code &>(b));
}
//! True if the status code's are semantically equal via `equivalent()` to the generic code.
inline bool operator==(const error &a, errc b) noexcept
{
  return a.equivalent(generic_code(b));
}
//! True if the status code's are semantically equal via `equivalent()` to the generic code.
inline bool operator==(errc a, const error &b) noexcept
{
  return b.equivalent(generic_code(a));
}
//! True if the status code's are not semantically equal via `equivalent()` to the generic code.
inline bool operator!=(const error &a, errc b) noexcept
{
  return !a.equivalent(generic_code(b));
}
//! True if the status code's are not semantically equal via `equivalent()` to the generic code.
inline bool operator!=(errc a, const error &b) noexcept
{
  return !b.equivalent(generic_code(a));
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
