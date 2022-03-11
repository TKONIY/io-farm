#pragma once

#include <stdint.h>

namespace tcp {
template <typename handler_t> class server {
protected:
  handler_t _handler{};
  static constexpr int N_LISTEN = 10;

public:
  // run the server and blocked
  [[noreturn]] virtual void run(const char *ip, uint16_t port) = 0;

  // response message to client
  // mark the end of a connection
  virtual void set_response(const char *) = 0;

  // register the handler
  void on_message(handler_t &&handler) { _handler = handler; }
};

} // namespace tcp