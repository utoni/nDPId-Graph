#ifndef JSON_H
#define JSON_H 1

#include "tsqueue.hpp"

#include <atomic>
#include <boost/property_tree/ptree.hpp>
#include <ostream>

using boost::property_tree::ptree;

struct nDPIsrvd_json {
  bool parse();

  std::string alias;
  std::string source;
  unsigned long long int packet_id;
  int thread_id;
  enum nj_event {
    NJ_INVALID = 0,
    NJ_DAEMON_EVENT,
    NJ_ERROR_EVENT,
    NJ_FLOW_EVENT,
    NJ_PACKET_EVENT,
    NJ_COUNT
  } event;
  union subevent {
    enum daemon {
      NJ_DAEMON_INVALID = 0,
      NJ_DAEMON_INIT,
      NJ_DAEMON_RECONNECT,
      NJ_DAEMON_SHUTDOWN,
      NJ_DAEMON_STATUS,
      NJ_DAEMON_COUNT
    } daemon;
    enum error { NJ_ERROR_INVALID = 0, NJ_ERROR_COUNT = 9 } error;
    enum flow {
      NJ_FLOW_INVALID = 0,
      NJ_FLOW_NEW,
      NJ_FLOW_END,
      NJ_FLOW_IDLE,
      NJ_FLOW_UPDATE,
      NJ_FLOW_ANALYSE,
      NJ_FLOW_GUESSED,
      NJ_FLOW_DETECTED,
      NJ_FLOW_DETECTION_UPDATE,
      NJ_FLOW_NOT_DETECTED,
      NJ_FLOW_COUNT
    } flow;
    enum packet {
      NJ_PACKET_INVALID = 0,
      NJ_PACKET_PAYLOAD,
      NJ_PACKET_PAYLOAD_FLOW,
      NJ_PACKET_COUNT
    } packet;
  } subevent;
  int subevent_id;

  ptree pt;
};

struct nDPIsrvd_stats {
  void print(std::ostream &out) const;
  void update(const struct nDPIsrvd_json &nj);
  void operator+=(const nDPIsrvd_stats &other);

  std::atomic<size_t> event_count[nDPIsrvd_json::NJ_COUNT];
  std::atomic<size_t>
      daemon_subevent_count[nDPIsrvd_json::subevent::NJ_DAEMON_COUNT];
  std::atomic<size_t>
      error_subevent_count[nDPIsrvd_json::subevent::NJ_ERROR_COUNT];
  std::atomic<size_t>
      flow_subevent_count[nDPIsrvd_json::subevent::NJ_FLOW_COUNT];
  std::atomic<size_t>
      packet_subevent_count[nDPIsrvd_json::subevent::NJ_PACKET_COUNT];
};

using nDPIsrvd_client_queue = ThreadSafeQueue<std::string>;
using nDPIsrvd_graph_queue = ThreadSafeQueue<struct nDPIsrvd_json>;

#endif
