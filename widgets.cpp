#include "widgets.hpp"

#include "json.hpp"

#include <array>
#include <cmath>
#include <string>

template <typename T> inline T RandomRange(T min, T max) {
  T scale = rand() / (T)RAND_MAX;
  return min + scale * (max - min);
}

EventPieChart::EventPieChart() : Widget("EventPieChart"), m_events() {}

EventPieChart::~EventPieChart() {}

bool EventPieChart::draw() {
  static const char *const labels[nDPIsrvd_json::NJ_COUNT] = {"Daemon", "Error",
                                                              "Flow", "Packet"};
  float data[] = {static_cast<float>(m_events[nDPIsrvd_json::NJ_DAEMON_EVENT]),
                  static_cast<float>(m_events[nDPIsrvd_json::NJ_ERROR_EVENT]),
                  static_cast<float>(m_events[nDPIsrvd_json::NJ_FLOW_EVENT]),
                  static_cast<float>(m_events[nDPIsrvd_json::NJ_PACKET_EVENT])};

  ImGui::Begin(m_id.c_str());
  auto window_size = ImGui::GetWindowSize();
  window_size.y -= 45;
  window_size.x -= 15;
  if (ImPlot::BeginPlot("##EventPieChart", window_size)) {
    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations,
                      ImPlotAxisFlags_NoDecorations);
    ImPlot::SetupAxesLimits(0, 1, 0, 1);
    ImPlot::PlotPieChart(labels, data, 4, 0.5, 0.5, 0.4, "%.0f", 90, 0);
    ImPlot::EndPlot();
  }
  ImGui::End();

  return true;
}

void EventPieChart::update(const nDPIsrvd_stats &stats) {
  for (size_t i = 0; i < nDPIsrvd_json::NJ_COUNT; ++i)
    m_events[i] = stats.event_count[i];
}

FlowEventPlotter::FlowEventPlotter(size_t history_size)
    : Widget("FlowEventPlotter"), m_time_steps(history_size, 0.0f) {
  for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_FLOW_COUNT; ++i)
    m_events[i] = std::vector<float>(history_size, 0.0f);
}

FlowEventPlotter::~FlowEventPlotter() {}

bool FlowEventPlotter::draw() {
  ImGui::Begin(m_id.c_str());
  auto window_size = ImGui::GetWindowSize();
  window_size.y -= 45;
  window_size.x -= 15;
  ImPlot::PushStyleVar(ImPlotStyleVar_PlotMinSize, ImVec2(300, 225));
  ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(40, 20));
  if (ImPlot::BeginPlot("##FlowEventPlotter", window_size)) {
    ImPlot::SetupAxes("Time", "Events", ImPlotAxisFlags_AutoFit,
                      ImPlotAxisFlags_AutoFit);
    ImPlot::SetupAxesLimits(0, 100, 0, 500);

    ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
    struct {
      char const *const name;
    } mappings[nDPIsrvd_json::subevent::NJ_FLOW_COUNT] = {
        {"Invalid"},          {"New"},         {"End"},     {"Idle"},
        {"Update"},           {"Analyse"},     {"Guessed"}, {"Detected"},
        {"Detection-Update"}, {"Not-Detected"}};
    for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_FLOW_COUNT; ++i) {
      ImPlot::PlotShaded(mappings[i].name, m_time_steps.data(),
                         m_events[i].data(), m_time_steps.size(),
                         m_shade_mode == 0   ? -INFINITY
                         : m_shade_mode == 1 ? INFINITY
                                             : m_fill_ref,
                         m_implot_flags);
    }
    ImPlot::PopStyleVar();
    for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_FLOW_COUNT; ++i) {
      ImPlot::PlotLine(mappings[i].name, m_time_steps.data(),
                       m_events[i].data(), m_time_steps.size());
    }
    ImPlot::EndPlot();
  }
  ImGui::End();
  ImPlot::PopStyleVar(2);

  return true;
}

void FlowEventPlotter::update(const nDPIsrvd_stats &stats) {
  m_time_steps.erase(m_time_steps.begin());
  m_time_steps.push_back(m_event_count + 1);

  for (size_t i = 0; i < nDPIsrvd_json::subevent::NJ_FLOW_COUNT; ++i) {
    m_events[i].erase(m_events[i].begin());
    m_events[i].push_back(stats.flow_subevent_count[i]);
  }

  m_event_count++;
}
