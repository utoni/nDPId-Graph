#include "graph.hpp"

#include "widgets.hpp"

#include <exception>
#include <memory>

nDPIsrvd_graph::nDPIsrvd_graph() : m_recvq(), m_imweb() {}

void nDPIsrvd_graph::init(int width, int height) {
  m_imweb.init(width, height);
  m_imweb.addDrawable(std::make_shared<EventPieChart>());
  m_imweb.addDrawable(std::make_shared<FlowEventPlotter>());
}

void nDPIsrvd_graph::loop() {
  m_imweb.loop([&]() {
    bool retval = true;

    if (!m_recvq.empty()) {
      auto nj = m_recvq.pop();
#if 0
      printf("Thread: %d - Event: %d - Subevent: %d - Alias: %s - Source: %s - "
             "Packet %llu\n",
             nj.thread_id, nj.event, nj.subevent_id, nj.alias.c_str(),
             nj.source.c_str(), nj.packet_id);
#endif

      auto drawablesBegin = m_imweb.drawablesBegin();
      auto drawablesEnd = m_imweb.drawablesEnd();
      for (auto &drawablePtr = drawablesBegin; drawablePtr != drawablesEnd;
           ++drawablePtr) {
        auto drawable = *drawablePtr;
        auto widget = reinterpret_pointer_cast<Widget>(drawable);
        if (widget)
          widget->update(m_stats);
        else
          throw std::runtime_error("Invalid cast to class Widget!");
      }
    }

    return retval;
  });
}
