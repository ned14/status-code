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

#ifndef SYSTEM_ERROR2_STATUS_CODE_HPP
#define SYSTEM_ERROR2_STATUS_CODE_HPP

#include "status_code_domain.hpp"

#if __cplusplus >= 201700 || _HAS_CXX17
// 0.26
#include <utility>  // for in_place

SYSTEM_ERROR2_NAMESPACE_BEGIN
using in_place_t = std::in_place_t;
using std::in_place;
SYSTEM_ERROR2_NAMESPACE_END

#else

SYSTEM_ERROR2_NAMESPACE_BEGIN
//! Aliases `std::in_place_t` if on C++ 17 or later, else defined locally.
struct in_place_t
{
  explicit in_place_t() = default;
};
//! Aliases `std::in_place` if on C++ 17 or later, else defined locally.
constexpr in_place_t in_place{};
SYSTEM_ERROR2_NAMESPACE_END
#endif

SYSTEM_ERROR2_NAMESPACE_BEGIN

/*! A tag for an erased value type for `status_code<D>`.
  Available only if `ErasedType` is an integral type.
  */
template <class ErasedType,  //
          typename std::enable_if<std::is_integral<ErasedType>::value, bool>::type = true>
struct erased
{
  using value_type = ErasedType;
};

namespace detail
{
  template <class T, class U> struct safe_reinterpret_cast
  {
    union {
      T a{};
      U b;
    };
    constexpr explicit safe_reinterpret_cast(const U &v)
        : b(v)
    {
    }
    constexpr T value() const { return a; }  // NOLINT
  };

#if 0
  template <class T, class U,  //
            typename std::enable_if<std::is_same<typename std::decay<decltype(make_status_code(std::declval<U>()))>::type, T>::value, bool>::type = true>
  constexpr inline int has_make_status_code(U &&);
  template <class T, class U> constexpr inline char has_make_status_code(...);
  template <class T, class U> struct is_implicit_construction_enabled
  {
    static constexpr bool value = sizeof(has_make_status_code<T, U>(std::declval<U>())) == sizeof(int);
  };
#endif

  template <class T> struct is_status_code
  {
    static constexpr bool value = false;
  };
  template <class T> struct is_status_code<status_code<T>>
  {
    static constexpr bool value = true;
  };
  template <class T> struct is_erased_status_code
  {
    static constexpr bool value = false;
  };
  template <class T> struct is_erased_status_code<status_code<erased<T>>>
  {
    static constexpr bool value = true;
  };
}  // namespace detail

//! Trait returning true if the type is a status code.
template <class T> struct is_status_code
{
  static constexpr bool value = detail::is_status_code<typename std::decay<T>::type>::value;
};

/*! A type erased lightweight status code reflecting empty, success, or failure.
Differs from `status_code<erased<>>` by being always available irrespective of
the domain's value type, but cannot be copied, moved, nor destructed. Thus one
always passes this around by const lvalue reference.
*/
template <> class status_code<void>
{
  template <class T> friend class status_code;

public:
  //! The type of the domain.
  using domain_type = status_code_domain;
  //! The type of the status code.
  using value_type = void;
  //! The type of a reference to a message string.
  using string_ref = typename domain_type::string_ref;

protected:
  const status_code_domain *_domain{nullptr};

protected:
  //! No default construction at type erased level
  status_code() = default;
  //! No public copying at type erased level
  status_code(const status_code &) = default;
  //! No public moving at type erased level
  status_code(status_code &&) = default;
  //! No public assignment at type erased level
  status_code &operator=(const status_code &) = default;
  //! No public assignment at type erased level
  status_code &operator=(status_code &&) = default;
  //! No public destruction at type erased level
  ~status_code() = default;

  //! Used to construct a non-empty type erased status code
  constexpr explicit status_code(const status_code_domain *v)
      : _domain(v)
  {
  }

public:
  //! Return the status code domain.
  constexpr const status_code_domain &domain() const noexcept { return *_domain; }
  //! True if the status code is empty.
  constexpr bool empty() const noexcept { return _domain == nullptr; }

  //! Return a reference to a string textually representing a code.
  string_ref message() const noexcept { return (_domain != nullptr) ? _domain->_message(*this) : string_ref("(empty)"); }
  //! True if code means success.
  bool success() const noexcept { return (_domain != nullptr) ? !_domain->_failure(*this) : false; }
  //! True if code means failure.
  bool failure() const noexcept { return (_domain != nullptr) ? _domain->_failure(*this) : false; }
  /*! True if code is strictly (and potentially non-transitively) semantically equivalent to another code in another domain.
  Note that usually non-semantic i.e. pure value comparison is used when the other status code has the same domain.
  As `equivalent()` will try mapping to generic code, this usually captures when two codes have the same semantic
  meaning in `equivalent()`.
  */
  template <class T> bool strictly_equivalent(const status_code<T> &o) const noexcept
  {
    if(_domain && o._domain)
      return _domain->_equivalent(*this, o);
    // If we are both empty, we are equivalent
    if(!_domain && !o._domain)
      return true;
    // Otherwise not equivalent
    return false;
  }
  /*! True if code is equivalent, by any means, to another code in another domain (guaranteed transitive).
  Firstly `strictly_equivalent()` is run in both directions. If neither succeeds, each domain is asked
  for the equivalent generic code and those are compared.
  */
  template <class T> inline bool equivalent(const status_code<T> &o) const noexcept;
  //! Throw a code as a C++ exception.
  void throw_exception() const { _domain->_throw_exception(*this); }
};

