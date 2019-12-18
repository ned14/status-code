
# Worked example writing a custom status code domain and code

In this worked example, we will implement a custom status code
domain whose code carries as payload a `std::exception_ptr` i.e.
a previously thrown C++ exception object. This will be somewhat
challenging as we shall be keeping our custom `status_code` trivially copyable
in order to preserve the type erasability into `status_code<erased<intptr_t>>`,
which in turn creates the problem of managing the lifetime of the
`std::exception_ptr`.

The way we will solve this is to keep a threadsafe global register of
`std::exception_ptr` instances. This could, of course, also be thread local
or use a wide variety of other methods of storage, but for here
we shall be keeping it simple.

## Storing the `std::exception_ptr` instances

```c++
#include "system_error2.hpp"

#include <exception>
#include <mutex>

static constexpr size_t max_exception_ptrs = 16;


using namespace SYSTEM_ERROR2_NAMESPACE;

struct exception_ptr_storage_t
{
  using index_type = unsigned int;

  mutable std::mutex lock;
  std::exception_ptr items[max_exception_ptrs];
  index_type idx{0};

  std::exception_ptr operator[](index_type i) const
  {
    std::lock_guard h(lock);
    return (idx - i < max_exception_ptrs) ? items[i % max_exception_ptrs] : std::exception_ptr();
  }
  index_type add(std::exception_ptr p)
  {
    std::lock_guard h(lock);
    items[idx] = std::move(p);
    return idx++;
  }
};
inline exception_ptr_storage_t exception_ptr_storage;
```

The above is a fairly standard way of implementing a threadsafe
global register of instances. We keep `max_exception_ptrs`
instances, using modulus to convert some index id into a slot.
We detect when an index id refers to an instance whose slot
has been used by a more recent addition, and for that return
a null instance.

## Declaring `thrown_exception_code` and its domain

```c++
// Alias our new status code to its domain
class _thrown_exception_domain;
using thrown_exception_code = status_code<_thrown_exception_domain>;
```

As a general rule, one usually does not need to create a custom
status code implementation, typedefing it to a custom domain is
almost always sufficient. If you'd like status code to implicitly
construct from some type, you can write an ADL discovered function
called `make_status_code(T)` where `T` is the type you want
implicit construction from. Place the ADL discovered function into
the same namespace as `T`, and status code will find it. All
that said, you can of course inherit from `status_code<YourDomain>`
if you'd like, this can be useful if you have particularly custom
constructors.

```c++
class _thrown_exception_domain : public status_code_domain
{
  // We permit status code to call our protected functions
  template <class DomainType> friend class status_code;
  using _base = status_code_domain;

public:
  // Our value type is the index into the exception_ptr storage
  using value_type = exception_ptr_storage_t::index_type;
```

status code does not *require* trivial copyability, but you are highly
advised to use a value type which is trivially copyable as then
the compiler can store the code in CPU registers rather than memory. This
leads to higher quality codegen.

It also enables type erasure into the copyable and moveable form
of status code i.e. `status_code<erased<T>>`. If your value type
is not trivially copyable, you only have the immutable form of
type erased status code available `status_code<void>`.

```c++
  // std::exception::what() returns const char *, so the default string_ref is sufficient
  using _base::string_ref;

  // Always use https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h to create a
  // unique 64 bit value for every unique domain you create!
  constexpr _thrown_exception_domain() noexcept : _base(0xb766b5e50597a655) {}
```

Code domains use a unique 64 bit id to identify themselves. This
allows multiple singletons to exist and correctly compare equal.
The all-constexpr construction and destruction of the code domain
ensures that the compiler will assume that the domain can be assumed
to not have unique instancing i.e. the id comparison will generally
be compiled out as it is considered part of the domain's type.

```c++
  // Default all the copy, move and destruct. This makes the type 100% constexpr in every way
  // which in turns allows the compiler to assume it will not be instantiated at runtime.
  _thrown_exception_domain(const _thrown_exception_domain &) = default;
  _thrown_exception_domain(_thrown_exception_domain &&) = default;
  _thrown_exception_domain &operator=(const _thrown_exception_domain &) = default;
  _thrown_exception_domain &operator=(_thrown_exception_domain &&) = default;
  ~_thrown_exception_domain() = default;

  // Fetch a constexpr instance of this domain
  static inline constexpr const _thrown_exception_domain &get();

  // Return the name of this domain
  virtual _base::string_ref name() const noexcept override final { return _base::string_ref("thrown exception"); }
```

