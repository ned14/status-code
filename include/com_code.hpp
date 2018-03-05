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

#ifndef SYSTEM_ERROR2_COM_CODE_HPP
#define SYSTEM_ERROR2_COM_CODE_HPP

#if !defined(_WIN32) && !defined(STANDARDESE_IS_IN_THE_HOUSE)
#error This file should only be included on Windows
#endif

#include "nt_code.hpp"
#include "win32_code.hpp"

#ifndef STANDARDESE_IS_IN_THE_HOUSE
#include <comdef.h>
#endif

SYSTEM_ERROR2_NAMESPACE_BEGIN

class _com_code_domain;
/*! (Windows only) A COM error code. Note semantic equivalence testing is only implemented for `FACILITY_WIN32`
and `FACILITY_NT_BIT`. As you can see at [https://blogs.msdn.microsoft.com/eldar/2007/04/03/a-lot-of-hresult-codes/](https://blogs.msdn.microsoft.com/eldar/2007/04/03/a-lot-of-hresult-codes/),
there are an awful lot of COM error codes, and keeping mapping tables for all of them would be impractical
(for the Win32 and NT facilities, we actually reuse the mapping tables in `win32_code` and `nt_code`).
You can, of course, inherit your own COM code domain from this one and override the `_equivalent()` function
to add semantic equivalence testing for whichever extra COM codes that your application specifically needs.
*/
using com_code = status_code<_com_code_domain>;

/*! (Windows only) The implementation of the domain for COM error codes and/or `IErrorInfo`.
*/
class _com_code_domain : public status_code_domain
{
  template <class DomainType> friend class status_code;
  using _base = status_code_domain;

public:
  //! The value type of the COM code, which is a `HRESULT`
  using value_type = HRESULT;
  //! Thread safe reference to a message string fetched by `FormatMessage()` or `_com_error::ErrorMessage`
  class string_ref : public _base::string_ref
  {
  public:
    explicit string_ref(const _base::string_ref &o)
        : _base::string_ref(o)
    {
    }
    explicit string_ref(_base::string_ref &&o)
        : _base::string_ref(std::move(o))
    {
    }
    constexpr string_ref()
        : _base::string_ref(_base::string_ref::_refcounted_string_thunk)
    {
    }
    SYSTEM_ERROR2_CONSTEXPR14 explicit string_ref(const char *str)
        : _base::string_ref(str, _base::string_ref::_refcounted_string_thunk)
    {
    }
    string_ref(const string_ref &) = default;
    string_ref(string_ref &&) = default;
    string_ref &operator=(const string_ref &) = default;
    string_ref &operator=(string_ref &&) = default;
    ~string_ref() = default;
    //! Construct from a `HRESULT` error code
    explicit string_ref(HRESULT c, IErrorInfo *perrinfo = nullptr)
        : _base::string_ref(_base::string_ref::_refcounted_string_thunk)
    {
      _com_error ce(c, perrinfo);
#ifdef _UNICODE
      win32::DWORD wlen = (win32::DWORD) wcslen(ce.ErrorMessage());
      size_t allocation = wlen + (wlen >> 1);
      win32::DWORD bytes;
      if(wlen == 0)
      {
        goto failure;
      }
      for(;;)
      {
        auto *p = static_cast<char *>(malloc(allocation));  // NOLINT
        if(p == nullptr)
        {
          goto failure;
        }
        bytes = win32::WideCharToMultiByte(65001 /*CP_UTF8*/, 0, ce.ErrorMessage(), wlen + 1, p, allocation, nullptr, nullptr);
        if(bytes != 0)
        {
          this->_begin = p;
          this->_end = strchr(p, 0);
          while(this->_end[-1] == 10 || this->_end[-1] == 13)
          {
            --this->_end;
          }
          *const_cast<char *>(this->_end) = 0;  // NOLINT
          break;
        }
        free(p);  // NOLINT
        if(win32::GetLastError() == 0x7a /*ERROR_INSUFFICIENT_BUFFER*/)
        {
          allocation += allocation >> 2;
          continue;
        }
        goto failure;
      }
#else
      auto wlen = static_cast<win32::DWORD>(strlen(ce.ErrorMessage()));
      auto *p = static_cast<char *>(malloc(wlen + 1));  // NOLINT
      if(p == nullptr)
      {
        goto failure;
      }
      memcpy(p, ce.ErrorMessage(), wlen + 1);
      this->_begin = p;
      this->_end = p + wlen;  // NOLINT
      while(this->_end[-1] == 10 || this->_end[-1] == 13)
      {
        --this->_end;
      }
      *const_cast<char *>(this->_end) = 0;  // NOLINT
#endif
      _msg() = (_allocated_msg *) calloc(1, sizeof(_allocated_msg));  // NOLINT
      if(_msg() == nullptr)
      {
        free((void *) this->_begin);  // NOLINT
        goto failure;
      }
      ++_msg()->count;
      return;
    failure:
      _msg() = nullptr;  // disabled
      this->_begin = "failed to get message from system";
      this->_end = strchr(this->_begin, 0);
    }
  };

public:
  //! Default constructor
  constexpr _com_code_domain() noexcept : _base(0xdc8275428b4effac) {}
  _com_code_domain(const _com_code_domain &) = default;
  _com_code_domain(_com_code_domain &&) = default;
  _com_code_domain &operator=(const _com_code_domain &) = default;
  _com_code_domain &operator=(_com_code_domain &&) = default;
  ~_com_code_domain() = default;