/*! A lightweight, typed, status code reflecting empty, success, or failure.
This is the main workhorse of the system_error2 library.

An ADL discovered helper function `make_status_code(T, Args...)` is looked up by one of the constructors.
If it is found, and it generates a status code compatible with this status code, implicit construction
is made available.
*/
template <class DomainType> class status_code : public status_code<void>
{
  template <class T> friend class status_code;
  using _base = status_code<void>;

public:
  //! The type of the domain.
  using domain_type = DomainType;
  //! The type of the status code.
  using value_type = typename domain_type::value_type;
  //! The type of a reference to a message string.
  using string_ref = typename domain_type::string_ref;

protected:
  value_type _value{};

public:
  //! Default construction to empty
  status_code() = default;
  //! Copy constructor
  status_code(const status_code &) = default;
  //! Move constructor
  status_code(status_code &&) = default;  // NOLINT
  //! Copy assignment
  status_code &operator=(const status_code &) = default;
  //! Move assignment
  status_code &operator=(status_code &&) = default;  // NOLINT
  ~status_code() = default;

  //! Implicit construction from any type where an ADL discovered `make_status_code(T, Args ...)` returns a `status_code`.
  template <class T, class... Args,                                                                                //
            class MakeStatusCodeOutType = decltype(make_status_code(std::declval<T>(), std::declval<Args>()...)),  // ADL enable
            typename std::enable_if<!std::is_same<typename std::decay<T>::type, status_code>::value                // not copy/move of self
                                    && !std::is_same<typename std::decay<T>::type, value_type>::value              // not copy/move of value type
                                    && is_status_code<MakeStatusCodeOutType>::value                                // ADL makes a status code
                                    && std::is_constructible<status_code, MakeStatusCodeOutType>::value,           // ADLed status code is compatible
                                    bool>::type = true>
  constexpr status_code(T &&v, Args &&... args) noexcept(noexcept(make_status_code(std::declval<T>(), std::declval<Args>()...)))  // NOLINT
  : status_code(make_status_code(static_cast<T &&>(v), static_cast<Args &&>(args)...))
  {
  }
  //! Explicit in-place construction.
  template <class... Args>
  constexpr explicit status_code(in_place_t /*unused */, Args &&... args) noexcept(std::is_nothrow_constructible<value_type, Args &&...>::value)
      : _base(domain_type::get())
      , _value(static_cast<Args &&>(args)...)
  {
  }
  //! Explicit in-place construction from initialiser list.
  template <class T, class... Args>
  constexpr explicit status_code(in_place_t /*unused */, std::initializer_list<T> il, Args &&... args) noexcept(std::is_nothrow_constructible<value_type, std::initializer_list<T>, Args &&...>::value)
      : _base(domain_type::get())
      , _value(il, static_cast<Args &&>(args)...)
  {
  }
  //! Explicit copy construction from a `value_type`.
  constexpr explicit status_code(const value_type &v) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
      : _base(domain_type::get())
      , _value(v)
  {
  }
  //! Explicit move construction from a `value_type`.
  constexpr explicit status_code(value_type &&v) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
      : _base(domain_type::get())
      , _value(static_cast<value_type &&>(v))
  {
  }
  /*! Explicit construction from an erased status code. Available only if
  `value_type` is trivially destructible and `sizeof(status_code) <= sizeof(status_code<erased<>>)`.
  Does not check if domains are equal.
  */
  template <class ErasedType,  //
            typename std::enable_if<detail::type_erasure_is_safe<ErasedType, value_type>::value, bool>::type = true>
  constexpr explicit status_code(const status_code<erased<ErasedType>> &v)
      : status_code(reinterpret_cast<const value_type &>(v._value))  // NOLINT
  {
#if __cplusplus >= 201400
    assert(v.domain() == domain());
#endif
  }

  //! Assignment from a `value_type`.
  constexpr status_code &operator=(const value_type &v) noexcept(std::is_nothrow_copy_assignable<value_type>::value)
  {
    _value = v;
    return *this;
  }

  // Replace the type erased implementations with type aware implementations for better codegen
  //! Return the status code domain.
  constexpr const domain_type &domain() const noexcept { return *static_cast<const domain_type *>(this->_domain); }
  //! Return a reference to a string textually representing a code.
  string_ref message() const noexcept { return this->_domain ? string_ref(domain()._message(*this)) : string_ref("(empty)"); }

  //! Reset the code to empty.
  SYSTEM_ERROR2_CONSTEXPR14 void clear() { *this = status_code(); }

#if __cplusplus >= 201400 || _MSC_VER >= 1910 /* VS2017 */
  //! Return a reference to the `value_type`.
  constexpr value_type &value() & noexcept { return _value; }
  //! Return a reference to the `value_type`.
  constexpr value_type &&value() && noexcept { return _value; }
#endif
  //! Return a reference to the `value_type`.
  constexpr const value_type &value() const &noexcept { return _value; }
  //! Return a reference to the `value_type`.
  constexpr const value_type &&value() const &&noexcept { return _value; }
};

