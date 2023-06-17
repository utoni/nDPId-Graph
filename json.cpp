#include "json.hpp"

bool nDPIsrvd_json::parse() {
  int subevent_min = -1, subevent_max = -1;

  if (!pt.count("alias"))
    throw std::runtime_error("Missing required JSON key `alias'");
  alias = pt.get<std::string>("alias");

  if (!pt.count("source"))
    throw std::runtime_error("Missing required JSON key `source'");
  source = pt.get<std::string>("source");

  if (!pt.count("packet_id"))
    throw std::runtime_error("Missing required JSON key `packet_id'");
  packet_id = pt.get<unsigned long long int>("packet_id");

  thread_id = -1;
  if (pt.count("thread_id"))
    thread_id = pt.get<int>("thread_id");

  event = nDPIsrvd_json::NJ_INVALID;
  subevent_id = -1;
  if (pt.count("daemon_event_id")) {
    event = nDPIsrvd_json::NJ_DAEMON_EVENT;
    subevent_id = pt.get<int>("daemon_event_id");
    subevent_min = nDPIsrvd_json::subevent::NJ_DAEMON_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_DAEMON_COUNT;
  } else if (pt.count("error_event_id")) {
    event = nDPIsrvd_json::NJ_ERROR_EVENT;
    subevent_id = pt.get<int>("error_event_id");
    subevent_min = nDPIsrvd_json::subevent::NJ_ERROR_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_ERROR_COUNT;
  } else if (pt.count("flow_event_id")) {
    event = nDPIsrvd_json::NJ_FLOW_EVENT;
    subevent_id = pt.get<int>("flow_event_id");
    subevent_min = nDPIsrvd_json::subevent::NJ_FLOW_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_FLOW_COUNT;
  } else if (pt.count("packet_event_id")) {
    event = nDPIsrvd_json::NJ_PACKET_EVENT;
    subevent_id = pt.get<int>("packet_event_id");
    subevent_min = nDPIsrvd_json::subevent::NJ_PACKET_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_PACKET_COUNT;
  }
  if (event <= NJ_INVALID || event >= NJ_COUNT || subevent_id <= subevent_min ||
      subevent_id >= subevent_max)
    throw std::runtime_error("Unknown event received");

  return true;
}

void nDPIsrvd_stats::print(std::ostream &out) const {
  out << "Daemon: " << event_count[nDPIsrvd_json::NJ_DAEMON_EVENT]
      << ", Error: " << event_count[nDPIsrvd_json::NJ_ERROR_EVENT]
      << ", Flow: " << event_count[nDPIsrvd_json::NJ_FLOW_EVENT]
      << ", Packet: " << event_count[nDPIsrvd_json::NJ_PACKET_EVENT]
      << std::endl;
}

void nDPIsrvd_stats::update(const struct nDPIsrvd_json &nj) {
  int subevent_min, subevent_max;

  if (nj.event >= nDPIsrvd_json::NJ_INVALID &&
      nj.event < nDPIsrvd_json::NJ_COUNT)
    event_count[nj.event]++;
  else
    throw std::runtime_error("Could not increment event count");

  switch (nj.event) {
  case nDPIsrvd_json::NJ_INVALID:
    return;
  case nDPIsrvd_json::NJ_DAEMON_EVENT:
    subevent_min = nDPIsrvd_json::subevent::NJ_DAEMON_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_DAEMON_COUNT - 1;
    break;
  case nDPIsrvd_json::NJ_ERROR_EVENT:
    subevent_min = nDPIsrvd_json::subevent::NJ_ERROR_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_ERROR_COUNT - 1;
    break;
  case nDPIsrvd_json::NJ_FLOW_EVENT:
    subevent_min = nDPIsrvd_json::subevent::NJ_FLOW_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_FLOW_COUNT - 1;
    break;
  case nDPIsrvd_json::NJ_PACKET_EVENT:
    subevent_min = nDPIsrvd_json::subevent::NJ_PACKET_INVALID;
    subevent_max = nDPIsrvd_json::subevent::NJ_PACKET_COUNT - 1;
    break;
  default:
    throw std::runtime_error("Unknown event id: " + std::to_string(nj.event));
  }

  if (nj.subevent_id < subevent_min || nj.subevent_id > subevent_max)
    throw std::runtime_error("Unknown sub-event id: " +
                             std::to_string(nj.subevent_id));

  switch (nj.event) {
  case nDPIsrvd_json::NJ_INVALID:
    return;
  case nDPIsrvd_json::NJ_DAEMON_EVENT:
    daemon_subevent_count[nj.subevent_id]++;
    break;
  case nDPIsrvd_json::NJ_ERROR_EVENT:
    error_subevent_count[nj.subevent_id]++;
    break;
  case nDPIsrvd_json::NJ_FLOW_EVENT:
    flow_subevent_count[nj.subevent_id]++;
    break;
  case nDPIsrvd_json::NJ_PACKET_EVENT:
    packet_subevent_count[nj.subevent_id]++;
    break;
  }
}

void nDPIsrvd_stats::operator+=(const nDPIsrvd_stats &other) {
  for (size_t i = 0; i < nDPIsrvd_json::NJ_COUNT; ++i)
    event_count[i] += other.event_count[i];
  for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_DAEMON_COUNT; ++i)
    daemon_subevent_count[i] += other.daemon_subevent_count[i];
  for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_ERROR_COUNT; ++i)
    error_subevent_count[i] += other.error_subevent_count[i];
  for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_FLOW_COUNT; ++i)
    flow_subevent_count[i] += other.flow_subevent_count[i];
  for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_PACKET_COUNT; ++i)
    packet_subevent_count[i] += other.packet_subevent_count[i];
}
