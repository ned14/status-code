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

/*! An erased status code which is always a failure. This refines
system_code into an object meeting the requirements of
https://wg21.link/P0709 *Just throw (or, Zero-overhead deterministic exceptions)*
with the following differences:

    - Always a failure (this is checked at construction, and if not the case,
    the program is terminated as this is a logic error)
    - No default construction.
    - No empty state possible.

As with `system_code`, it remains guaranteed to be two CPU registers in size,
and trivially copyable.
*/
class error : protected system_code
{
public:
  using system_code::value_type;
  using system_code::string_ref;

  error(const error &) = default;
  error(error &&) = default;
  error &operator=(const error &) = default;
  error &operator=(error &&) = default;
  ~error() = default;

  using system_code::domain;
  using system_code::message;
  using system_code::strictly_equivalent;
  using system_code::equivalent;
  using system_code::throw_exception;
  using system_code::value;

  /*! Explicit copy construction from any other status code if its type is trivially copyable and it would fit into our storage.
  The input is checked to ensure it is a failure, if not then `SYSTEM_ERROR2_FATAL(msg)` is called which by default calls `std::terminate()`.
  */
  template <class DomainType,  //
            typename std::enable_if<detail::type_erasure_is_safe<value_type, typename DomainType::value_type>::value, bool>::type = true>
  SYSTEM_ERROR2_CONSTEXPR14 explicit error(const status_code<DomainType> &v) noexcept : system_code(v)
  {
    if(!v.failure())
    {
      SYSTEM_ERROR2_FATAL("error constructed from a status code which is not a failure");
    }
  }
};

SYSTEM_ERROR2_NAMESPACE_END

#endif
