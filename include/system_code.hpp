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

#ifndef SYSTEM_ERROR2_SYSTEM_ERROR_HPP
#define SYSTEM_ERROR2_SYSTEM_ERROR_HPP

#include "posix_code.hpp"

#if defined(_WIN32) || defined(STANDARDESE_IS_IN_THE_HOUSE)
#include "nt_code.hpp"

SYSTEM_ERROR2_NAMESPACE_BEGIN
/*! An erased-mutable status code suitably large for all the system codes
which can be returned on this system.

For Windows, these might be:

    - `com_code` (`HRESULT`)  [you need to include "com_code.hpp" explicitly for this]
    - `nt_code` (`LONG`)
    - `win32_code` (`DWORD`)

So the erased type is `LONG` on Windows, as that can represent all of
the above.

For POSIX, `posix_code` (`int`) is possible, so this erased type is `int`
i.e. this is the type alias on POSIX, not the above:

`using system_code = status_code<erased<int>>`
*/
using system_code = status_code<erased<long>>;
SYSTEM_ERROR2_NAMESPACE_END

#else

SYSTEM_ERROR2_NAMESPACE_BEGIN
using system_code = status_code<erased<int>>;
SYSTEM_ERROR2_NAMESPACE_END

#endif

#endif
