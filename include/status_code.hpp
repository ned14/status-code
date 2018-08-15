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

//! Namespace for user injected mixins
namespace mixins
{
  template <class Base, class T> struct mixin : public Base
  {
    using Base::Base;
  };
}

/*! A tag for an erased value type for `status_code<D>`.
Available only if `ErasedType` is a trivially copyable type.
*/
template <class ErasedType,  //
          typename std::enable_if<std::is_trivially_copyable<ErasedType>::value, bool>::type = true>
struct erased
{
  using value_type = ErasedType;
};

namespace detail
{
  /* A partially compliant implementation of C++20's std::bit_cast function.
  Our bit_cast is only guaranteed to be constexpr when both the input and output
  arguments are either integrals or enums. However, this covers most use cases
  since the vast majority of status_codes have an underlying type that is either
  an integral or enum. */

  template <class T> struct is_integral_or_enum
    : std::integral_constant<bool, std::is_integral<T>::value || std::is_enum<T>::value> {};

  template <class To, class From> struct is_static_castable
    : std::integral_constant<bool, is_integral_or_enum<To>::value && is_integral_or_enum<From>::value> {};

  template <class To, class From> struct is_bit_castable
    : std::integral_constant<bool,
        sizeof(To) == sizeof(From) &&
        std::is_trivially_copyable<To>::value &&
        std::is_trivially_copyable<From>::value> {};

  template <class To, class From> union bit_cast_union {
    From source;
    To target;
  };

  template <class To, class From,
            typename std::enable_if<is_bit_castable<To, From>::value && is_static_castable<To, From>::value, bool>::type = true>
  constexpr To bit_cast(const From &from) noexcept
  {
    return static_cast<To>(from);
  }

  template <class To, class From,
            typename std::enable_if<is_bit_castable<To, From>::value && !is_static_castable<To, From>::value, bool>::type = true>
  constexpr To bit_cast(const From &from) noexcept
  {
    return detail::bit_cast_union<To, From>{from}.target;
  }

  /* erasure_cast performs a bit_cast with additional rules to handle types
  of differing sizes. For integral & enum types, it may perform a narrowing
  or widing conversion with static_cast if necessary, before doing the final
  conversion with bit_cast. When casting to or from non-integral, non-enum
  types it may insert the value into another object with extra padding bytes
  to satisfy bit_cast's preconditions that both types have the same size. */

  template <class To, class From> struct is_erasure_castable
    : std::integral_constant<bool,
        std::is_trivially_copyable<To>::value &&
        std::is_trivially_copyable<From>::value> {};

  template <class T, bool = std::is_enum<T>::value> struct identity_or_underlying_type { using type = T; };
  template <class T> struct identity_or_underlying_type<T, true> { using type = typename std::underlying_type<T>::type; };

  template <class OfSize, class OfSign> struct erasure_integer
  {
    using type = typename std::conditional<
      std::is_signed<typename identity_or_underlying_type<OfSign>::type>::value,
      typename std::make_signed<typename identity_or_underlying_type<OfSize>::type>::type,
      typename std::make_unsigned<typename identity_or_underlying_type<OfSize>::type>::type>::type;
  };

  template <class ErasedType, std::size_t N> struct padded_erasure_object
  {
    static_assert(std::is_trivially_copyable<ErasedType>::value, "ErasedType must be TriviallyCopyable");
    static_assert(alignof(ErasedType) <= sizeof(ErasedType), "ErasedType must not be over-aligned");
    ErasedType value;
    char padding[N];
    constexpr padded_erasure_object(const ErasedType &v) noexcept : value(v), padding{} {}
  };

  template <class To, class From,
            typename std::enable_if<is_erasure_castable<To, From>::value && (sizeof(To) == sizeof(From)), bool>::type = true>
  constexpr To erasure_cast(const From &from) noexcept
  {
    return detail::bit_cast<To>(from);
  }

  template <class To, class From,
            typename std::enable_if<is_erasure_castable<To, From>::value && is_static_castable<To, From>::value && (sizeof(To) < sizeof(From)), bool>::type = true>
  constexpr To erasure_cast(const From &from) noexcept
  {
    return static_cast<To>(detail::bit_cast<typename erasure_integer<From, To>::type>(from));
  }

  template <class To, class From,
            typename std::enable_if<is_erasure_castable<To, From>::value && is_static_castable<To, From>::value && (sizeof(To) > sizeof(From)), bool>::type = true>
  constexpr To erasure_cast(const From &from) noexcept
  {
    return detail::bit_cast<To>(static_cast<typename erasure_integer<To, From>::type>(from));
  }

