#pragma once
#include <fmt/core.h>
namespace tcp {
template <typename Callback> int check(int ret, Callback &&callback = nullptr) {
  if (ret < 0) {
    fmt::print("[ERROR]: {}\n", std::strerror(errno));
    if (callback) {
      callback();
    }
    return ret;
  }
  return 0;
}
} // namespace tcp