You can customise your implementation of `string_ref` to implement
reference counting or other lifetime management of strings returned
by the domain. This lets you fetch a string in the current locale
into a memory allocation, and once nobody is using it, it can be
deallocated. The built-in domains of `posix_code`, `win32_code` etc
do exactly this, so examine their source code for an example of how
to implement atomics-based threadsafe reference counting.

The `name()` is the first pure virtual function which all implementations
of `status_code_domain` must implement.

```c++
protected:
  // This internal routine maps an exception ptr onto a generic_code
  // It is surely hideously slow, but that's all relative in the end
  static errc _generic_code(value_type c) noexcept
  {
    try
    {
      std::exception_ptr e = exception_ptr_storage[c];
      if(!e)
        return errc::unknown;
      std::rethrow_exception(e);
    }
    catch(const std::invalid_argument & /*unused*/)
    {
      return errc::invalid_argument;
    }
    catch(const std::domain_error & /*unused*/)
    {
      return errc::argument_out_of_domain;
    }
    catch(const std::length_error & /*unused*/)
    {
      return errc::argument_list_too_long;
    }
    catch(const std::out_of_range & /*unused*/)
    {
      return errc::result_out_of_range;
    }
    catch(const std::logic_error & /*unused*/) /* base class for this group */
    {
      return errc::invalid_argument;
    }
    catch(const std::system_error &e) /* also catches ios::failure */
    {
      return static_cast<errc>(e.code().value());
    }
    catch(const std::overflow_error & /*unused*/)
    {
      return errc::value_too_large;
    }
    catch(const std::range_error & /*unused*/)
    {
      return errc::result_out_of_range;
    }
    catch(const std::runtime_error & /*unused*/) /* base class for this group */
    {
      return errc::resource_unavailable_try_again;
    }
    catch(const std::bad_alloc & /*unused*/)
    {
      return errc::not_enough_memory;
    }
    catch(...)
    {
    }
    return errc::unknown;
  }
```

`errc` is a slight superset of `std::errc`, but ia otherwise identical.
It has a special place in `<system_error2>` because it is the value type
of the `generic_code` status code, and all other status codes are expected
to "speak" `generic_code`. We therefore need a way of mapping the thrown
C++ exception into a `errc`, so we rethrow it and catch all of the STL
exception types, returning their equivalent `errc` code. This function
will be used in two parts of the code domain's implementation shortly.

```c++
  // Always true, as exception_ptr always represents failure
  virtual bool _do_failure(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    return true;
  }
```

This is the second pure virtual function which all implementations
of `status_code_domain` must implement. Each code might have multiple
success or failure states, and this function must return true if the
code given represents a failure. In this domain's case, all thrown
exceptions represent failure. So we always return true.

```c++
  // True if the exception ptr is equivalent to some other status code
  virtual bool _do_equivalent(const status_code<void> &code1, const status_code<void> &code2) const noexcept override final
  {
    assert(code1.domain() == *this);
    const auto &c1 = static_cast<const thrown_exception_code &>(code1);
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const thrown_exception_code &>(code2);
      // Always perform literal comparison when domains are equal. The fallback
      // semantic comparison of converting both to generic_code and comparing
      // will handle semantic comparison of the same domain.
      return c1.value() == c2.value();
    }
    // If anything in your coding matches anything in errc, you should match it here
    if(code2.domain() == generic_code_domain)
    {
      const auto &c2 = static_cast<const generic_code &>(code2);
      if(c2.value() == _generic_code(c1.value()))
      {
        return true;
      }
    }
    return false;
  }
```

