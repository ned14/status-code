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

#ifndef SYSTEM_ERROR2_GENERIC_CODE_HPP
#define SYSTEM_ERROR2_GENERIC_CODE_HPP

#include "status_error.hpp"

#include <cerrno>  // for error constants

SYSTEM_ERROR2_NAMESPACE_BEGIN

//! The generic error coding (POSIX)
enum class errc : int
{
  success = 0,
  unknown = -1,

  address_family_not_supported = EAFNOSUPPORT,
  address_in_use = EADDRINUSE,
  address_not_available = EADDRNOTAVAIL,
  already_connected = EISCONN,
  argument_list_too_long = E2BIG,
  argument_out_of_domain = EDOM,
  bad_address = EFAULT,
  bad_file_descriptor = EBADF,
  broken_pipe = EPIPE,
  connection_aborted = ECONNABORTED,
  connection_already_in_progress = EALREADY,
  connection_refused = ECONNREFUSED,
  connection_reset = ECONNRESET,
  cross_device_link = EXDEV,
  destination_address_required = EDESTADDRREQ,
  device_or_resource_busy = EBUSY,
  directory_not_empty = ENOTEMPTY,
  executable_format_error = ENOEXEC,
  file_exists = EEXIST,
  file_too_large = EFBIG,
  filename_too_long = ENAMETOOLONG,
  function_not_supported = ENOSYS,
  host_unreachable = EHOSTUNREACH,
  illegal_byte_sequence = EILSEQ,
  inappropriate_io_control_operation = ENOTTY,
  interrupted = EINTR,
  invalid_argument = EINVAL,
  invalid_seek = ESPIPE,
  io_error = EIO,
  is_a_directory = EISDIR,
  message_size = EMSGSIZE,
  network_down = ENETDOWN,
  network_reset = ENETRESET,
  network_unreachable = ENETUNREACH,
  no_buffer_space = ENOBUFS,
  no_child_process = ECHILD,
  no_lock_available = ENOLCK,
  no_message = ENOMSG,
  no_protocol_option = ENOPROTOOPT,
  no_space_on_device = ENOSPC,
  no_such_device_or_address = ENXIO,
  no_such_device = ENODEV,
  no_such_file_or_directory = ENOENT,
  no_such_process = ESRCH,
  not_a_directory = ENOTDIR,
  not_a_socket = ENOTSOCK,
  not_connected = ENOTCONN,
  not_enough_memory = ENOMEM,
  operation_in_progress = EINPROGRESS,
  operation_not_permitted = EPERM,
  operation_not_supported = EOPNOTSUPP,
  permission_denied = EACCES,
  protocol_not_supported = EPROTONOSUPPORT,
  read_only_file_system = EROFS,
  resource_deadlock_would_occur = EDEADLK,
  resource_unavailable_try_again = EAGAIN,
  result_out_of_range = ERANGE,
  timed_out = ETIMEDOUT,
  too_many_files_open_in_system = ENFILE,
  too_many_files_open = EMFILE,
  too_many_links = EMLINK,
  too_many_symbolic_link_levels = ELOOP,
  wrong_protocol_type = EPROTOTYPE
};

