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

#ifndef STATUS_CODE_STATUS_CODE_HPP
#define STATUS_CODE_STATUS_CODE_HPP

#include "config.hpp"

#include <cassert>
#include <cerrno>
#include <stdexcept>
#include <type_traits>

//! Namespace for status_code
namespace system_error2
{
  template <class DomainType> class status_code;
  class _generic_code_domain;
  using generic_code = status_code<_generic_code_domain>;

  namespace detail
  {
    inline STATUS_CODE_CONSTEXPR14 size_t cstrlen(const char *str)
    {
      const char *end = nullptr;
      for(end = str; *end != 0; ++end)
        ;
      return end - str;
    }
  }

  /*! Abstract base class for a domain for a status code.
  */
  class status_code_domain
  {
    template <class DomainType> friend class status_code;

  public:
    //! Type of the unique id for this domain.
    using unique_id_type = unsigned long long;
    //! Thread safe reference to a message string.
    class string_ref
    {
    public:
      //! The value type
      using value_type = const char;
      //! The size type
      using size_type = size_t;
      //! The pointer type
      using pointer = const char *;
      //! The const pointer type
      using const_pointer = const char *;
      //! The iterator type
      using iterator = const char *;
      //! The const iterator type
      using const_iterator = const char *;

    protected:
      pointer _begin{}, _end{};
      void *_state[2]{};  // at least the size of a shared_ptr

      string_ref() = default;
      //! Invoked to perform a copy construction
      virtual void _copy(string_ref *dest) const & { new(dest) string_ref(_begin, _end, _state[0], _state[1]); }
      //! Invoked to perform a move construction
      virtual void _move(string_ref *dest) && noexcept { new(dest) string_ref(_begin, _end, _state[0], _state[1]); }

    public:
      //! Construct from a C string literal
      STATUS_CODE_CONSTEXPR14 explicit string_ref(const char *str)
          : _begin(str)
          , _end(str + detail::cstrlen(str))
      {
      }
      //! Construct from a set of data
      constexpr string_ref(pointer begin, pointer end, void *state0, void *state1)
          : _begin(begin)
          , _end(end)
          , _state{state0, state1}
      {
      }
      //! Copy construct the derived implementation.
      string_ref(const string_ref &o) { o._copy(this); }
      //! Move construct the derived implementation.
      string_ref(string_ref &&o) noexcept { std::move(o)._move(this); }
      //! Copy assignment
      string_ref &operator=(const string_ref &o)
      {
        this->~string_ref();
        o._copy(this);
        return *this;
      }
      //! Public moving not permitted.
      string_ref &operator=(string_ref &&o) noexcept
      {
        this->~string_ref();
        std::move(o)._move(this);
        return *this;
      }
      //! Destruction permitted.
      virtual ~string_ref() { _begin = _end = nullptr; }

      //! Returns whether the reference is empty or not
      bool empty() const noexcept { return _begin == _end; }
      //! Returns the size of the string
      size_type size() const { return _end - _begin; }
      //! Returns a null terminated C string
      value_type *c_str() const { return _begin; }
      //! Returns the beginning of the string
      iterator begin() { return _begin; }
      //! Returns the beginning of the string
      const_iterator begin() const { return _begin; }
      //! Returns the beginning of the string
      const_iterator cbegin() const { return _begin; }
      //! Returns the end of the string
      iterator end() { return _end; }
      //! Returns the end of the string
      const_iterator end() const { return _end; }
      //! Returns the end of the string
      const_iterator cend() const { return _end; }
    };

  private:
    unique_id_type _id;

  protected:
    //! Use https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h to get a random 64 bit id
    constexpr explicit status_code_domain(unique_id_type id) noexcept : _id(id) {}
    //! No public copying at type erased level
    status_code_domain(const status_code_domain &) = default;
    //! No public moving at type erased level
    status_code_domain(status_code_domain &&) = default;
    //! No public assignment at type erased level
    status_code_domain &operator=(const status_code_domain &) = default;
    //! No public assignment at type erased level
    status_code_domain &operator=(status_code_domain &&) = default;
    //! No public destruction at type erased level
    ~status_code_domain() = default;

  public:
    //! True if the unique ids match.
    constexpr bool operator==(const status_code_domain &o) const noexcept { return _id == o._id; }
    //! True if the unique ids do not match.
    constexpr bool operator!=(const status_code_domain &o) const noexcept { return _id != o._id; }
    //! True if this unique is lower than the other's unique id.
    constexpr bool operator<(const status_code_domain &o) const noexcept { return _id < o._id; }

    //! Returns the unique id used to identify identical category instances.
    constexpr unique_id_type id() const noexcept { return _id; }
    //! Name of this category.
    virtual string_ref name() const noexcept = 0;

