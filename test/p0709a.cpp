#include "status-code/system_error2.hpp"

#include <climits>  // for INT_MAX

// status_code<erased<intptr_t>>
using error = SYSTEM_ERROR2_NAMESPACE::error;

enum class arithmetic_errc
{
  success,
  divide_by_zero,
  integer_divide_overflows,
  not_integer_division
};

class _arithmetic_errc_domain;
using arithmetic_errc_error = SYSTEM_ERROR2_NAMESPACE::status_code<_arithmetic_errc_domain>;

class _arithmetic_errc_domain : public SYSTEM_ERROR2_NAMESPACE::status_code_domain
{
  using _base = SYSTEM_ERROR2_NAMESPACE::status_code_domain;

public:
  using value_type = arithmetic_errc;

  constexpr explicit _arithmetic_errc_domain(typename _base::unique_id_type id = 0x290f170194f0c6c7) noexcept
      : _base(id)
  {
  }
  static inline constexpr const _arithmetic_errc_domain &get();

protected:
  SYSTEM_ERROR2_CONSTEXPR20 virtual int _do_name(_vtable_name_args &args) const noexcept override final
  {
    args.ret = string_ref("arithmetic error domain");
    return 0;
  }
  SYSTEM_ERROR2_CONSTEXPR20 virtual void _do_payload_info(_vtable_payload_info_args &args) const noexcept override final
  {
    args.ret = {sizeof(value_type), sizeof(status_code_domain *) + sizeof(value_type),
                (alignof(value_type) > alignof(status_code_domain *)) ? alignof(value_type) :
                                                                        alignof(status_code_domain *)};
  }
  SYSTEM_ERROR2_CONSTEXPR20 virtual bool
  _do_failure(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override final
  {
    assert(code.domain() == *this);
    const auto &c1 = static_cast<const arithmetic_errc_error &>(code);  // NOLINT
    return c1.value() != arithmetic_errc::success;
  }
  SYSTEM_ERROR2_CONSTEXPR20 virtual bool
  _do_equivalent(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code1,
                 const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code2) const noexcept override final
  {
    return false;
  }
  SYSTEM_ERROR2_CONSTEXPR20 virtual void _do_generic_code(_vtable_generic_code_args &args) const noexcept override final
  {
    args.ret = {};
  }
  SYSTEM_ERROR2_CONSTEXPR20 virtual int _do_message(_vtable_message_args &args) const noexcept override final
  {
    assert(args.code.domain() == *this);
    const auto &c1 = static_cast<const arithmetic_errc_error &>(args.code);  // NOLINT
    switch(c1.value())
    {
    case arithmetic_errc::success:
      args.ret = _base::string_ref("success");
      return 0;
    case arithmetic_errc::divide_by_zero:
      args.ret = _base::string_ref("divide by zero");
      return 0;
    case arithmetic_errc::integer_divide_overflows:
      args.ret = _base::string_ref("integer divide overflows");
      return 0;
    case arithmetic_errc::not_integer_division:
      args.ret = _base::string_ref("not integer division");
      return 0;
    }
    args.ret = _base::string_ref("unknown");
    return 0;
  }
  SYSTEM_ERROR2_NORETURN virtual void
  _do_throw_exception(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const override final
  {
    abort();
  }
};

constexpr _arithmetic_errc_domain arithmetic_errc_domain;
inline constexpr const _arithmetic_errc_domain &_arithmetic_errc_domain::get()
{
  return arithmetic_errc_domain;
}

// Tell status code about the available implicit conversion
inline arithmetic_errc_error make_status_code(arithmetic_errc e)
{
  return arithmetic_errc_error(SYSTEM_ERROR2_NAMESPACE::in_place, e);
}


error safe_divide(int i, int j)
{
  if(j == 0)
    return arithmetic_errc::divide_by_zero;
  if(i == INT_MIN && j == -1)
    return arithmetic_errc::integer_divide_overflows;
  if(i % j != 0)
    return arithmetic_errc::not_integer_division;
  return arithmetic_errc::success;
}

int caller2(int i, int j)
{
  auto e = safe_divide(i, j);
  if(e == arithmetic_errc::divide_by_zero)
    return 0;
  if(e == arithmetic_errc::not_integer_division)
    return i / j;  // ignore
  if(e == arithmetic_errc::integer_divide_overflows)
    return INT_MIN;
  return 0;
}

int main()
{
  return caller2(5, 6);
}