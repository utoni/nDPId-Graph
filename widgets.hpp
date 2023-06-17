#ifndef WIDGETS_H
#define WIDGETS_H 1

#include <implot.h>
#include <imweb.hpp>

#include <array>
#include <vector>

#include "json.hpp"

class Widget : public ImWebDrawable {
public:
  explicit Widget(std::string id) : ImWebDrawable(id) {}
  virtual bool draw() = 0;
  virtual void update(const nDPIsrvd_stats &) = 0;
};

class EventPieChart : public Widget {
public:
  EventPieChart();
  ~EventPieChart();
  bool draw() override;
  void update(const nDPIsrvd_stats &stats) override;

private:
  std::array<size_t, nDPIsrvd_json::NJ_COUNT> m_events;
};

class FlowEventPlotter : public Widget {
public:
  FlowEventPlotter(size_t history_size = 101);
  ~FlowEventPlotter();
  bool draw() override;
  void update(const nDPIsrvd_stats &stats) override;

private:
  ImPlotShadedFlags m_implot_flags = 0;
  bool m_shade_mode = false;
  float m_fill_ref = 0.0f;

private:
  size_t m_event_count = 0;
  std::vector<float> m_time_steps;
  std::array<std::vector<float>, nDPIsrvd_json::subevent::NJ_FLOW_COUNT>
      m_events;
};

#endif