  protected:
    //! True if code means failure.
    virtual bool _failure(const status_code<void> &code) const noexcept = 0;
    //! True if code is (potentially non-transitively) equivalent to another code in another domain.
    virtual bool _equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept = 0;
    //! Returns the generic code closest to this code, if any.
    virtual generic_code _generic_code(const status_code<void> &code) const noexcept = 0;
    //! Return a reference to a string textually representing a code.
    virtual string_ref _message(const status_code<void> &code) const noexcept = 0;
    //! Throw a code as a C++ exception.
    virtual void _throw_exception(const status_code<void> &code) const = 0;
  };


  /*! A tag for an erased value type for `status_code<T>`.
  Available only if `ErasedType` is an integral type.
  */
  template <class ErasedType,  //
            typename std::enable_if<std::is_integral<ErasedType>::value, bool>::type = true>
  struct erased
  {
    using value_type = ErasedType;
  };
  /*! A type erased lightweight status code reflecting empty, success, or failure.
  */
  template <> class status_code<void>
  {
    template <class T> friend class status_code;

  public:
    //! The type of the domain.
    using domain_type = status_code_domain;
    //! The type of the status code.
    using value_type = void;
    //! The type of a reference to a message string.
    using string_ref = typename domain_type::string_ref;

  protected:
    const status_code_domain *_domain{nullptr};

  protected:
    //! No default construction at type erased level
    status_code() = default;
    //! No public copying at type erased level
    status_code(const status_code &) = default;
    //! No public moving at type erased level
    status_code(status_code &&) = default;
    //! No public assignment at type erased level
    status_code &operator=(const status_code &) = default;
    //! No public assignment at type erased level
    status_code &operator=(status_code &&) = default;
    //! No public destruction at type erased level
    ~status_code() = default;

    //! Used to construct a non-empty type erased status code
    constexpr explicit status_code(const status_code_domain *v)
        : _domain(v)
    {
    }

  public:
    //! Return the status code domain.
    constexpr const status_code_domain &domain() const noexcept { return *_domain; }
    //! True if the status code is empty.
    constexpr bool empty() const noexcept { return _domain == nullptr; }

    //! Return a reference to a string textually representing a code.
    string_ref message() const noexcept { return _domain ? _domain->_message(*this) : string_ref("(empty)"); }
    //! True if code means success.
    bool success() const noexcept { return _domain ? !_domain->_failure(*this) : false; }
    //! True if code means failure.
    bool failure() const noexcept { return _domain ? _domain->_failure(*this) : false; }
    //! True if code is strictly (and potentially non-transitively) equivalent to another code in another domain.
    template <class T> bool strictly_equivalent(const status_code<T> &o) const noexcept
    {
      if(_domain && o._domain)
        return _domain->_equivalent(*this, o);
      // If we are both empty, we are equivalent
      if(!_domain && !o._domain)
        return true;
      // Otherwise not equivalent
      return false;
    }
    //! True if code is equivalent, by any means, to another code in another domain (guaranteed transitive).
    template <class T> inline bool equivalent(const status_code<T> &o) const noexcept;
    //! Throw a code as a C++ exception.
    void throw_exception() const { _domain->_throw_exception(*this); }
  };

  /*! A lightweight status code reflecting empty, success, or failure.
  */
  template <class DomainType> class status_code : public status_code<void>
  {
    template <class T> friend class status_code;
    using _base = status_code<void>;

  public:
    //! The type of the domain.
    using domain_type = DomainType;
    //! The type of the status code.
    using value_type = typename domain_type::value_type;
    //! The type of a reference to a message string.
    using string_ref = typename domain_type::string_ref;

  protected:
    value_type _value{};

  public:
    //! Default construction to empty
    status_code() = default;
    //! Copy constructor
    status_code(const status_code &) = default;
    //! Move constructor
    status_code(status_code &&) = default;
    //! Copy assignment
    status_code &operator=(const status_code &) = default;
    //! Move assignment
    status_code &operator=(status_code &&) = default;

    //! Implicit construction from any type where an ADL discovered `make_status_code(T &&)` returns a `status_code`.
    template <class T,  //
              typename std::enable_if<std::is_same<typename std::decay<decltype(make_status_code(std::declval<T>()))>::type, status_code>::value, bool>::type = true>
    constexpr explicit status_code(T &&v) noexcept(noexcept(make_status_code(std::declval<T>())))
        : status_code(make_status_code(std::forward<T>(v)))
    {
    }
    //! Explicit construction from a `value_type`.
    constexpr explicit status_code(const value_type &v) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : _value(v)
        , _base(domain_type::get())
    {
    }
    /*! Explicit construction from an erased status code. Available only if
    `value_type` is trivially destructible and `sizeof(value_type) <= sizeof(ErasedType)`.
    Does not check if domains are equal.
    */
    template <class ErasedType,  //
              typename std::enable_if<std::is_trivially_copyable<value_type>::value && sizeof(value_type) <= sizeof(ErasedType), bool>::type = true>
    constexpr explicit status_code(const status_code<erased<ErasedType>> &v)
        : status_code(reinterpret_cast<const value_type &>(v._value))
    {
      assert(v.domain() == domain());
    }

    // Replace the type erased implementations with type aware implementations for better codegen
    //! Return the status code domain.
    constexpr const domain_type &domain() const noexcept { return *static_cast<const domain_type *>(this->_domain); }
    //! Return a reference to a string textually representing a code.
    string_ref message() const noexcept { return this->_domain ? string_ref(domain()._message(*this)) : string_ref("(empty)"); }

    //! Reset the code to empty.
    STATUS_CODE_CONSTEXPR14 void clear() { *this = status_code(); }

#if __cplusplus >= 201400 || _MSC_VER >= 1910 /* VS2017 */
    //! Return a reference to the `value_type`.
    constexpr value_type &value() & noexcept { return _value; }
    //! Return a reference to the `value_type`.
    constexpr value_type &&value() && noexcept { return _value; }
#endif
    //! Return a reference to the `value_type`.
    constexpr const value_type &value() const &noexcept { return _value; }
    //! Return a reference to the `value_type`.
    constexpr const value_type &&value() const &&noexcept { return _value; }
  };

