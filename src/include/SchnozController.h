#pragma once

#include "events/EventHandler.h"
#include <GameObject.h>
#include <events/PushSchnozEvent.h>
#include <glm/fwd.hpp>

class SchnozController : public GameObject {
public:
  SchnozController();
  virtual ~SchnozController();

  void Awake();
private:
  void PushSchnoz();

  EventHandler<PushSchnozEvent> onPushSchnoz;
};
