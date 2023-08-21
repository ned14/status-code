Herein lie the following single header editions of https://wg21.link/P1028 `status_code`:

<dl>
  <dt><tt>system_error2.hpp</tt></dt>
  <dd>An inclusion of the following headers:
  
  <ul>
  <li><tt>config.hpp</tt>
  <li><tt>error.hpp</tt>
  <li><tt>errored_status_code.hpp</tt>
  <li><tt>generic_code.hpp</tt>
  <li><tt>nested_status_code.hpp</tt>
  <li><tt>posix_code.hpp</tt>
  <li><tt>status_code.hpp</tt>
  <li><tt>status_code_domain.hpp</tt>
  <li><tt>status_error.hpp</tt>
  <li><tt>system_code.hpp</tt>
  </ul>
  <ul>
  <li><tt>nt_code.hpp</tt> (used on Windows only, completely omitted in <tt>system_error2-nowindows.hpp</tt>)
  <li><tt>win32_code.hpp</tt> (used on Windows only, completely omitted in <tt>system_error2-nowindows.hpp</tt>)
  </ul>
  
  The following headers are specifically NOT included:
  
  <ul>
  <li><tt>com_code.hpp</tt> (drags in COM)
  <li><tt>iostream_support.hpp</tt> (drags in <tt>&lt;iostream&gt;</tt>)
  <li><tt>std_error_code.hpp</tt> (drags in <tt>&lt;system_error&gt;</tt> and its many, many dependencies)
  <li><tt>system_code_from_exception.hpp</tt> (drags in <tt>&lt;system_error&gt;</tt> and its many, many dependencies)
  </ul>
  
  This bisection is to give an absolute minimum compile time
  impact edition of this library. See https://github.com/ned14/stl-header-heft.
  The bisection above causes the inclusion of the following system headers:
  
  <ul>
  <li><tt>&lt;atomic&gt;</tt>
  <li><tt>&lt;cassert&gt;</tt>
  <li><tt>&lt;cerrno&gt;</tt>
  <li><tt>&lt;cstddef&gt;</tt>
  <li><tt>&lt;cstdlib&gt;</tt>
  <li><tt>&lt;cstring&gt;</tt>
  <li><tt>&lt;exception&gt;</tt>
  <li><tt>&lt;initializer_list&gt;</tt>
  <li><tt>&lt;new&gt;</tt>
  <li><tt>&lt;type_traits&gt;</tt>
  <li><tt>&lt;unistd.h&gt;</tt> (Mac OS only)
  <li><tt>&lt;utility&gt;</tt> (C++ 17 or later only)
  </ul>

  </dd>
</dl>
