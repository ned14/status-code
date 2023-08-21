/* Example use of transporting a std::exception_ptr inside an error
(C) 2018 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: March 2018


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

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>  // for sprintf

#include "status-code/system_error2.hpp"

#include <exception>
#include <mutex>
#include <stdexcept>
#include <system_error>
#include <utility>  // for std::move

static constexpr size_t max_exception_ptrs = 16;


using namespace SYSTEM_ERROR2_NAMESPACE;

struct exception_ptr_storage_t
{
  using index_type = unsigned int;

  mutable std::mutex lock;
  std::exception_ptr items[max_exception_ptrs];
  index_type idx{0};

  std::exception_ptr operator[](index_type i) const
  {
    std::lock_guard h(lock);
    return (idx - i < max_exception_ptrs) ? items[i % max_exception_ptrs] : std::exception_ptr();
  }
  index_type add(std::exception_ptr p)
  {
    std::lock_guard h(lock);
    items[idx] = std::move(p);
    return idx++;
  }
};
inline exception_ptr_storage_t exception_ptr_storage;

// Alias our new status code to its domain
class _thrown_exception_domain;
using thrown_exception_code = status_code<_thrown_exception_domain>;

class _thrown_exception_domain : public status_code_domain
{
  // We permit status code to call our protected functions
  template <class DomainType> friend class status_code;
  using _base = status_code_domain;

public:
  // Our value type is the index into the exception_ptr storage
  using value_type = exception_ptr_storage_t::index_type;
  // std::exception::what() returns const char *, so the default string_ref is sufficient
  using _base::string_ref;

  // Always use https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h to create a
  // unique 64 bit value for every unique domain you create!
  constexpr _thrown_exception_domain() noexcept
      : _base(0xb766b5e50597a655)
  {
  }

  // Default all the copy, move and destruct. This makes the type 100% constexpr in every way
  // which in turns allows the compiler to assume it will not be instantiated at runtime.
  _thrown_exception_domain(const _thrown_exception_domain &) = default;
  _thrown_exception_domain(_thrown_exception_domain &&) = default;
  _thrown_exception_domain &operator=(const _thrown_exception_domain &) = default;
  _thrown_exception_domain &operator=(_thrown_exception_domain &&) = default;
  ~_thrown_exception_domain() = default;

  // Fetch a constexpr instance of this domain
  static inline constexpr const _thrown_exception_domain &get();

  // Return the name of this domain
  virtual _base::string_ref name() const noexcept override final { return _base::string_ref("thrown exception"); }

  // Return information about the value type of this domain
  virtual payload_info_t payload_info() const noexcept override
  {
    return {sizeof(value_type), sizeof(status_code_domain *) + sizeof(value_type),
            (alignof(value_type) > alignof(status_code_domain *)) ? alignof(value_type) : alignof(status_code_domain *)};
  }

protected:
  // This internal routine maps an exception ptr onto a generic_code
  // It is surely hideously slow, but that's all relative in the end
  static errc _to_generic_code(value_type c) noexcept
  {
    const std::exception_ptr &e = exception_ptr_storage[c];
    try
    {
      if(!e)
        return errc::unknown;
      std::rethrow_exception(e);
    }
    catch(const std::invalid_argument & /*unused*/)
    {
      return errc::invalid_argument;
    }
    catch(const std::domain_error & /*unused*/)
    {
      return errc::argument_out_of_domain;
    }
    catch(const std::length_error & /*unused*/)
    {
      return errc::argument_list_too_long;
    }
    catch(const std::out_of_range & /*unused*/)
    {
      return errc::result_out_of_range;
    }
    catch(const std::logic_error & /*unused*/) /* base class for this group */
    {
      return errc::invalid_argument;
    }
    catch(const std::system_error &e) /* also catches ios::failure */
    {
      return static_cast<errc>(e.code().value());
    }
    catch(const std::overflow_error & /*unused*/)
    {
      return errc::value_too_large;
    }
    catch(const std::range_error & /*unused*/)
    {
      return errc::result_out_of_range;
    }
    catch(const std::runtime_error & /*unused*/) /* base class for this group */
    {
      return errc::resource_unavailable_try_again;
    }
    catch(const std::bad_alloc & /*unused*/)
    {
      return errc::not_enough_memory;
    }
    catch(...)
    {
    }
    return errc::unknown;
  }

  // Always true, as exception_ptr always represents failure
  virtual bool _do_failure(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    return true;
  }
  // True if the exception ptr is equivalent to some other status code
  virtual bool _do_equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const thrown_exception_code &>(code1);
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const thrown_exception_code &>(code2);
      // Always perform literal comparison when domains are equal. The fallback
      // semantic comparison of converting both to generic_code and comparing
      // will handle semantic comparison of the same domain.
      return c1.value() == c2.value();
    }
    // If anything in your coding matches anything in errc, you should match it here
    if(code2.domain() == generic_code_domain)
    {
      const auto &c2 = static_cast<const generic_code &>(code2);
      if(c2.value() == _to_generic_code(c1.value()))
      {
        return true;
      }
    }
    return false;
  }
  // Called as a fallback if _equivalent() fails
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c1 = static_cast<const thrown_exception_code &>(code);
    return generic_code(_to_generic_code(c1.value()));
  }
  // Extract the what() from the exception
  virtual _base::string_ref _do_message(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const thrown_exception_code &>(code);
    const std::exception_ptr &e = exception_ptr_storage[c.value()];
    try
    {
      if(!e)
        return _base::string_ref("expired");
      std::rethrow_exception(e);
    }
    catch(const std::exception &x)
    {
      /* MSVC throws a full copy here, because its rethrow_exception()
      makes copies instead of using the one stored in the array. So
      for it we actively must copy the message. */
#ifdef _WIN32
      auto *msg = x.what();
      auto len = strlen(msg);
      auto *p = static_cast<char *>(malloc(len + 1));
      if(p == nullptr)
      {
        return _base::string_ref("failed to allocate memory for what()");
      }
      memcpy(p, msg, len + 1);
      return _base::atomic_refcounted_string_ref(p, len);
#else
      return _base::string_ref(x.what());
#endif
    }
    catch(...)
    {
      return _base::string_ref("unknown thrown exception");
    }
  }
  // Throw the code as a C++ exception
  virtual void _do_throw_exception(const status_code<void> &code) const override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const thrown_exception_code &>(code);
    const std::exception_ptr &e = exception_ptr_storage[c.value()];
    std::rethrow_exception(e);
  }
};

//! A constexpr source variable for the throw exception code domain to return via get()
constexpr inline _thrown_exception_domain thrown_exception_domain;
inline constexpr const _thrown_exception_domain &_thrown_exception_domain::get()
{
  return thrown_exception_domain;
}

// Helper to construct a thrown_exception_code from a std::exception_ptr
inline thrown_exception_code make_status_code(std::exception_ptr ep)
{
  return thrown_exception_code(in_place, exception_ptr_storage.add(std::move(ep)));
}

#ifndef DONT_DEFINE_MAIN
int main()
{
  thrown_exception_code tec(make_status_code(std::make_exception_ptr(std::bad_alloc())));
  system_code sc(tec);
  printf("Thrown exception code has message %s\n", sc.message().c_str());
  printf("Thrown exception code == errc::not_enough_memory = %d\n", sc == errc::not_enough_memory);
  return 0;
}
#endif