namespace detail
{
  struct generic_code_messages
  {
    const char *msgs[256];
    SYSTEM_ERROR2_CONSTEXPR14 size_t size() const { return sizeof(msgs) / sizeof(*msgs); }
    SYSTEM_ERROR2_CONSTEXPR14 const char *operator[](int i) const { return (i < 0 || i >= static_cast<int>(size()) || nullptr == msgs[i]) ? "unknown" : msgs[i]; }  // NOLINT
    SYSTEM_ERROR2_CONSTEXPR14 generic_code_messages()
        : msgs{}
    {
      msgs[0] = "Success";
      msgs[EPERM] = "Operation not permitted";
      msgs[ENOENT] = "No such file or directory";
      msgs[ESRCH] = "No such process";
      msgs[EINTR] = "Interrupted system call";
      msgs[EIO] = "Input/output error";
      msgs[ENXIO] = "No such device or address";
      msgs[E2BIG] = "Argument list too long";
      msgs[ENOEXEC] = "Exec format error";
      msgs[EBADF] = "Bad file descriptor";
      msgs[ECHILD] = "No child processes";
      msgs[EAGAIN] = "Resource temporarily unavailable";
      msgs[ENOMEM] = "Cannot allocate memory";
      msgs[EACCES] = "Permission denied";
      msgs[EFAULT] = "Bad address";
      // msgs[ENOTBLK] = "Block device required";
      msgs[EBUSY] = "Device or resource busy";
      msgs[EEXIST] = "File exists";
      msgs[EXDEV] = "Invalid cross-device link";
      msgs[ENODEV] = "No such device";
      msgs[ENOTDIR] = "Not a directory";
      msgs[EISDIR] = "Is a directory";
      msgs[EINVAL] = "Invalid argument";
      msgs[ENFILE] = "Too many open files in system";
      msgs[EMFILE] = "Too many open files";
      msgs[ENOTTY] = "Inappropriate ioctl for device";
      msgs[ETXTBSY] = "Text file busy";
      msgs[EFBIG] = "File too large";
      msgs[ENOSPC] = "No space left on device";
      msgs[ESPIPE] = "Illegal seek";
      msgs[EROFS] = "Read-only file system";
      msgs[EMLINK] = "Too many links";
      msgs[EPIPE] = "Broken pipe";
      msgs[EDOM] = "Numerical argument out of domain";
      msgs[ERANGE] = "Numerical result out of range";
      msgs[EDEADLK] = "Resource deadlock avoided";
      msgs[ENAMETOOLONG] = "File name too long";
      msgs[ENOLCK] = "No locks available";
      msgs[ENOSYS] = "Function not implemented";
      msgs[ENOTEMPTY] = "Directory not empty";
      msgs[ELOOP] = "Too many levels of symbolic links";
      // msgs[41] = "(null)";
      msgs[ENOMSG] = "No message of desired type";
      msgs[EIDRM] = "Identifier removed";
// msgs[ECHRNG] = "Channel number out of range";
// msgs[EL2NSYNC] = "Level 2 not synchronized";
// msgs[EL3HLT] = "Level 3 halted";
// msgs[EL3RST] = "Level 3 reset";
#if 0
        msgs[48] = "Link number out of range";
        msgs[49] = "Protocol driver not attached";
        msgs[50] = "No CSI structure available";
        msgs[51] = "Level 2 halted";
        msgs[52] = "Invalid exchange";
        msgs[53] = "Invalid request descriptor";
        msgs[54] = "Exchange full";
        msgs[55] = "No anode";
        msgs[56] = "Invalid request code";
        msgs[57] = "Invalid slot";
        msgs[58] = "(null)";
        msgs[59] = "Bad font file format";
        msgs[60] = "Device not a stream";
        msgs[61] = "No data available";
        msgs[62] = "Timer expired";
        msgs[63] = "Out of streams resources";
        msgs[64] = "Machine is not on the network";
        msgs[65] = "Package not installed";
        msgs[66] = "Object is remote";
        msgs[67] = "Link has been severed";
        msgs[68] = "Advertise error";
        msgs[69] = "Srmount error";
        msgs[70] = "Communication error on send";
        msgs[71] = "Protocol error";
        msgs[72] = "Multihop attempted";
        msgs[73] = "RFS specific error";
        msgs[74] = "Bad message";
        msgs[75] = "Value too large for defined data type";
        msgs[76] = "Name not unique on network";
        msgs[77] = "File descriptor in bad state";
        msgs[78] = "Remote address changed";
        msgs[79] = "Can not access a needed shared library";
        msgs[80] = "Accessing a corrupted shared library";
        msgs[81] = ".lib section in a.out corrupted";
        msgs[82] = "Attempting to link in too many shared libraries";
        msgs[83] = "Cannot exec a shared library directly";
        msgs[84] = "Invalid or incomplete multibyte or wide character";
        msgs[85] = "Interrupted system call should be restarted";
        msgs[86] = "Streams pipe error";
        msgs[87] = "Too many users";
        msgs[88] = "Socket operation on non-socket";
        msgs[89] = "Destination address required";
        msgs[90] = "Message too long";
        msgs[91] = "Protocol wrong type for socket";
        msgs[92] = "Protocol not available";
        msgs[93] = "Protocol not supported";
        msgs[94] = "Socket type not supported";
        msgs[95] = "Operation not supported";
        msgs[96] = "Protocol family not supported";
        msgs[97] = "Address family not supported by protocol";
        msgs[98] = "Address already in use";
        msgs[99] = "Cannot assign requested address";
        msgs[100] = "Network is down";
        msgs[101] = "Network is unreachable";
        msgs[102] = "Network dropped connection on reset";
        msgs[103] = "Software caused connection abort";
        msgs[104] = "Connection reset by peer";
        msgs[105] = "No buffer space available";
        msgs[106] = "Transport endpoint is already connected";
        msgs[107] = "Transport endpoint is not connected";
        msgs[108] = "Cannot send after transport endpoint shutdown";
        msgs[109] = "Too many references: cannot splice";
        msgs[110] = "Connection timed out";
        msgs[111] = "Connection refused";
        msgs[112] = "Host is down";
        msgs[113] = "No route to host";
        msgs[114] = "Operation already in progress";
        msgs[115] = "Operation now in progress";
        msgs[116] = "Stale file handle";
        msgs[117] = "Structure needs cleaning";
        msgs[118] = "Not a XENIX named type file";
        msgs[119] = "No XENIX semaphores available";
        msgs[120] = "Is a named type file";
        msgs[121] = "Remote I/O error";
        msgs[122] = "Disk quota exceeded";
        msgs[123] = "No medium found";
        msgs[124] = "Wrong medium type";
        msgs[125] = "Operation canceled";
        msgs[126] = "Required key not available";
        msgs[127] = "Key has expired";
        msgs[128] = "Key has been revoked";
        msgs[129] = "Key was rejected by service";
        msgs[130] = "Owner died";
        msgs[131] = "State not recoverable";
        msgs[132] = "Operation not possible due to RF-kill";
        msgs[133] = "Memory page has hardware error";
        msgs[134] = "(null)";
#endif
    }
  };
}  // namespace detail