/*! Type erased status_code, but copyable/movable/destructible unlike `status_code<void>`. Available
only if `erased<>` is available, which is when the domain's type is trivially
copyable, and if the size of the domain's typed error code is less than or equal to
this erased error code.

An ADL discovered helper function `make_status_code(T, Args...)` is looked up by one of the constructors.
If it is found, and it generates a status code compatible with this status code, implicit construction
is made available.
*/
template <class ErasedType> class status_code<erased<ErasedType>> : public status_code<void>
{
  template <class T> friend class status_code;
  using _base = status_code<void>;

public:
  //! The type of the domain (void, as it is erased).
  using domain_type = void;
  //! The type of the erased status code.
  using value_type = ErasedType;
  //! The type of a reference to a message string.
  using string_ref = typename _base::string_ref;

protected:
  value_type _value{};

public:
  //! Default construction to empty
  status_code() = default;
  //! Copy constructor
  status_code(const status_code &) = default;
  //! Move constructor
  status_code(status_code &&) = default;  // NOLINT
  //! Copy assignment
  status_code &operator=(const status_code &) = default;
  //! Move assignment
  status_code &operator=(status_code &&) = default;  // NOLINT
  ~status_code() = default;

  //! Implicit copy construction from any other status code if its value type is trivially copyable and it would fit into our storage
  template <class DomainType,  //
            typename std::enable_if<detail::type_erasure_is_safe<value_type, typename DomainType::value_type>::value, bool>::type = true>
  constexpr status_code(const status_code<DomainType> &v) noexcept : _base(v), _value(detail::safe_reinterpret_cast<value_type, typename DomainType::value_type>(v.value()).value())  // NOLINT
  {
  }
  //! Implicit construction from any type where an ADL discovered `make_status_code(T, Args ...)` returns a `status_code`.
  template <class T, class... Args,                                                                                //
            class MakeStatusCodeOutType = decltype(make_status_code(std::declval<T>(), std::declval<Args>()...)),  // ADL enable
            typename std::enable_if<!std::is_same<typename std::decay<T>::type, status_code>::value                // not copy/move of self
                                    && !std::is_same<typename std::decay<T>::type, value_type>::value              // not copy/move of value type
                                    && is_status_code<MakeStatusCodeOutType>::value                                // ADL makes a status code
                                    && std::is_constructible<status_code, MakeStatusCodeOutType>::value,           // ADLed status code is compatible
                                    bool>::type = true>
  constexpr status_code(T &&v, Args &&... args) noexcept(noexcept(make_status_code(std::declval<T>(), std::declval<Args>()...)))  // NOLINT
  : status_code(make_status_code(static_cast<T &&>(v), static_cast<Args &&>(args)...))
  {
  }
  //! Reset the code to empty.
  SYSTEM_ERROR2_CONSTEXPR14 void clear() { *this = status_code(); }
  //! Return the erased `value_type` by value.
  constexpr value_type value() const noexcept { return _value; }
};

SYSTEM_ERROR2_NAMESPACE_END

#endif
