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

#include "quick_status_code_from_enum.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

namespace detail
{
  template <class T> inline void is_errored_check_function(T *v)
  {
    if(v->success())
    {
      std::terminate();
    }
  }
}  // namespace detail

/*! A `status_code` which is always a failure. The closest equivalent to
`std::error_code`, except it cannot be modified, and is templated.

Differences from `status_code`:

- Never successful (this contract is checked on construction, if fails then it
terminates the process).
- Is immutable.
*/
template <class DomainType>
class errored_status_code : public detail::status_code_impl<DomainType, detail::is_errored_check_function>
{
  using _base = detail::status_code_impl<DomainType, detail::is_errored_check_function>;
  using _base::clear;
  using _base::success;

public:
  using _base::_base;

  //! The type of the domain.
  using typename _base::domain_type;
  //! The type of the error code.
  using typename _base::value_type;
  //! The type of a reference to a message string.
  using typename _base::string_ref;

  //! Explicitly construct from any similar status code
  explicit errored_status_code(const _base &o) noexcept(std::is_nothrow_copy_constructible<_base>::value)
      : _base(o)
  {
    detail::is_errored_check_function(this);
  }
  //! Explicitly construct from any similar status code
  explicit errored_status_code(_base &&o) noexcept(std::is_nothrow_move_constructible<_base>::value)
      : _base(static_cast<_base &&>(o))
  {
    detail::is_errored_check_function(this);
  }

  //! Implicit construction from any type where an ADL discovered `make_status_code(T, Args ...)` returns a
  //! `status_code`.
  SYSTEM_ERROR2_TEMPLATE(class T, class... Args,  //
                         class MakeStatusCodeResult = typename detail::safe_get_make_status_code_result<
                         T, Args...>::type)  // Safe ADL lookup of make_status_code(), returns void if not found
  SYSTEM_ERROR2_TREQUIRES(
  SYSTEM_ERROR2_TPRED(!std::is_same<typename std::decay<T>::type, errored_status_code>::value  // not copy/move of self
                      && !std::is_same<typename std::decay<T>::type, in_place_t>::value        // not in_place_t
                      && is_status_code<MakeStatusCodeResult>::value                  // ADL makes a status code
                      && std::is_constructible<_base, MakeStatusCodeResult>::value))  // ADLed status code is compatible
  errored_status_code(T &&v,
                      Args &&...args) noexcept(detail::safe_get_make_status_code_noexcept<T, Args...>::value)  // NOLINT
      : _base(make_status_code(static_cast<T &&>(v), static_cast<Args &&>(args)...))
  {
    detail::is_errored_check_function(this);
  }

  //! Always false (including at compile time), as errored status codes are never successful.
  constexpr bool success() const noexcept { return false; }
  //! Return a const reference to the `value_type`.
  constexpr const value_type &value() const & noexcept { return this->_value; }
};

namespace traits
{
  template <class DomainType> struct is_move_bitcopying<errored_status_code<DomainType>>
  {
    static constexpr bool value = is_move_bitcopying<typename DomainType::value_type>::value;
  };
}  // namespace traits

template <class ErasedType>
class errored_status_code<detail::erased<ErasedType>>
    : public detail::status_code_impl<detail::erased<ErasedType>, detail::is_errored_check_function>
{
  using _base = detail::status_code_impl<detail::erased<ErasedType>, detail::is_errored_check_function>;
  using _base::success;

public:
  using _base::_base;

  using domain_type = typename _base::domain_type;
  using value_type = typename _base::value_type;
  using string_ref = typename _base::string_ref;

  //! Explicitly construct from any similarly erased status code
  explicit errored_status_code(const _base &o) noexcept(std::is_nothrow_copy_constructible<_base>::value)
      : _base(o)
  {
    detail::is_errored_check_function(this);
  }
  //! Explicitly construct from any similarly erased status code
  explicit errored_status_code(_base &&o) noexcept(std::is_nothrow_move_constructible<_base>::value)
      : _base(static_cast<_base &&>(o))
  {
    detail::is_errored_check_function(this);
  }

  //! Implicit construction from any type where an ADL discovered `make_status_code(T, Args ...)` returns a
  //! `status_code`.
  SYSTEM_ERROR2_TEMPLATE(class T, class... Args,  //
                         class MakeStatusCodeResult = typename detail::safe_get_make_status_code_result<
                         T, Args...>::type)  // Safe ADL lookup of make_status_code(), returns void if not found
  SYSTEM_ERROR2_TREQUIRES(
  SYSTEM_ERROR2_TPRED(!std::is_same<typename std::decay<T>::type, errored_status_code>::value  // not copy/move of self
                      && !std::is_same<typename std::decay<T>::type, value_type>::value  // not copy/move of value type
                      && is_status_code<MakeStatusCodeResult>::value                     // ADL makes a status code
                      && std::is_constructible<_base, MakeStatusCodeResult>::value))  // ADLed status code is compatible
  errored_status_code(T &&v,
                      Args &&...args) noexcept(detail::safe_get_make_status_code_noexcept<T, Args...>::value)  // NOLINT
      : _base(make_status_code(static_cast<T &&>(v), static_cast<Args &&>(args)...))
  {
    detail::is_errored_check_function(this);
  }

  //! Always false (including at compile time), as errored status codes are never successful.
  constexpr bool success() const noexcept { return false; }
  //! Return the erased `value_type` by value.
  constexpr value_type value() const noexcept { return this->_value; }
};
/*! An erased type specialisation of `errored_status_code<D>`.
Available only if `ErasedType` satisfies `traits::is_move_bitcopying<ErasedType>::value`.
*/
template <class ErasedType> using erased_errored_status_code = errored_status_code<detail::erased<ErasedType>>;


