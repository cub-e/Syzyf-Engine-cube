#include "events/PushSchnozEvent.h"

PushSchnozEvent::PushSchnozEvent() {}
const char* PushSchnozEvent::GetStaticEventType() {
// In the post it is suggested to use a hash here
//  https://denyskryvytskyi.github.io/event-system
  return "POOGA";
}

const char* PushSchnozEvent::GetEventType() const {
  return GetStaticEventType();
}

