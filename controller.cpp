#include "controller.hpp"

#include <boost/json.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <exception>
#include <iostream>

Controller::Controller() : m_io_context(), m_client_sem(0), m_graph_sem(0) {
  m_client = std::make_shared<nDPIsrvd_client>(m_io_context);
  m_graphics = std::make_shared<nDPIsrvd_graph>();

  if (!m_client || !m_graphics)
    throw std::runtime_error("Controller dynamic memory allocation failed!");
}

Controller::~Controller() {}

void Controller::client_thread(Controller *_this, std::string host,
                               const std::string port) {
  try {
    auto &io_context = _this->get_io_context();
    auto &client = _this->get_client();
    tcp::resolver r(io_context);
    client->start(r.resolve(host, port));

    _this->m_graph_sem.release(2);
    _this->m_client_sem.acquire();

    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Client Exception: " << e.what() << std::endl;
  }

  _this->m_graph_sem.release(2);
}

void Controller::graph_thread(Controller *_this) {
  try {
    auto &graphics = _this->get_graphics();
    graphics->init();

    _this->m_client_sem.release(2);
    _this->m_graph_sem.acquire();

    graphics->loop();
    graphics.reset();
  } catch (std::exception &e) {
    std::cerr << "Graph Exception: " << e.what() << std::endl;
  }

  _this->m_client_sem.release(2);
}

bool Controller::start(const std::string host, const std::string port) {
  m_client_thread = std::thread(client_thread, this, host, port);
  m_graph_thread = std::thread(graph_thread, this);

  return m_client_thread.joinable() && m_graph_thread.joinable();
}

void Controller::loop() {
  bool run = true;
  auto &clientq = m_client->get_queue();
  auto &graphq = m_graphics->get_queue();

  m_client_sem.acquire();
  m_graph_sem.acquire();

  while (run) {
    if (!m_client_thread.joinable() || !m_graph_thread.joinable())
      run = false;

    if (m_client->stopped() || !m_graphics->is_running())
      run = false;

    if (!clientq.empty()) {
      auto json_str = clientq.pop();
      try {
        std::istringstream is(json_str);
        nDPIsrvd_json nj;
        read_json(is, nj.pt);
        if (!nj.parse())
          std::cerr << "Parsing JSON nDPIsrvd failed" << std::endl;
        else {
          nDPIsrvd_stats stats;
          stats.update(nj);
          m_graphics->update_stats(std::move(stats));
          graphq.push(std::move(nj));
        }
      } catch (const std::exception &e) {
        std::cerr << "Parsing JSON string failed: " << e.what() << std::endl;
      }
    }
  }

  std::cerr << "Stopping Controller main loop" << std::endl;
  m_graphics->stop();
  m_client->stop();

  m_client_thread.join();
  m_graph_thread.join();
}
