#ifndef CLIENT_H
#define CLIENT_H 1

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <string>

#include "json.hpp"

using boost::asio::steady_timer;
using boost::asio::ip::tcp;

class nDPIsrvd_client {
public:
  nDPIsrvd_client(boost::asio::io_context &io_context);
  void start(tcp::resolver::results_type endpoints);
  void stop();
  nDPIsrvd_client_queue &get_queue() { return m_recvq; }
  bool stopped() { return m_stopped; }

private:
  void start_connect(tcp::resolver::results_type::iterator endpoint_iter);
  void handle_connect(const boost::system::error_code &error,
                      tcp::resolver::results_type::iterator endpoint_iter);
  void start_read();
  void handle_read(const boost::system::error_code &error, std::size_t n);
  void check_deadline();
  bool handle_protocol(const std::string &line);

private:
  nDPIsrvd_client_queue m_recvq;
  std::atomic<bool> m_stopped = false;
  tcp::resolver::results_type m_endpoints;
  tcp::socket m_socket;
  std::string m_input_buffer;
  steady_timer m_deadline_timer;
  steady_timer m_heartbeat_timer;
};

#endif
