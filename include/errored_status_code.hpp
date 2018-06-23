/* Proposed SG14 status_code
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Jun 2018


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

#ifndef SYSTEM_ERROR2_ERRORED_STATUS_CODE_HPP
#define SYSTEM_ERROR2_ERRORED_STATUS_CODE_HPP

#include "generic_code.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

/*! A `status_code` which is always a failure. The closest equivalent to
`std::error_code`, except it cannot be null, cannot be modified, and is templated.

Differences from `status_code`:

- Always a failure (this is checked at construction, and if not the case,
the program is terminated as this is a logic error)
- No default construction.
- No empty state possible.
- Is immutable.
*/
template <class DomainType> class errored_status_code : public status_code<DomainType>
{
  using _base = status_code<DomainType>;
  using _base::clear;
  using _base::empty;
  using _base::success;
  using _base::failure;

  struct implicit_converting_constructor
  {
  };
  struct explicit_converting_constructor
  {
  };

public:
  //! The type of the erased error code.
  using typename _base::value_type;
  //! The type of a reference to a message string.
  using typename _base::string_ref;

  using _base::_base;
  //! Default construction not permitted.
  errored_status_code() = delete;
  //! Copy constructor.
  errored_status_code(const errored_status_code &) = default;
  //! Move constructor.
  errored_status_code(errored_status_code &&) = default;
  //! Copy assignment.
  errored_status_code &operator=(const errored_status_code &) = default;
  //! Move assignment.
  errored_status_code &operator=(errored_status_code &&) = default;
  ~errored_status_code() = default;

  //! Implicitly construct from any similarly erased status code
  constexpr errored_status_code(const _base &o) noexcept(std::is_nothrow_copy_constructible<_base>::value)
      : _base(o)
  {
  }
  //! Implicitly construct from any similarly erased status code
  constexpr errored_status_code(_base &&o) noexcept(std::is_nothrow_move_constructible<_base>::value)
      : _base(static_cast<_base &&>(o))
  {
  }
  //! Implicitly construct from any convertible status code
  template <class T, typename std::enable_if<std::is_convertible<T, status_code<DomainType>>::value && !std::is_constructible<status_code<DomainType>, T>::value, bool>::type = true>
  constexpr errored_status_code(T &&o, implicit_converting_constructor /*unused*/ = {}) noexcept(std::is_nothrow_constructible<_base, T>::value)
      : _base(static_cast<T &&>(o))
  {
  }
  //! Explicitly construct from any constructible status code
  template <class T, typename std::enable_if<!std::is_convertible<T, status_code<DomainType>>::value && std::is_constructible<status_code<DomainType>, T>::value, bool>::type = true>
  constexpr explicit errored_status_code(T &&o, explicit_converting_constructor /*unused*/ = {}) noexcept(std::is_nothrow_constructible<_base, T>::value)
      : _base(static_cast<T &&>(o))
  {
  }

  //! Return a const reference to the `value_type`.
  constexpr const value_type &value() const &noexcept { return this->_value; }
};

//! True if the status code's are semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2> inline bool operator==(const status_code<DomainType1> &a, const errored_status_code<DomainType2> &b) noexcept
{
  return a.equivalent(static_cast<const status_code<DomainType2> &>(b));
}
//! True if the status code's are semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2> inline bool operator==(const errored_status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept
{
  return static_cast<const status_code<DomainType1> &>(a).equivalent(b);
}
//! True if the status code's are not semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2> inline bool operator!=(const status_code<DomainType1> &a, const errored_status_code<DomainType2> &b) noexcept
{
  return !a.equivalent(static_cast<const status_code<DomainType2> &>(b));
}
//! True if the status code's are not semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2> inline bool operator!=(const errored_status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept
{
  return !static_cast<const status_code<DomainType1> &>(a).equivalent(b);
}
//! True if the status code's are semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator==(const errored_status_code<DomainType1> &a, errc b) noexcept
{
  return static_cast<const status_code<DomainType1> &>(a).equivalent(generic_code(b));
}
//! True if the status code's are semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator==(errc a, const errored_status_code<DomainType1> &b) noexcept
{
  return static_cast<const status_code<DomainType1> &>(b).equivalent(generic_code(a));
}
//! True if the status code's are not semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator!=(const errored_status_code<DomainType1> &a, errc b) noexcept
{
  return !static_cast<const status_code<DomainType1> &>(a).equivalent(generic_code(b));
}
//! True if the status code's are not semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator!=(errc a, const errored_status_code<DomainType1> &b) noexcept
{
  return !static_cast<const status_code<DomainType1> &>(b).equivalent(generic_code(a));
}


SYSTEM_ERROR2_NAMESPACE_END

#endif
