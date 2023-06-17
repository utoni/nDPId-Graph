#ifndef CONTROLLER_H
#define CONTROLLER_H 1

#include <boost/asio/io_context.hpp>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>

#include "client.hpp"
#include "graph.hpp"
#include "json.hpp"

class Controller {
public:
  Controller();
  ~Controller();
  bool start(const std::string host, const std::string port);
  void loop();

  boost::asio::io_context &get_io_context() { return m_io_context; }
  std::shared_ptr<nDPIsrvd_client> &get_client() { return m_client; }
  std::shared_ptr<nDPIsrvd_graph> &get_graphics() { return m_graphics; }

  friend void client_thread(Controller *_this, const std::string host,
                            const std::string port);
  friend void graph_thread(Controller *_this);

private:
  static void client_thread(Controller *_this, const std::string host,
                            const std::string port);
  static void graph_thread(Controller *_this);

  std::binary_semaphore m_client_sem, m_graph_sem;

private:
  std::thread m_client_thread, m_graph_thread;
  boost::asio::io_context m_io_context;
  std::shared_ptr<nDPIsrvd_client> m_client;
  std::shared_ptr<nDPIsrvd_graph> m_graphics;
};

#endif