  /*! Type erased status_code, but copyable/movable/destructible.
  */
  template <class ErasedType> class status_code<erased<ErasedType>> : public status_code<void>
  {
    template <class T> friend class status_code;
    using _base = status_code<void>;

  public:
    //! The type of the domain (void, as it is erased).
    using domain_type = void;
    //! The type of the erased status code.
    using value_type = ErasedType;
    //! The type of a reference to a message string.
    using string_ref = typename _base::string_ref;

  protected:
    value_type _value{};

  public:
    //! Default construction to empty
    status_code() = default;
    //! Copy constructor
    status_code(const status_code &) = default;
    //! Move constructor
    status_code(status_code &&) = default;
    //! Copy assignment
    status_code &operator=(const status_code &) = default;
    //! Move assignment
    status_code &operator=(status_code &&) = default;

    //! Explicit copy construction from any other status code if its type is trivially copyable and it would fit into our storage
    template <class DomainType,  //
              typename std::enable_if<std::is_trivially_copyable<typename DomainType::value_type>::value && sizeof(value_type) >= sizeof(typename DomainType::value_type), bool>::type = true>
    constexpr explicit status_code(const status_code<DomainType> &v) noexcept : _base(v), _value(reinterpret_cast<const value_type &>(v.value()))
    {
    }
    //! Return the erased `value_type` by value.
    constexpr value_type value() const noexcept { return _value; }
  };


  /*! Exception type representing a thrown status_code

  */
  template <class DomainType> class status_error : public std::runtime_error
  {
    status_code<DomainType> _code;

  public:
    //! The type of the status domain
    using domain_type = DomainType;
    //! The type of the status code
    using status_code_type = status_code<DomainType>;

    //! Constructs an instance
    status_error(status_code<DomainType> code)
        : std::runtime_error(code.message().c_str())
        , _code(std::move(code))
    {
    }
    //! Returns a reference to the code
    const status_code_type &code() const & { return _code; }
    //! Returns a reference to the code
    status_code_type &code() & { return _code; }
    //! Returns a reference to the code
    const status_code_type &&code() const && { return _code; }
    //! Returns a reference to the code
    status_code_type &&code() && { return _code; }
  };


  /*************************************************************************************************************/


  //! The generic error coding (POSIX)
  enum class errc : int
  {
    success = 0,

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
      STATUS_CODE_CONSTEXPR14 size_t size() const { return sizeof(msgs) / sizeof(*msgs); }
      STATUS_CODE_CONSTEXPR14 const char *operator[](int i) const { return (i < 0 || i >= (int) size() || !msgs[i]) ? "unknown" : msgs[i]; }
      STATUS_CODE_CONSTEXPR14 generic_code_messages()
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
  }

  /*! The domain for generic status codes, those mapped by `errc`.
  */
  class _generic_code_domain : public status_code_domain
  {
    template <class> friend class status_code;
    using _base = status_code_domain;

  public:
    //! The value type of the generic code, which is an `errc` as per POSIX.
    using value_type = errc;
    //! Thread safe reference to a message string, reimplemented to implement finality for better codegen
    class string_ref : public status_code_domain::string_ref
    {
    protected:
      virtual void _copy(_base::string_ref *dest) const & override final { new(static_cast<string_ref *>(dest)) string_ref(this->_begin, this->_end, this->_state[0], this->_state[1]); }
      virtual void _move(_base::string_ref *dest) && noexcept override final { new(static_cast<string_ref *>(dest)) string_ref(this->_begin, this->_end, this->_state[0], this->_state[1]); }
    public:
      using status_code_domain::string_ref::string_ref;
      // Allow explicit cast up
      explicit string_ref(_base::string_ref v) { static_cast<string_ref &&>(v)._move(this); }
      ~string_ref() override final = default;
    };

