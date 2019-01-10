Herein lies a single header edition of https://wg21.link/P1028 `status_code`:

<dl>
  <dt>`system_error2.hpp`</dt>
  <dd>An inclusion of the following headers:
  
  <ul>
  <li>`config.hpp`
  <li>`error.hpp`
  <li>`errored_status_code.hpp`
  <li>`generic_code.hpp`
  <li>`posix_code.hpp`
  <li>`status_code.hpp`
  <li>`status_code_domain.hpp`
  <li>`status_code_ptr.hpp`
  <li>`status_error.hpp`
  <li>`system_code.hpp`
  </ul>
  <ul>
  <li>`nt_code.hpp` (Windows only)
  <li>`win32_code.hpp` (Windows only)
  </ul>
  
  The following headers are specifically NOT included:
  
  <ul>
  <li>`com_code.hpp` (drags in COM)
  <li>`iostream_support.hpp` (drags in `<iostream>`)
  <li>`std_error_code.hpp` (drags in `<system_error>` and its many, many dependencies)
  <li>`system_code_from_exception.hpp` (drags in `<system_error>` and its many, many dependencies)
  </ul>
  
  This bisection is to give an absolute minimum compile time
  impact edition of this library. See https://github.com/ned14/stl-header-heft.
  The bisection above causes the inclusion of the following system headers:
  
  <ul>
  <li>`<atomic>`
  <li>`<cassert>`
  <li>`<cerrno>`
  <li>`<cstddef>`
  <li>`<cstdlib>`
  <li>`<cstring>`
  <li>`<exception>`
  <li>`<initializer_list>`
  <li>`<new>`
  <li>`<type_traits>`
  <li>`<unistd.h>` (Mac OS only)
  <li>`<utility>` (C++ 17 or later only)
  </ul>

  </dd>
</dl>