namespace traits
{
  template <class ErasedType> struct is_move_bitcopying<errored_status_code<detail::erased<ErasedType>>>
  {
    static constexpr bool value = true;
  };
}  // namespace traits


//! True if the status code's are semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2>
inline bool operator==(const errored_status_code<DomainType1> &a, const errored_status_code<DomainType2> &b) noexcept
{
  return a.equivalent(static_cast<const status_code<DomainType2> &>(b));
}
//! True if the status code's are semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2>
inline bool operator==(const status_code<DomainType1> &a, const errored_status_code<DomainType2> &b) noexcept
{
  return a.equivalent(static_cast<const status_code<DomainType2> &>(b));
}
//! True if the status code's are semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2>
inline bool operator==(const errored_status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept
{
  return static_cast<const status_code<DomainType1> &>(a).equivalent(b);
}
//! True if the status code's are not semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2>
inline bool operator!=(const errored_status_code<DomainType1> &a, const errored_status_code<DomainType2> &b) noexcept
{
  return !a.equivalent(static_cast<const status_code<DomainType2> &>(b));
}
//! True if the status code's are not semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2>
inline bool operator!=(const status_code<DomainType1> &a, const errored_status_code<DomainType2> &b) noexcept
{
  return !a.equivalent(static_cast<const status_code<DomainType2> &>(b));
}
//! True if the status code's are not semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2>
inline bool operator!=(const errored_status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept
{
  return !static_cast<const status_code<DomainType1> &>(a).equivalent(b);
}
//! True if the status code's are semantically equal via `equivalent()` to `make_status_code(T)`.
SYSTEM_ERROR2_TEMPLATE(class DomainType1, class T,  //
                       class MakeStatusCodeResult = typename detail::safe_get_make_status_code_result<
                       const T &>::type)  // Safe ADL lookup of make_status_code(), returns void if not found
SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(is_status_code<MakeStatusCodeResult>::value))  // ADL makes a status code
inline bool operator==(const errored_status_code<DomainType1> &a, const T &b)
{
  return a.equivalent(make_status_code(b));
}
//! True if the status code's are semantically equal via `equivalent()` to `make_status_code(T)`.
SYSTEM_ERROR2_TEMPLATE(class T, class DomainType1,  //
                       class MakeStatusCodeResult = typename detail::safe_get_make_status_code_result<
                       const T &>::type)  // Safe ADL lookup of make_status_code(), returns void if not found
SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(is_status_code<MakeStatusCodeResult>::value))  // ADL makes a status code
inline bool operator==(const T &a, const errored_status_code<DomainType1> &b)
{
  return b.equivalent(make_status_code(a));
}
//! True if the status code's are not semantically equal via `equivalent()` to `make_status_code(T)`.
SYSTEM_ERROR2_TEMPLATE(class DomainType1, class T,  //
                       class MakeStatusCodeResult = typename detail::safe_get_make_status_code_result<
                       const T &>::type)  // Safe ADL lookup of make_status_code(), returns void if not found
SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(is_status_code<MakeStatusCodeResult>::value))  // ADL makes a status code
inline bool operator!=(const errored_status_code<DomainType1> &a, const T &b)
{
  return !a.equivalent(make_status_code(b));
}
//! True if the status code's are semantically equal via `equivalent()` to `make_status_code(T)`.
SYSTEM_ERROR2_TEMPLATE(class T, class DomainType1,  //
                       class MakeStatusCodeResult = typename detail::safe_get_make_status_code_result<
                       const T &>::type)  // Safe ADL lookup of make_status_code(), returns void if not found
SYSTEM_ERROR2_TREQUIRES(SYSTEM_ERROR2_TPRED(is_status_code<MakeStatusCodeResult>::value))  // ADL makes a status code
inline bool operator!=(const T &a, const errored_status_code<DomainType1> &b)
{
  return !b.equivalent(make_status_code(a));
}
//! True if the status code's are semantically equal via `equivalent()` to
//! `quick_status_code_from_enum<T>::code_type(b)`.
template <class DomainType1, class T,                                                     //
          class QuickStatusCodeType = typename quick_status_code_from_enum<T>::code_type  // Enumeration has been
                                                                                          // activated
          >
inline bool operator==(const errored_status_code<DomainType1> &a, const T &b)
{
  return a.equivalent(QuickStatusCodeType(b));
}
//! True if the status code's are semantically equal via `equivalent()` to
//! `quick_status_code_from_enum<T>::code_type(a)`.
template <class T, class DomainType1,                                                     //
          class QuickStatusCodeType = typename quick_status_code_from_enum<T>::code_type  // Enumeration has been
                                                                                          // activated
          >
inline bool operator==(const T &a, const errored_status_code<DomainType1> &b)
{
  return b.equivalent(QuickStatusCodeType(a));
}
//! True if the status code's are not semantically equal via `equivalent()` to
//! `quick_status_code_from_enum<T>::code_type(b)`.
template <class DomainType1, class T,                                                     //
          class QuickStatusCodeType = typename quick_status_code_from_enum<T>::code_type  // Enumeration has been
                                                                                          // activated
          >
inline bool operator!=(const errored_status_code<DomainType1> &a, const T &b)
{
  return !a.equivalent(QuickStatusCodeType(b));
}
//! True if the status code's are not semantically equal via `equivalent()` to
//! `quick_status_code_from_enum<T>::code_type(a)`.
template <class T, class DomainType1,                                                     //
          class QuickStatusCodeType = typename quick_status_code_from_enum<T>::code_type  // Enumeration has been
                                                                                          // activated
          >
inline bool operator!=(const T &a, const errored_status_code<DomainType1> &b)
{
  return !b.equivalent(QuickStatusCodeType(a));
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
