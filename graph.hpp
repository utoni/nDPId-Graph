#ifndef GRAPH_H
#define GRAPH_H 1

#include <imweb.hpp>

#include "json.hpp"

class nDPIsrvd_graph {
public:
  explicit nDPIsrvd_graph();
  void init(int width = 1024, int height = 768);
  void loop();
  void stop() { m_imweb.stop(); }
  nDPIsrvd_graph_queue &get_queue() { return m_recvq; }
  void update_stats(const nDPIsrvd_stats &stats) { m_stats += stats; }
  bool is_initialized() { return m_imweb.isInitialized(); }
  bool is_running() { return m_imweb.isRunning(); }

private:
  nDPIsrvd_graph_queue m_recvq;
  nDPIsrvd_stats m_stats;
  ImWeb m_imweb;
};

#endif
