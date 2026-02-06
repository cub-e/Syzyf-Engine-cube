#pragma once

#include "GameObject.h"
#include "events/EventHandler.h"
#include "events/PushSchnozEvent.h"

class SchnozController : public GameObject {
public:
  SchnozController();
  virtual ~SchnozController();

  void Awake();
private:
  void PushSchnoz();

  EventHandler<PushSchnozEvent> onPushSchnoz;
};
