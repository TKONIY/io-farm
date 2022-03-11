#include "block_server.h"
int main() {
  tcp::block_server server;
  server.on_message([&](const auto &msg) {
    fmt::print("[DEBUG] server recv {}\n", msg);

    // block call

    server.set_response(msg.c_str());
  });
  server.run("0.0.0.0", 1111);
  return 0;
}