#include "common/server.h"
#include "common/utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <functional>
#include <stdint.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

namespace tcp {
class single_reactor_server
    : public tcp::server<std::function<void(const std::string &)>> {
public:
  void run(const char *ip, uint16_t port) override final;

  // Closed connection when set_response() called and write_buffer's are empty.
  void set_response(const char *resp) override final;

  // TODO: Provide async POSIX io interfaces for user.
  // Origin return values are passed to callback functions as parameters.
  void read(int fd, void *buf, size_t count, std::function<void(ssize_t)> &&);
  void write(int fd, const void *buf, size_t count, std::function<void(ssize_t)> &&);
  void connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen,
               std::function<void(int)> &&);

private:
  // Single Thread. Can be easily extended to multi-thread.
  std::unordered_map<int, std::string> _read_buffer_map{};
  std::unordered_map<int, std::string> _write_buffer_map{};
  std::unordered_map<int, bool> _responsed{};
  bool _responsed{false};
  int _curr_fd{}; // for handler
  int _epoll_fd{};

  std::string &get_read_buffer(int fd);
  std::string &get_write_buffer(int fd);

  // static
  static int set_socket_nonblock(int fd);
  static int epoll_ctl(int epfd, int op, int fd, uint32_t event);
  static const int MAX_BUFFER_SIZE{4096};
  static const int MAX_EVENTS{UINT16_MAX * 4};
  static const int MAX_EPOLL_FD{UINT16_MAX * 4};
};
} // namespace tcp

void tcp::single_reactor_server::run(const char *ip, uint16_t port) {
  int server_fd{};

  // socket()
  tcp::check(server_fd = socket(AF_INET, SOCK_STREAM, 0),
             []() { throw std::runtime_error("socket() error"); });

  // bind()
  {
    sockaddr_in server_addr{};
    inet_pton(AF_INET, ip, &server_addr.sin_addr.s_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    tcp::check(bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)),
               []() { throw std::runtime_error("bind() error"); });
  }

  // listen
  tcp::check(listen(server_fd, N_LISTEN),
             []() { throw std::runtime_error("listen() error"); });

  // set nonblock
  tcp::check(set_socket_nonblock(server_fd),
             []() { throw std::runtime_error("set_socket_nonblock() error"); });

  // init epoll
  tcp::check(_epoll_fd = epoll_create(MAX_EPOLL_FD),
             []() { throw std::runtime_error("epoll_create() error"); });

  // add server fd
  tcp::check(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, server_fd, EPOLLIN),
             []() { throw std::runtime_error("epoll_ctl(ADD, server_fd) error"); });

  // loop, can be dispatched to each threads
  auto events = new epoll_event[MAX_EVENTS];

  while (true) {
    int nfds;
    tcp::check(nfds = epoll_wait(_epoll_fd, events, MAX_EVENTS, 0),
               []() { throw std::runtime_error("epoll_wait() error"); });

    for (int i = 0; i < nfds; ++i) {
      if (events[i].data.fd == server_fd) { // new connection event
        // accept conection
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd;
        tcp::check(client_fd = accept(server_fd, (sockaddr *)&client_addr, &addr_len),
                   []() { throw std::runtime_error("accept() error"); });

        // init buffer
        _read_buffer_map[client_fd].clear();
        _write_buffer_map[client_fd].clear();

        // set nonblocking socket
        tcp::check(set_socket_nonblock(client_fd),
                   []() { throw std::runtime_error("set_socket_nonblock() failed"); });

        // add to epoll
        // LT mode, always listen to readable event, only listen to writable event
        // when message are not totally sended by one write() call.
        tcp::check(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, EPOLLIN),
                   []() { throw std::runtime_error("set_socket_nonblock() failed"); });

      } else { // read/write event
        int client_fd = events[i].data.fd;

        // handle writable event
        if (events[i].events & EPOLLOUT) {
          // TODO: handle writable event
        }

        // handle readable event
        if (events[i].events & EPOLLIN) {
          // TODO: handle readable event
        }
      }
    }
  }
}

void tcp::single_reactor_server::set_response(const char *resp) {
  // todo
}

int tcp::single_reactor_server::set_socket_nonblock(int fd) {
  int old_flag = fcntl(fd, F_GETFL, 0);
  int new_flag = old_flag | O_NONBLOCK;
  return fcntl(fd, F_SETFL, new_flag);
}

int tcp::single_reactor_server::epoll_ctl(int epfd, int op, int fd, uint32_t event) {
  epoll_event ev{};
  ev.data.fd = fd;
  ev.events = event; // LT
  return ::epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}
