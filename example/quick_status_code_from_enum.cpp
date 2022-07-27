/* Example use of status_code with outcome
(C) 2020 Niall Douglas <http://www.nedproductions.biz/> (5 commits)
File Created: Sept 2020


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

#include "status-code/system_error2.hpp"

#include <iostream>

// This is some third party enumeration type in another namespace
namespace another_namespace
{
  // "Initialiser list" custom status code domain
  enum class AnotherCode : size_t
  {
    success1,
    goaway,
    success2,
    error2
  };
}  // namespace another_namespace

// To synthesise a custom status code domain for `AnotherCode`, inject the following
// template specialisation:
SYSTEM_ERROR2_NAMESPACE_BEGIN
template <> struct quick_status_code_from_enum<another_namespace::AnotherCode> : quick_status_code_from_enum_defaults<another_namespace::AnotherCode>
{
  // Text name of the enum
  static constexpr const auto domain_name = "Another Code";

  // Unique UUID for the enum. PLEASE use https://www.random.org/cgi-bin/randbyte?nbytes=16&format=h
  static constexpr const auto domain_uuid = "{be201f65-3962-dd0e-1266-a72e63776a42}";

  // Map of each enum value to its text string, and list of semantically equivalent errc's
  static const std::initializer_list<mapping> &value_mappings()
  {
    static const std::initializer_list<mapping> v = {
    // Format is: { enum value, "string representation", { list of errc mappings ... } }
    {another_namespace::AnotherCode::success1, "Success 1", {errc::success}},             //
    {another_namespace::AnotherCode::goaway, "Go away", {errc::permission_denied}},       //
    {another_namespace::AnotherCode::success2, "Success 2", {errc::success}},             //
    {another_namespace::AnotherCode::error2, "Error 2", {errc::function_not_supported}},  //
    };
    return v;
  }

  // Completely optional definition of mixin for the status code synthesised from `Enum`.
  // It can be omitted.
  template <class Base> struct mixin : Base
  {
    using Base::Base;

    // A custom method on the synthesised status code
    constexpr int custom_method() const { return 42; }
  };
};
SYSTEM_ERROR2_NAMESPACE_END

// If you wish easy manufacture of status codes from AnotherCode:
namespace another_namespace
{
  // ADL discovered, must be in same namespace as AnotherCode
  SYSTEM_ERROR2_CONSTEXPR14 inline SYSTEM_ERROR2_NAMESPACE::quick_status_code_from_enum_code<another_namespace::AnotherCode> status_code(AnotherCode c) { return SYSTEM_ERROR2_NAMESPACE::quick_status_code_from_enum_code<another_namespace::AnotherCode>(c); }
}  // namespace another_namespace


int main(int argc, char *argv[])
{
  // Make a status code of the synthesised code domain for `AnotherCode`. Note the
  // unqualified lookup, ADL discovers the status_code() free function.
  SYSTEM_ERROR2_CONSTEXPR14 auto v = status_code(another_namespace::AnotherCode::error2);
  assert(v.value() == another_namespace::AnotherCode::error2);
  assert(v.custom_method() == 42);

  // If you don't need custom methods, just use system_code, all erased
  // status codes recognise quick_status_code_from_enum
  SYSTEM_ERROR2_NAMESPACE::system_code v2(another_namespace::AnotherCode::error2);
  assert(v2 == v);

  // If v.success() is true, this is a precondition failure which terminates
  // the program
  SYSTEM_ERROR2_NAMESPACE::error err = v;
  assert(v2 == err);
  return 0;
}
