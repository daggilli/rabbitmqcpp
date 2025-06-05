#ifndef __SENDCONCEPT_H__
#define __SENDCONCEPT_H__

#include <concepts>
#include <string>
#include <type_traits>

namespace RabbitMQCpp {
  template <typename>
  struct function_traits;

  template <typename R, typename C, typename... A>
  struct function_traits<R (C::*)(A...)> {
    using return_type = R;
  };

  template <typename R, typename C, typename... A>
  struct function_traits<R (C::*)(A...) const> {
    using return_type = R;
  };

  template <typename T>
  concept HasVoidSendMember = (requires { &T::send; } &&
                               std::is_member_function_pointer<decltype(&T::send)>::value) &&
                              std::is_same_v<void, typename function_traits<decltype(&T::send)>::return_type>;
};  // namespace RabbitMQCpp
#endif
