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

#ifndef SYSTEM_ERROR2_WIN32_CODE_HPP
#define SYSTEM_ERROR2_WIN32_CODE_HPP

#if !defined(_WIN32) && !defined(STANDARDESE_IS_IN_THE_HOUSE)
#error This file should only be included on Windows
#endif

#include "generic_code.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

//! \exclude
namespace win32
{
  // A Win32 DWORD
  using DWORD = unsigned long;
  // Used to retrieve the current Win32 error code
  extern "C" DWORD __stdcall GetLastError();
  // Used to retrieve a locale-specific message string for some error code
  extern "C" DWORD __stdcall FormatMessageW(DWORD dwFlags, const void *lpSource, DWORD dwMessageId, DWORD dwLanguageId, wchar_t *lpBuffer, DWORD nSize, void /*va_list*/ *Arguments);
  // Converts UTF-16 message string to UTF-8
  extern "C" int __stdcall WideCharToMultiByte(unsigned int CodePage, DWORD dwFlags, const wchar_t *lpWideCharStr, int cchWideChar, char *lpMultiByteStr, int cbMultiByte, const char *lpDefaultChar, int *lpUsedDefaultChar);
#pragma comment(lib, "kernel32.lib")
}  // namespace win32

class _win32_code_domain;
class _com_code_domain;
//! (Windows only) A Win32 error code, those returned by `GetLastError()`.
using win32_code = status_code<_win32_code_domain>;
//! (Windows only) A specialisation of `status_error` for the Win32 error code domain.
using win32_error = status_error<_win32_code_domain>;

/*! (Windows only) The implementation of the domain for Win32 error codes, those returned by `GetLastError()`.
*/
class _win32_code_domain : public status_code_domain
{
  template <class DomainType> friend class status_code;
  friend class _com_code_domain;
  using _base = status_code_domain;
  static int _win32_code_to_errno(win32::DWORD c)
  {
    switch(c)
    {
    case 0:
      return 0;
#include "detail/win32_code_to_generic_code.ipp"
    }
    return -1;
  }
  //! Construct from a Win32 error code
  static _base::string_ref _make_string_ref(win32::DWORD c) noexcept
  {
    wchar_t buffer[32768];
    win32::DWORD wlen = win32::FormatMessageW(0x00001000 /*FORMAT_MESSAGE_FROM_SYSTEM*/ | 0x00000200 /*FORMAT_MESSAGE_IGNORE_INSERTS*/, nullptr, c, 0, buffer, 32768, nullptr);
    size_t allocation = wlen + (wlen >> 1);
    win32::DWORD bytes;
    if(wlen == 0)
    {
      return _base::string_ref("failed to get message from system");
    }
    for(;;)
    {
      auto *p = static_cast<char *>(malloc(allocation));  // NOLINT
      if(p == nullptr)
      {
        return _base::string_ref("failed to get message from system");
      }
      bytes = win32::WideCharToMultiByte(65001 /*CP_UTF8*/, 0, buffer, (int) (wlen + 1), p, (int) allocation, nullptr, nullptr);
      if(bytes != 0)
      {
        char *end = strchr(p, 0);
        while(end[-1] == 10 || end[-1] == 13)
        {
          --end;
        }
        *end = 0;  // NOLINT
        return _base::atomic_refcounted_string_ref(p, end - p);
      }
      free(p);  // NOLINT
      if(win32::GetLastError() == 0x7a /*ERROR_INSUFFICIENT_BUFFER*/)
      {
        allocation += allocation >> 2;
        continue;
      }
      return _base::string_ref("failed to get message from system");
    }
  }

public:
  //! The value type of the win32 code, which is a `win32::DWORD`
  using value_type = win32::DWORD;
  using _base::string_ref;

public:
  //! Default constructor
  constexpr _win32_code_domain() noexcept : _base(0x8cd18ee72d680f1b) {}
  _win32_code_domain(const _win32_code_domain &) = default;
  _win32_code_domain(_win32_code_domain &&) = default;
  _win32_code_domain &operator=(const _win32_code_domain &) = default;
  _win32_code_domain &operator=(_win32_code_domain &&) = default;
  ~_win32_code_domain() = default;

  //! Constexpr singleton getter. Returns the address of the constexpr win32_code_domain variable.
  static inline constexpr const _win32_code_domain *get();

  virtual string_ref name() const noexcept override final { return string_ref("win32 domain"); }  // NOLINT
protected:
  virtual bool _failure(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    return static_cast<const win32_code &>(code).value() != 0;  // NOLINT
  }
  virtual bool _equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final  // NOLINT
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const win32_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const win32_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    if(code2.domain() == generic_code_domain)
    {
      const auto &c2 = static_cast<const generic_code &>(code2);  // NOLINT
      if(static_cast<int>(c2.value()) == _win32_code_to_errno(c1.value()))
      {
        return true;
      }
    }
    return false;
  }
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const win32_code &>(code);  // NOLINT
    return generic_code(static_cast<errc>(_win32_code_to_errno(c.value())));
  }
  virtual string_ref _message(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const win32_code &>(code);  // NOLINT
    return _make_string_ref(c.value());
  }
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(STANDARDESE_IS_IN_THE_HOUSE)
  virtual void _throw_exception(const status_code<void> &code) const override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const win32_code &>(code);  // NOLINT
    throw status_error<_win32_code_domain>(c);
  }
#endif
};
//! (Windows only) A constexpr source variable for the win32 code domain, which is that of `GetLastError()` (Windows). Returned by `_win32_code_domain::get()`.
constexpr _win32_code_domain win32_code_domain;
inline constexpr const _win32_code_domain *_win32_code_domain::get()
{
  return &win32_code_domain;
}

SYSTEM_ERROR2_NAMESPACE_END

#endif