#include "common/server.h"
#include "common/utils.h"
#include <arpa/inet.h>
#include <functional>
#include <sys/socket.h>
#include <unistd.h>

namespace tcp {
class block_server : public tcp::server<std::function<void(const std::string &)>> {
public:
  void run(const char *ip, uint16_t port) override final;
  void set_response(const char *resp) override final;

private:
  std::string _read_buffer{};
  std::string _write_buffer{};
  bool _responsed{false};

  void accept_and_handle(int server_fd);
};
} // namespace tcp

void tcp::block_server::run(const char *ip, uint16_t port) {
  int server_fd{};

  // socket()
  tcp::check(server_fd = socket(AF_INET, SOCK_STREAM, 0),
             []() { throw std::runtime_error("socket() error"); });

  // bind()
  sockaddr_in server_addr{};
  inet_pton(AF_INET, ip, &server_addr.sin_addr.s_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  tcp::check(bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)),
             []() { throw std::runtime_error("bind() error"); });

  // listen
  tcp::check(listen(server_fd, N_LISTEN),
             []() { throw std::runtime_error("listen() error"); });

  // loop, can be dispatched to each threads
  while (true) {
    accept_and_handle(server_fd);
    _read_buffer.clear();
    _write_buffer.clear();
  }
}

void tcp::block_server::set_response(const char *resp) {
  _write_buffer = resp;
  _responsed = true;
}

void tcp::block_server::accept_and_handle(int server_fd) {
  sockaddr_in client_addr{};
  socklen_t addr_len = sizeof(client_addr);

  int client_fd{};
  tcp::check(client_fd = accept(server_fd, (sockaddr *)&client_addr, &addr_len),
             []() { throw std::runtime_error("accept() failed"); });

  // run to complete
  _responsed = false;
  while (!_responsed) {
    // read
    char buffer[1024];
    int n_read{};
    tcp::check(n_read = read(client_fd, buffer, 1024),
               []() { throw std::runtime_error("read() failed"); });
    if (n_read == 0) {
      fmt::print("[DEBUG] socket closed from other side\n");
      close(client_fd);
      return;
    }

    // Append to buffer. This costs one more unnessary copy
    // which is a BAD practice. Please refer moduo's buffer
    _read_buffer.append(buffer, n_read);

    fmt::print("read buffer {}", _read_buffer);

    // callback
    _handler(_read_buffer);
  }

  // send response
  int n_all_write = 0;
  size_t n_all = _write_buffer.size();
  const char *buffer = _write_buffer.c_str();
  while (n_all_write < n_all) {
    int n_write{};
    tcp::check(n_write = write(client_fd, buffer + n_all_write, n_all - n_all_write),
               []() { throw std::runtime_error("write() failed"); });
    n_all_write += n_write;
  }

  // close connection
  fmt::print("[DEBUG] socket closed by server\n");
  close(client_fd);
  return;
}