  public:
    //! Default constructor
    constexpr _generic_code_domain()
        : status_code_domain(0x746d6354f4f733e9)
    {
    }
    _generic_code_domain(const _generic_code_domain &) = default;
    _generic_code_domain(_generic_code_domain &&) = default;
    _generic_code_domain &operator=(const _generic_code_domain &) = default;
    _generic_code_domain &operator=(_generic_code_domain &&) = default;
    ~_generic_code_domain() = default;

    //! Singleton getter
    static inline constexpr const _generic_code_domain *get();

    virtual _base::string_ref name() const noexcept override final { return string_ref("generic domain"); }
  protected:
    virtual bool _failure(const status_code<void> &code) const noexcept override final
    {
      assert(code.domain() == *this);
      return static_cast<const generic_code &>(code).value() != errc::success;
    }
    virtual bool _equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final
    {
      assert(code1.domain() == *this);
      const auto &c1 = static_cast<const generic_code &>(code1);
      if(code2.domain() == *this)
      {
        const auto &c2 = static_cast<const generic_code &>(code2);
        return c1.value() == c2.value();
      }
      return false;
    }
    virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final
    {
      assert(code.domain() == *this);
      return static_cast<const generic_code &>(code);
    }
    virtual _base::string_ref _message(const status_code<void> &code) const noexcept override final
    {
      assert(code.domain() == *this);
      const auto &c = static_cast<const generic_code &>(code);
      static STATUS_CODE_CONSTEXPR14 detail::generic_code_messages msgs;
      return string_ref(msgs[static_cast<int>(c.value())]);
    }
    virtual void _throw_exception(const status_code<void> &code) const override final
    {
      assert(code.domain() == *this);
      const auto &c = static_cast<const generic_code &>(code);
      switch(c.value())
      {
      case errc::invalid_argument:
        throw std::invalid_argument(_message(code).c_str());
      case errc::argument_out_of_domain:
        throw std::domain_error(_message(code).c_str());
      case errc::argument_list_too_long:
      case errc::filename_too_long:
      case errc::no_buffer_space:
        throw std::length_error(_message(code).c_str());
      case errc::result_out_of_range:
        throw std::range_error(_message(code).c_str());
      }
      throw status_error<_generic_code_domain>(c);
    }
  };
  constexpr _generic_code_domain generic_code_domain;
  inline constexpr const _generic_code_domain *_generic_code_domain::get() { return &generic_code_domain; }


  /*************************************************************************************************************/


  template <class T> inline bool status_code<void>::equivalent(const status_code<T> &o) const noexcept
  {
    if(_domain && o._domain)
    {
      if(_domain->_equivalent(*this, o))
        return true;
      if(o._domain->_equivalent(o, *this))
        return true;
      generic_code c1 = o._domain->_generic_code(o);
      if(_domain->_equivalent(*this, c1))
        return true;
      generic_code c2 = _domain->_generic_code(*this);
      if(o._domain->_equivalent(o, c2))
        return true;
    }
    // If we are both empty, we are equivalent
    if(!_domain && !o._domain)
      return true;
    // Otherwise not equivalent
    return false;
  }
  //! True if the status code's are semantically equal via `equivalent()`.
  template <class DomainType1, class DomainType2> inline bool operator==(const status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept { return a.equivalent(b); }
  //! True if the status code's are not semantically equal via `equivalent()`.
  template <class DomainType1, class DomainType2> inline bool operator!=(const status_code<DomainType1> &a, const status_code<DomainType2> &b) noexcept { return !a.equivalent(b); }
  //! True if the status code's are semantically equal via `equivalent()` to the generic code.
  template <class DomainType1, class DomainType2> inline bool operator==(const status_code<DomainType1> &a, errc b) noexcept { return a.equivalent(generic_code(b)); }
  //! True if the status code's are semantically equal via `equivalent()` to the generic code.
  template <class DomainType1, class DomainType2> inline bool operator==(errc a, const status_code<DomainType1> &b) noexcept { return b.equivalent(generic_code(a)); }
  //! True if the status code's are not semantically equal via `equivalent()` to the generic code.
  template <class DomainType1, class DomainType2> inline bool operator!=(const status_code<DomainType1> &a, errc b) noexcept { return !a.equivalent(generic_code(b)); }
  //! True if the status code's are not semantically equal via `equivalent()` to the generic code.
  template <class DomainType1, class DomainType2> inline bool operator!=(errc a, const status_code<DomainType1> &b) noexcept { return !b.equivalent(generic_code(a)); }
}  // namespace

#endif