  template <class To, class From,
            typename std::enable_if<is_erasure_castable<To, From>::value && !is_static_castable<To, From>::value && (sizeof(To) < sizeof(From)), bool>::type = true>
  constexpr To erasure_cast(const From &from) noexcept
  {
    return detail::bit_cast<padded_erasure_object<To, sizeof(From) - sizeof(To)>>(from).value;
  }

  template <class To, class From,
            typename std::enable_if<is_erasure_castable<To, From>::value && !is_static_castable<To, From>::value && (sizeof(To) > sizeof(From)), bool>::type = true>
  constexpr To erasure_cast(const From &from) noexcept
  {
    return detail::bit_cast<To>(padded_erasure_object<From, sizeof(To) - sizeof(From)>{from});
  }

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
  using domain_type = void;
  //! The type of the status code.
  using value_type = void;
  //! The type of a reference to a message string.
  using string_ref = typename status_code_domain::string_ref;

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
  constexpr explicit status_code(const status_code_domain *v) noexcept : _domain(v) {}

public:
  //! Return the status code domain.
  constexpr const status_code_domain &domain() const noexcept { return *_domain; }
  //! True if the status code is empty.
  SYSTEM_ERROR2_NODISCARD constexpr bool empty() const noexcept { return _domain == nullptr; }

