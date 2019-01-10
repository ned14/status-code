Herein lie the following single header editions of https://wg21.link/P1028 `status_code`:

<dl>
  <dt><tt>system_error2.hpp</tt></dt>
  <dd>An inclusion of the following headers:
  
  <ul>
  <li><tt>config.hpp</tt>
  <li><tt>error.hpp</tt>
  <li><tt>errored_status_code.hpp</tt>
  <li><tt>generic_code.hpp</tt>
  <li><tt>posix_code.hpp</tt>
  <li><tt>status_code.hpp</tt>
  <li><tt>status_code_domain.hpp</tt>
  <li><tt>status_code_ptr.hpp</tt>
  <li><tt>status_error.hpp</tt>
  <li><tt>system_code.hpp</tt>
  </ul>
  <ul>
  <li><tt>nt_code.hpp</tt> (Windows only)
  <li><tt>win32_code.hpp</tt> (Windows only)
  </ul>
  
  The following headers are specifically NOT included:
  
  <ul>
  <li><tt>com_code.hpp</tt> (drags in COM)
  <li><tt>iostream_support.hpp</tt> (drags in <tt><iostream></tt>)
  <li><tt>std_error_code.hpp</tt> (drags in <tt><system_error></tt> and its many, many dependencies)
  <li><tt>system_code_from_exception.hpp</tt> (drags in <tt><system_error></tt> and its many, many dependencies)
  </ul>
  
  This bisection is to give an absolute minimum compile time
  impact edition of this library. See https://github.com/ned14/stl-header-heft.
  The bisection above causes the inclusion of the following system headers:
  
  <ul>
  <li><tt><atomic></tt>
  <li><tt><cassert></tt>
  <li><tt><cerrno></tt>
  <li><tt><cstddef></tt>
  <li><tt><cstdlib></tt>
  <li><tt><cstring></tt>
  <li><tt><exception></tt>
  <li><tt><initializer_list></tt>
  <li><tt><new></tt>
  <li><tt><type_traits></tt>
  <li><tt><unistd.h></tt> (Mac OS only)
  <li><tt><utility></tt> (C++ 17 or later only)
  </ul>

  </dd>
</dl>
