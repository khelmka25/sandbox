#pragma once

#include <type_traits>

#include <glad/glad.h>
#include <iostream>

namespace utils {

template <typename T>
concept HasUnderlyingType = requires(T) {
  { std::is_integral<std::underlying_type_t<T>>::value->std::true_type::value };
};

template <typename T>
  requires HasUnderlyingType<T>
inline decltype(auto) to_underlying(T t) noexcept {
  return static_cast<std::underlying_type_t<T>>(t);
}

template <typename T>
  requires HasUnderlyingType<T>
inline decltype(auto) to_underlying_ptr(T* t) noexcept {
  return reinterpret_cast<std::add_pointer_t<std::underlying_type_t<T>>>(t);
}

inline void checkError() {
  unsigned result;
  while (result = glGetError(), result != GL_NO_ERROR) {
    std::cout << "Error: " << result;
  }
}

}  // namespace utils