#include <events/PushSchnozEvent.h>

PushSchnozEvent::PushSchnozEvent() {}
const std::string PushSchnozEvent::GetStaticEventType() {
  return "POOGA";
}

const std::string PushSchnozEvent::GetEventType() const {
  return GetStaticEventType();
}