This third pure virtual function implementation is the heart of the
implementation of semantic comparisons. Semantic comparisons are implemented
as follows:

    1. If the two codes are empty, they are equivalent.
    2. Ask the first code's domain if its code is `_equivalent()` to the second code.
    3. Ask the second code's domain if its code is `_equivalent()` to the first code.
    4. Map the second code to its nearest generic code, and ask the first
    code's domain if its code is `_equivalent()` to the nearest generic code.
    5. Map the first code to its nearest generic code, and ask the second
    code's domain if its code is `_equivalent()` to the nearest generic code.

In `_equivalent()`, we always first check if the other domain is us,
if so we do a literal comparison knowing that the generic mapping
fallback will handle the semantic comparison of codes of the same domain.
If the other domain is the generic code domain, we map our thrown
exception to `errc` as described earlier, and if that is the same
we return true.

You can of course also do matching on any other custom domain of
your choice. For example, `com_code` also recognises `win32_code` and
`nt_code` during `_equivalent()`. You can examine its source code if
you'd like to know more.

```c++
  // Called as a fallback if _equivalent() fails
  virtual generic_code _generic_code(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c1 = static_cast<const thrown_exception_code &>(code);
    return generic_code(_generic_code(c1.value()));
  }
```

This fourth of the pure virtual functions in `status_code_domain`
is fairly self explanatory.

```c++
  // Extract the what() from the exception
  virtual _base::string_ref _do_message(const status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const thrown_exception_code &>(code);
    try
    {
      std::exception_ptr e = exception_ptr_storage[c.value()];
      if(!e)
        return _base::string_ref("expired");
      std::rethrow_exception(e);
    }
    catch(const std::exception &x)
    {
      return _base::string_ref(x.what());
    }
    catch(...)
    {
      return _base::string_ref("unknown thrown exception");
    }
  }
```

Usefully, `std::exception` provides a `what()` function returning
a `const char *`. So we don't need to implement extra lifetime
management as the exception ptr holds open the message string for
us. Therefore we simply rethrow the exception, catch any 
`std::exception` implementations and return their `what()` string.
A nicer fallback might be to extract the type of the exception from
`typeid()` and return that, that is left to the reader to implement.

```c++
  // Throw the code as a C++ exception
  virtual void _do_throw_exception(const status_code<void> &code) const override final
  {
    assert(code.domain() == *this);
    const auto &c = static_cast<const thrown_exception_code &>(code);
    std::exception_ptr e = exception_ptr_storage[c.value()];
    std::rethrow_exception(e);
  }
};
```

The final pure virtual function which must be implemented is how
best to throw the status code as a C++ exception. In this case
this is very easy, we rethrow the thrown exception.

```c++
//! A constexpr source variable for the throw exception code domain to return via get()
constexpr inline _thrown_exception_domain thrown_exception_domain;
inline constexpr const _thrown_exception_domain &_thrown_exception_domain::get()
{
  return thrown_exception_domain;
}

// Helper to construct a thrown_exception_code from a std::exception_ptr
inline thrown_exception_code make_status_code(std::exception_ptr ep)
{
  return thrown_exception_code(in_place, exception_ptr_storage.add(std::move(ep)));
}
```

Finally, we need to implement the domain's static `get()` function
which returns a constexpr source of the domain from which codes of
this domain can initialise their reference to their domain. We
also declare a helper function `make_status_code()` which will
store an exception ptr into the global threadsafe storage and
return a `thrown_exception_code` referencing that stored thrown
exception.

Let's quickly see a use case:

```c++
int main()
{
  thrown_exception_code tec(make_status_code(std::make_exception_ptr(std::bad_alloc())));
  system_code sc(tec);
  printf("Thrown exception code has message %s\n", sc.message().c_str());
  printf("Thrown exception code == errc::not_enough_memory = %d\n", sc == errc::not_enough_memory);
  return 0;
}
```

Because the size of `thrown_exception_code` is not bigger than
`system_code` and the value type is trivially copyable, `system_code`
which is a type erased form of status code will accept construction
from `thrown_exception_code`. This allows your functions to return
`system_code`, thus allowing multiple code domains to be returned.

If you would like to see this custom status code and domain in action,
you can find it in an online C++ compiler at
[https://wandbox.org/permlink/0BKDWa7yk62uIFXc](https://wandbox.org/permlink/0BKDWa7yk62uIFXc).

You can also find the source code for the above in example/thrown_exception.cpp.
