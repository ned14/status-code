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

#ifndef SYSTEM_ERROR2_STATUS_ERROR_HPP
#define SYSTEM_ERROR2_STATUS_ERROR_HPP

#include "status_code.hpp"

#include <exception>  // for std::exception

SYSTEM_ERROR2_NAMESPACE_BEGIN

/*! Exception type representing a thrown status_code
  */
template <class DomainType> class status_error : public std::exception
{
  status_code<DomainType> _code;
  typename DomainType::string_ref _msgref;

public:
  //! The type of the status domain
  using domain_type = DomainType;
  //! The type of the status code
  using status_code_type = status_code<DomainType>;

  //! Constructs an instance
  explicit status_error(status_code<DomainType> code)
      : _code(static_cast<status_code<DomainType> &&>(code))
      , _msgref(_code.message())
  {
  }

  //! Return an explanatory string
  virtual const char *what() const noexcept override { return _msgref.c_str(); }  // NOLINT
  //! Returns a reference to the code
  const status_code_type &code() const & { return _code; }
  //! Returns a reference to the code
  status_code_type &code() & { return _code; }
  //! Returns a reference to the code
  const status_code_type &&code() const && { return _code; }
  //! Returns a reference to the code
  status_code_type &&code() && { return _code; }
};

SYSTEM_ERROR2_NAMESPACE_END

#endif
