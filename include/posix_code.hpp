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

#ifndef SYSTEM_ERROR2_POSIX_CODE_HPP
#define SYSTEM_ERROR2_POSIX_CODE_HPP

#include "generic_code.hpp"

#include <atomic>
#include <cstdlib>  // for malloc
#include <cstring>  // for strchr and strerror_r

SYSTEM_ERROR2_NAMESPACE_BEGIN

class _posix_code_domain;
//! A POSIX error code, those returned by `errno`.
using posix_code = status_code<_posix_code_domain>;

/*! The implementation of the domain for POSIX error codes, those returned by `errno`.
*/
class _posix_code_domain : public status_code_domain
{
  template <class DomainType> friend class status_code;
  using _base = status_code_domain;

public:
  //! The value type of the POSIX code, which is an `int`
  using value_type = int;
  //! Thread safe reference to a message string fetched by `strerror_r()`
  class string_ref : public _base::string_ref
  {
    struct _allocated_msg
    {
      mutable std::atomic<unsigned> count;
    };
    _allocated_msg *&_msg() { return reinterpret_cast<_allocated_msg *&>(this->_state[0]); }
    const _allocated_msg *_msg() const { return reinterpret_cast<const _allocated_msg *>(this->_state[0]); }
  protected:
    virtual void _copy(_base::string_ref *dest) const & override final
    {
      if(_msg())
      {
        auto count = _msg()->count.fetch_add(1);
        assert(count != 0);
      }
      new(static_cast<string_ref *>(dest)) string_ref(this->_begin, this->_end, this->_state[0], this->_state[1]);
    }
    virtual void _move(_base::string_ref *dest) && noexcept override final
    {
      new(static_cast<string_ref *>(dest)) string_ref(this->_begin, this->_end, this->_state[0], this->_state[1]);
      if(_msg())
      {
        _msg() = nullptr;
      }
    }

  public:
    using _base::string_ref::string_ref;
    //! Construct from a POSIX error code
    explicit string_ref(int c)
    {
      char buffer[1024] = "";
#ifdef _WIN32
      strerror_s(buffer, sizeof(buffer), c);
#elif defined(__linux__)
      char *s = strerror_r(c, buffer, sizeof(buffer));
      if(s != nullptr)
      {
        strncpy(buffer, s, sizeof(buffer));
        buffer[1023] = 0;
      }
#else
      strerror_r(c, buffer, sizeof(buffer));
#endif
      size_t length = strlen(buffer);
      char *p = (char *) malloc(length + 1);
      if(p == nullptr)
        goto failure;
      memcpy(p, buffer, length + 1);
      this->_begin = p;
      this->_end = p + length;
      _msg() = (_allocated_msg *) calloc(1, sizeof(_allocated_msg));
      if(_msg() == nullptr)
      {
        free((void *) this->_begin);
        goto failure;
      }
      ++_msg()->count;
      return;
    failure:
      _msg() = nullptr;  // disabled
      this->_begin = "failed to get message from system";
      this->_end = strchr(this->_begin, 0);
    }
    //! Allow explicit cast up
    explicit string_ref(_base::string_ref v) { static_cast<string_ref &&>(v)._move(this); }
    ~string_ref() override final
    {
      if(_msg())
      {
        auto count = _msg()->count.fetch_sub(1);
        if(count == 1)
        {
          free((void *) this->_begin);
          delete _msg();
        }
      }
    }
  };

public:
  //! Default constructor
  constexpr _posix_code_domain()
      : _base(0xa59a56fe5f310933)
  {
  }
  _posix_code_domain(const _posix_code_domain &) = default;
  _posix_code_domain(_posix_code_domain &&) = default;
  _posix_code_domain &operator=(const _posix_code_domain &) = default;
  _posix_code_domain &operator=(_posix_code_domain &&) = default;
  ~_posix_code_domain() = default;

  //! Constexpr singleton getter. Returns the address of the constexpr posix_code_domain variable.
  static inline constexpr const _posix_code_domain *get();

  virtual _base::string_ref name() const noexcept override final { return _base::string_ref("posix domain"); }
protected:
  virtual bool _failure(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    return static_cast<const posix_code &>(code).value() != 0;
  }
  virtual bool _equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const posix_code &>(code1);
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const posix_code &>(code2);
      return c1.value() == c2.value();
    }
    if(code2.domain() == generic_code_domain)
    {
      const auto &c2 = static_cast<const generic_code &>(code2);
      if(static_cast<int>(c2.value()) == c1.value())
        return true;
    }
    return false;
  }
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const posix_code &>(code);
    return generic_code(static_cast<errc>(c.value()));
  }
  virtual _base::string_ref _message(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const posix_code &>(code);
    return string_ref(c.value());
  }
  virtual void _throw_exception(const status_code<void> &code) const override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const posix_code &>(code);
    throw status_error<_posix_code_domain>(c);
  }
};
//! A constexpr source variable for the POSIX code domain, which is that of `errno`. Returned by `_posix_code_domain::get()`.
constexpr _posix_code_domain posix_code_domain;
inline constexpr const _posix_code_domain *_posix_code_domain::get()
{
  return &posix_code_domain;
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