  //! Return a reference to a string textually representing a code.
  string_ref message() const noexcept { return (_domain != nullptr) ? _domain->_do_message(*this) : string_ref("(empty)"); }
  //! True if code means success.
  bool success() const noexcept { return (_domain != nullptr) ? !_domain->_do_failure(*this) : false; }
  //! True if code means failure.
  bool failure() const noexcept { return (_domain != nullptr) ? _domain->_do_failure(*this) : false; }
  /*! True if code is strictly (and potentially non-transitively) semantically equivalent to another code in another domain.
  Note that usually non-semantic i.e. pure value comparison is used when the other status code has the same domain.
  As `equivalent()` will try mapping to generic code, this usually captures when two codes have the same semantic
  meaning in `equivalent()`.
  */
  template <class T> bool strictly_equivalent(const status_code<T> &o) const noexcept
  {
    if(_domain && o._domain)
      return _domain->_do_equivalent(*this, o);
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
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(STANDARDESE_IS_IN_THE_HOUSE)
  //! Throw a code as a C++ exception.
  SYSTEM_ERROR2_NORETURN void throw_exception() const { _domain->_do_throw_exception(*this); }
#endif
};

namespace detail
{
  template <class DomainType> class status_code_storage : public status_code<void>
  {
    using _base = status_code<void>;

  public:
    //! The type of the domain.
    using domain_type = DomainType;
    //! The type of the status code.
    using value_type = typename domain_type::value_type;
    //! The type of a reference to a message string.
    using string_ref = typename domain_type::string_ref;

#ifndef NDEBUG
    static_assert(!std::is_default_constructible<value_type>::value || std::is_nothrow_default_constructible<value_type>::value, "DomainType::value_type is not nothrow default constructible!");
    static_assert(!std::is_move_constructible<value_type>::value || std::is_nothrow_move_constructible<value_type>::value, "DomainType::value_type is not nothrow move constructible!");
    static_assert(std::is_nothrow_destructible<value_type>::value, "DomainType::value_type is not nothrow destructible!");
#endif

    // Replace the type erased implementations with type aware implementations for better codegen
    //! Return the status code domain.
    constexpr const domain_type &domain() const noexcept { return *static_cast<const domain_type *>(this->_domain); }

    //! Reset the code to empty.
    SYSTEM_ERROR2_CONSTEXPR14 void clear() noexcept
    {
      this->_value.~value_type();
      this->_domain = nullptr;
      new(&this->_value) value_type();
    }

#if __cplusplus >= 201400 || _MSC_VER >= 1910 /* VS2017 */
    //! Return a reference to the `value_type`.
    constexpr value_type &value() & noexcept { return this->_value; }
    //! Return a reference to the `value_type`.
    constexpr value_type &&value() && noexcept { return this->_value; }
#endif
    //! Return a reference to the `value_type`.
    constexpr const value_type &value() const &noexcept { return this->_value; }
    //! Return a reference to the `value_type`.
    constexpr const value_type &&value() const &&noexcept { return this->_value; }

  protected:
    status_code_storage() = default;
    status_code_storage(const status_code_storage &) = default;
    status_code_storage(status_code_storage &&) = default;  // NOLINT
    status_code_storage &operator=(const status_code_storage &) = default;
    status_code_storage &operator=(status_code_storage &&) = default;  // NOLINT
    ~status_code_storage() = default;

    value_type _value{};
    struct _value_type_constructor
    {
    };
    template <class... Args>
    constexpr status_code_storage(_value_type_constructor /*unused*/, const status_code_domain *v, Args &&... args)
        : _base(v)
        , _value(static_cast<Args &&>(args)...)
    {
    }
  };
}

/*! A lightweight, typed, status code reflecting empty, success, or failure.
This is the main workhorse of the system_error2 library.

An ADL discovered helper function `make_status_code(T, Args...)` is looked up by one of the constructors.
If it is found, and it generates a status code compatible with this status code, implicit construction
is made available.

You may mix in custom member functions and member function overrides by injecting a specialisation of
`mixins::mixin<Base, YourDomainType>`. Your mixin must inherit from `Base`.
*/
template <class DomainType> class status_code : public mixins::mixin<detail::status_code_storage<DomainType>, DomainType>
{
  template <class T> friend class status_code;
  using _base = mixins::mixin<detail::status_code_storage<DomainType>, DomainType>;

public:
  //! The type of the domain.
  using domain_type = DomainType;
  //! The type of the status code.
  using value_type = typename domain_type::value_type;
  //! The type of a reference to a message string.
  using string_ref = typename domain_type::string_ref;

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

  /***** KEEP THESE IN SYNC WITH ERRORED_STATUS_CODE *****/
  //! Implicit construction from any type where an ADL discovered `make_status_code(T, Args ...)` returns a `status_code`.
  template <class T, class... Args,                                                                                //
            class MakeStatusCodeOutType = decltype(make_status_code(std::declval<T>(), std::declval<Args>()...)),  // ADL enable
            typename std::enable_if<!std::is_same<typename std::decay<T>::type, status_code>::value                // not copy/move of self
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
      : _base(typename _base::_value_type_constructor{}, &domain_type::get(), static_cast<Args &&>(args)...)
  {
  }
  //! Explicit in-place construction from initialiser list.
  template <class T, class... Args>
  constexpr explicit status_code(in_place_t /*unused */, std::initializer_list<T> il, Args &&... args) noexcept(std::is_nothrow_constructible<value_type, std::initializer_list<T>, Args &&...>::value)
      : _base(typename _base::_value_type_constructor{}, &domain_type::get(), il, static_cast<Args &&>(args)...)
  {
  }
  //! Explicit copy construction from a `value_type`.
  constexpr explicit status_code(const value_type &v) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
      : _base(typename _base::_value_type_constructor{}, &domain_type::get(), v)
  {
  }
  //! Explicit move construction from a `value_type`.
  constexpr explicit status_code(value_type &&v) noexcept(std::is_nothrow_move_constructible<value_type>::value)
      : _base(typename _base::_value_type_constructor{}, &domain_type::get(), static_cast<value_type &&>(v))
  {
  }
  /*! Explicit construction from an erased status code. Available only if
  `value_type` is trivially destructible and `sizeof(status_code) <= sizeof(status_code<erased<>>)`.
  Does not check if domains are equal.
  */
  template <class ErasedType,  //
            typename std::enable_if<detail::type_erasure_is_safe<ErasedType, value_type>::value, bool>::type = true>
  constexpr explicit status_code(const status_code<erased<ErasedType>> &v) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
      : status_code(detail::erasure_cast<value_type>(v.value()))
  {
#if __cplusplus >= 201400
    assert(v.domain() == this->domain());
#endif
  }

  //! Assignment from a `value_type`.
  SYSTEM_ERROR2_CONSTEXPR14 status_code &operator=(const value_type &v) noexcept(std::is_nothrow_copy_assignable<value_type>::value)
  {
    this->_value = v;
    return *this;
  }

  //! Return a reference to a string textually representing a code.
  string_ref message() const noexcept { return this->_domain ? string_ref(this->domain()._do_message(*this)) : string_ref("(empty)"); }
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

  /***** KEEP THESE IN SYNC WITH ERRORED_STATUS_CODE *****/
  //! Implicit copy construction from any other status code if its value type is trivially copyable and it would fit into our storage
  template <class DomainType,  //
            typename std::enable_if<detail::type_erasure_is_safe<value_type, typename DomainType::value_type>::value, bool>::type = true>
  constexpr status_code(const status_code<DomainType> &v) noexcept : _base(v), _value(detail::erasure_cast<value_type>(v.value()))
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
  SYSTEM_ERROR2_CONSTEXPR14 void clear() noexcept { *this = status_code(); }
  //! Return the erased `value_type` by value.
  constexpr value_type value() const noexcept { return _value; }
};

SYSTEM_ERROR2_NAMESPACE_END

#endif
