/* Proposed SG14 status_code
(C) 2023 Henrik Steffen Gaßmann
File Created: Apr 2023


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

#ifndef SYSTEM_ERROR2_FMT_SUPPORT_HPP
#define SYSTEM_ERROR2_FMT_SUPPORT_HPP

#include "status_code_domain.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN

namespace detail
{
  template <class StringView, template <class> class Formatter> struct string_ref_formatter : private Formatter<StringView>
  {
  private:
    using base = Formatter<StringView>;

  public:
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) -> typename ParseContext::iterator { return base::parse(ctx); }

    template <typename FormatContext> auto format(const status_code_domain::string_ref &str, FormatContext &ctx) -> typename FormatContext::iterator
    {
      return base::format(StringView(str.data(), str.size()), ctx);
    }
  };
}  // namespace detail

SYSTEM_ERROR2_NAMESPACE_END

#if __cpp_lib_format >= 202106L
#include <format>

SYSTEM_ERROR2_NAMESPACE_BEGIN
namespace detail
{
  template <typename T> using std_formatter = std::formatter<T, char>;
}
SYSTEM_ERROR2_NAMESPACE_END

template <>
struct std::formatter<SYSTEM_ERROR2_NAMESPACE::status_code_domain::string_ref, char>
    : SYSTEM_ERROR2_NAMESPACE::detail::string_ref_formatter<std::string_view, SYSTEM_ERROR2_NAMESPACE::detail::std_formatter>
{
};
#endif

#if __has_include(<fmt/core.h>)
#include <fmt/core.h>

SYSTEM_ERROR2_NAMESPACE_BEGIN
namespace detail
{
  template <typename T> using fmt_formatter = fmt::formatter<T, char>;
}
SYSTEM_ERROR2_NAMESPACE_END

template <>
struct fmt::formatter<SYSTEM_ERROR2_NAMESPACE::status_code_domain::string_ref, char>
    : SYSTEM_ERROR2_NAMESPACE::detail::string_ref_formatter<fmt::string_view, SYSTEM_ERROR2_NAMESPACE::detail::fmt_formatter>
{
};
#endif

#endif
