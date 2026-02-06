#pragma once

#include "events/Event.h"

class PushSchnozEvent : public Event {
public:
  PushSchnozEvent();

  static const char* GetStaticEventType();
  const char* GetEventType() const override;
};