  //! Constexpr singleton getter. Returns the address of the constexpr com_code_domain variable.
  static inline constexpr const _com_code_domain *get();

  virtual _base::string_ref name() const noexcept override final { return _base::string_ref("COM domain"); }  // NOLINT
protected:
  virtual bool _failure(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    return static_cast<const com_code &>(code).value() < 0;  // NOLINT
  }
  /*! Note semantic equivalence testing is only implemented for `FACILITY_WIN32` and `FACILITY_NT_BIT`.
  */
  virtual bool _equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final  // NOLINT
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const com_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const com_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    if((c1.value() & FACILITY_NT_BIT) != 0)
    {
      if(code2.domain() == nt_code_domain)
      {
        const auto &c2 = static_cast<const nt_code &>(code2);  // NOLINT
        if(c2.value() == (c1.value() & ~FACILITY_NT_BIT))
        {
          return true;
        }
      }
      else if(code2.domain() == generic_code_domain)
      {
        const auto &c2 = static_cast<const generic_code &>(code2);  // NOLINT
        if(static_cast<int>(c2.value()) == _nt_code_domain::_nt_code_to_errno(c1.value() & ~FACILITY_NT_BIT))
        {
          return true;
        }
      }
    }
    else if(HRESULT_FACILITY(c1.value()) == FACILITY_WIN32)
    {
      if(code2.domain() == win32_code_domain)
      {
        const auto &c2 = static_cast<const win32_code &>(code2);  // NOLINT
        if(c2.value() == HRESULT_CODE(c1.value()))
        {
          return true;
        }
      }
      else if(code2.domain() == generic_code_domain)
      {
        const auto &c2 = static_cast<const generic_code &>(code2);  // NOLINT
        if(static_cast<int>(c2.value()) == _win32_code_domain::_win32_code_to_errno(HRESULT_CODE(c1.value())))
        {
          return true;
        }
      }
    }
    return false;
  }
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c1 = static_cast<const com_code &>(code);  // NOLINT
    if(c1.value() == S_OK)
    {
      return errc::success;
    }
    if((c1.value() & FACILITY_NT_BIT) != 0)
    {
      return generic_code(static_cast<errc>(_nt_code_domain::_nt_code_to_errno(c1.value() & ~FACILITY_NT_BIT)));
    }
    if(HRESULT_FACILITY(c1.value()) == FACILITY_WIN32)
    {
      return generic_code(static_cast<errc>(_win32_code_domain::_win32_code_to_errno(HRESULT_CODE(c1.value()))));
    }
    return errc::unknown;
  }
  virtual _base::string_ref _message(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const com_code &>(code);  // NOLINT
    return string_ref(c.value());
  }
  virtual void _throw_exception(const status_code<void> &code) const override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const com_code &>(code);  // NOLINT
    throw status_error<_com_code_domain>(c);
  }
};
//! (Windows only) A constexpr source variable for the COM code domain. Returned by `_com_code_domain::get()`.
constexpr _com_code_domain com_code_domain;
inline constexpr const _com_code_domain *_com_code_domain::get()
{
  return &com_code_domain;
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