/*! The implementation of the domain for generic status codes, those mapped by `errc` (POSIX).
*/
class _generic_code_domain : public status_code_domain
{
  template <class> friend class status_code;
  using _base = status_code_domain;

public:
  //! The value type of the generic code, which is an `errc` as per POSIX.
  using value_type = errc;
  using string_ref = _base::string_ref;

public:
  //! Default constructor
  constexpr _generic_code_domain() noexcept : _base(0x746d6354f4f733e9) {}
  _generic_code_domain(const _generic_code_domain &) = default;
  _generic_code_domain(_generic_code_domain &&) = default;
  _generic_code_domain &operator=(const _generic_code_domain &) = default;
  _generic_code_domain &operator=(_generic_code_domain &&) = default;
  ~_generic_code_domain() = default;

  //! Constexpr singleton getter. Returns the address of the constexpr generic_code_domain variable.
  static inline constexpr const _generic_code_domain *get();

  virtual _base::string_ref name() const noexcept override final { return string_ref("generic domain"); }  // NOLINT
protected:
  virtual bool _failure(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    return static_cast<const generic_code &>(code).value() != errc::success;  // NOLINT
  }
  virtual bool _equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final  // NOLINT
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const generic_code &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const generic_code &>(code2);  // NOLINT
      return c1.value() == c2.value();
    }
    return false;
  }
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    return static_cast<const generic_code &>(code);  // NOLINT
  }
  virtual _base::string_ref _message(const status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const generic_code &>(code);  // NOLINT
    static SYSTEM_ERROR2_CONSTEXPR14 detail::generic_code_messages msgs;
    return string_ref(msgs[static_cast<int>(c.value())]);
  }
  virtual void _throw_exception(const status_code<void> &code) const override final  // NOLINT
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const generic_code &>(code);  // NOLINT
    throw status_error<_generic_code_domain>(c);
  }
};
//! A constexpr source variable for the generic code domain, which is that of `errc` (POSIX). Returned by `_generic_code_domain::get()`.
constexpr _generic_code_domain generic_code_domain;
inline constexpr const _generic_code_domain *_generic_code_domain::get()
{
  return &generic_code_domain;
}
// Enable implicit construction of generic_code from errc
constexpr inline generic_code make_status_code(errc c) noexcept
{
  return generic_code(c);
}


/*************************************************************************************************************/


template <class T> inline bool status_code<void>::equivalent(const status_code<T> &o) const noexcept
{
  if(_domain && o._domain)
  {
    if(_domain->_equivalent(*this, o))
    {
      return true;
    }
    if(o._domain->_equivalent(o, *this))
    {
      return true;
    }
    generic_code c1 = o._domain->_generic_code(o);
    if(c1.value() != errc::unknown && _domain->_equivalent(*this, c1))
    {
      return true;
    }
    generic_code c2 = _domain->_generic_code(*this);
    if(c2.value() != errc::unknown && o._domain->_equivalent(o, c2))
    {
      return true;
    }
  }
  // If we are both empty, we are equivalent
  if(!_domain && !o._domain)
  {
    return true;
  }
  // Otherwise not equivalent
  return false;
}
//! True if the status code's are semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2> inline bool operator==(const status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept
{
  return a.equivalent(b);
}
//! True if the status code's are not semantically equal via `equivalent()`.
template <class DomainType1, class DomainType2> inline bool operator!=(const status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept
{
  return !a.equivalent(b);
}
//! True if the status code's are semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator==(const status_code<DomainType1> &a, errc b) noexcept
{
  return a.equivalent(generic_code(b));
}
//! True if the status code's are semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator==(errc a, const status_code<DomainType1> &b) noexcept
{
  return b.equivalent(generic_code(a));
}
//! True if the status code's are not semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator!=(const status_code<DomainType1> &a, errc b) noexcept
{
  return !a.equivalent(generic_code(b));
}
//! True if the status code's are not semantically equal via `equivalent()` to the generic code.
template <class DomainType1> inline bool operator!=(errc a, const status_code<DomainType1> &b) noexcept
{
  return !b.equivalent(generic_code(a));
}

SYSTEM_ERROR2_NAMESPACE_END

#endif
