#pragma once

#include "events/Event.h"
#include <string>

class PushSchnozEvent : public Event {
public:
  PushSchnozEvent();

  static const std::string GetStaticEventType();
  const std::string GetEventType() const override;
};
