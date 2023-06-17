#include "client.hpp"

#define BOOST_JSON_STACK_BUFFER_SIZE 65535

#include <boost/asio/buffer.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/chrono.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

using std::placeholders::_1;
using std::placeholders::_2;

nDPIsrvd_client::nDPIsrvd_client(boost::asio::io_context &io_context)
    : m_recvq(), m_socket(io_context), m_deadline_timer(io_context),
      m_heartbeat_timer(io_context) {}

void nDPIsrvd_client::start(tcp::resolver::results_type endpoints) {
  m_endpoints = endpoints;
  start_connect(m_endpoints.begin());
  m_deadline_timer.async_wait(
      std::bind(&nDPIsrvd_client::check_deadline, this));
}

void nDPIsrvd_client::stop() {
  boost::system::error_code ignored_error;
  m_socket.close(ignored_error);
  m_deadline_timer.cancel();
  m_heartbeat_timer.cancel();
  m_stopped = true;
}

void nDPIsrvd_client::start_connect(
    tcp::resolver::results_type::iterator endpoint_iter) {
  if (endpoint_iter != m_endpoints.end()) {
    std::cerr << "Trying " << endpoint_iter->endpoint() << "..." << std::endl;
    m_deadline_timer.expires_after(std::chrono::seconds(60));
    m_socket.async_connect(
        endpoint_iter->endpoint(),
        std::bind(&nDPIsrvd_client::handle_connect, this, _1, endpoint_iter));
  } else {
    stop();
  }
}

void nDPIsrvd_client::handle_connect(
    const boost::system::error_code &error,
    tcp::resolver::results_type::iterator endpoint_iter) {
  if (m_stopped)
    return;

  if (!m_socket.is_open()) {
    std::cerr << "Connect timed out" << std::endl;
    start_connect(++endpoint_iter);
  } else if (error) {
    std::cerr << "Connect error: " << error.message() << std::endl;
    m_socket.close();
    start_connect(++endpoint_iter);
  } else {
    std::cerr << "Connected to " << endpoint_iter->endpoint() << std::endl;
    start_read();
  }
}

void nDPIsrvd_client::start_read() {
  m_deadline_timer.expires_after(std::chrono::seconds(30));
  boost::asio::async_read_until(
      m_socket,
      boost::asio::dynamic_buffer(m_input_buffer, BOOST_JSON_STACK_BUFFER_SIZE),
      '\n', std::bind(&nDPIsrvd_client::handle_read, this, _1, _2));
}

void nDPIsrvd_client::handle_read(const boost::system::error_code &error,
                                  std::size_t n) {
  if (m_stopped)
    return;

  if (!error) {
    std::string line(m_input_buffer.substr(0, n - 1));
    m_input_buffer.erase(0, n);
    if (!line.empty()) {
      if (!handle_protocol(line))
        std::cerr << "Protocol error for received line: " << line << std::endl;
    } else {
      std::cerr << "Empty line received" << std::endl;
    }

    start_read();
  } else {
    std::cout << "Error on receive: " << error.message() << std::endl;

    stop();
  }
}

void nDPIsrvd_client::check_deadline() {
  if (m_stopped)
    return;

  if (m_deadline_timer.expiry() <= steady_timer::clock_type::now()) {
    std::cerr << "No data received until deadline" << std::endl;
    m_deadline_timer.expires_after(std::chrono::seconds(30));
  }

  m_deadline_timer.async_wait(
      std::bind(&nDPIsrvd_client::check_deadline, this));
}

bool nDPIsrvd_client::handle_protocol(const std::string &line) {
  int json_length;
  std::size_t json_index;
  const auto line_length = line.length();

  if (line_length < 3 /* e.g. "0{}" */)
    return false;

  try {
    json_length = std::stoi(line, &json_index);
  } catch (const std::exception &e) {
    std::cerr << "Invalid PDU length: " << e.what() << std::endl;
    return false;
  }

  if (line[json_index] != '{' || line[line_length - 1] != '}') {
    std::cerr << "Invalid JSON string" << std::endl;
    return false;
  }

  m_recvq.push(&line[json_index]);

  return true;
}
