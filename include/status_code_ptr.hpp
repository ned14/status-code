/* Pointer to a SG14 status_code
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Sep 2018


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

#ifndef SYSTEM_ERROR2_STATUS_CODE_PTR_HPP
#define SYSTEM_ERROR2_STATUS_CODE_PTR_HPP

#include "status_code.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

namespace detail
{
  template <class StatusCode> class indirecting_domain : public status_code_domain
  {
    template <class DomainType> friend class status_code;
    using _base = status_code_domain;

  public:
    using value_type = StatusCode *;
    using _base::string_ref;

    constexpr indirecting_domain() noexcept : _base(0xc44f7bdeb2cc50e9 ^ typename StatusCode::domain_type().id() /* unique-ish based on domain's unique id */) {}
    indirecting_domain(const indirecting_domain &) = default;
    indirecting_domain(indirecting_domain &&) = default;  // NOLINT
    indirecting_domain &operator=(const indirecting_domain &) = default;
    indirecting_domain &operator=(indirecting_domain &&) = default;  // NOLINT
    ~indirecting_domain() = default;

#if __cplusplus < 201402L && !defined(_MSC_VER)
    static inline const indirecting_domain &get()
    {
      static indirecting_domain v;
      return v;
    }
#else
    static inline constexpr const indirecting_domain &get();
#endif

    virtual string_ref name() const noexcept override { return typename StatusCode::domain_type().name(); }  // NOLINT
  protected:
    using _mycode = status_code<indirecting_domain>;
    virtual bool _do_failure(const status_code<void> &code) const noexcept override  // NOLINT
    {
      assert(code.domain() == *this);
      const auto &c = static_cast<const _mycode &>(code);  // NOLINT
      return typename StatusCode::domain_type()._do_failure(*c.value());
    }
    virtual bool _do_equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override  // NOLINT
    {
      assert(code1.domain() == *this);
      const auto &c1 = static_cast<const _mycode &>(code1);  // NOLINT
      return typename StatusCode::domain_type()._do_equivalent(*c1.value(), code2);
    }
    virtual generic_code _generic_code(const status_code<void> &code) const noexcept override  // NOLINT
    {
      assert(code.domain() == *this);
      const auto &c = static_cast<const _mycode &>(code);  // NOLINT
      return typename StatusCode::domain_type()._generic_code(*c.value());
    }
    virtual string_ref _do_message(const status_code<void> &code) const noexcept override  // NOLINT
    {
      assert(code.domain() == *this);
      const auto &c = static_cast<const _mycode &>(code);  // NOLINT
      return typename StatusCode::domain_type()._do_message(*c.value());
    }
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(STANDARDESE_IS_IN_THE_HOUSE)
    SYSTEM_ERROR2_NORETURN virtual void _do_throw_exception(const status_code<void> &code) const override  // NOLINT
    {
      assert(code.domain() == *this);
      const auto &c = static_cast<const _mycode &>(code);  // NOLINT
      typename StatusCode::domain_type()._do_throw_exception(*c.value());
    }
#endif
    virtual void _do_erased_copy(status_code<void> &dst, const status_code<void> &src, size_t /*unused*/) const override  // NOLINT
    {
      assert(dst.domain() == *this);
      assert(src.domain() == *this);
      auto &d = static_cast<_mycode &>(dst);               // NOLINT
      const auto &_s = static_cast<const _mycode &>(src);  // NOLINT
      const StatusCode &s = *_s.value();
      new(&d) _mycode(in_place, new StatusCode(s));
    }
    virtual void _do_erased_destroy(status_code<void> &code, size_t /*unused*/) const noexcept override  // NOLINT
    {
      assert(code.domain() == *this);
      auto &c = static_cast<_mycode &>(code);  // NOLINT
      delete c.value();  // NOLINT
    }
  };
#if __cplusplus >= 201402L || defined(_MSC_VER)
  template <class StatusCode> constexpr indirecting_domain<StatusCode> _indirecting_domain{};
  template <class StatusCode> inline constexpr const indirecting_domain<StatusCode> &indirecting_domain<StatusCode>::get() { return _indirecting_domain<StatusCode>; }
#endif
}  // namespace detail

/*! Make an erased status code which indirects to a dynamically allocated status code.
This is useful for shoehorning a rich status code with large value type into a small
erased status code like `system_code`, with which the status code generated by this
function is compatible. Note that this function can throw due to `bad_alloc`.
*/
template <class T, typename std::enable_if<is_status_code<T>::value, bool>::type = true>  //
inline status_code<erased<typename std::add_pointer<typename std::decay<T>::type>::type>> make_status_code_ptr(T &&v)
{
  using status_code_type = typename std::decay<T>::type;
  return status_code<detail::indirecting_domain<status_code_type>>(in_place, new status_code_type(static_cast<T &&>(v)));
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
