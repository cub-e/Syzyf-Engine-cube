#include "events/EventManager.h"
#include <spdlog/spdlog.h>
#include <cstring>

void EventManager::Shutdown() {
   m_subscribers.clear();
}

void EventManager::Subscribe(const std::string& eventId, std::unique_ptr<EventHandlerWrapperInterface>&& handler) {
  auto subscribers = m_subscribers.find(eventId);
  if (subscribers != m_subscribers.end()) {
    auto& handlers = subscribers->second;
    for (auto& it : handlers) {
      if (std::strcmp(it->GetType(), handler->GetType()) == 0) {
        spdlog::warn("Attempting to double-register callback");
        return;
      }
    }
    handlers.emplace_back(std::move(handler));
  } else {
    m_subscribers[eventId].emplace_back(std::move(handler));
  }
}

void EventManager::Unsubscribe(const std::string& eventId, const std::string& handlerName) {
  auto it = m_subscribers.find(eventId);
  if (it == m_subscribers.end()) return;

  auto& handlers = it->second;
  for (auto hit = handlers.begin(); hit != handlers.end(); ++hit) {
    if (std::strcmp((*hit)->GetType(), handlerName.c_str()) == 0) {
      handlers.erase(hit);
      return;
    }
  }
}

void EventManager::TriggerEvent(const Event& event_) {
  auto it = m_subscribers.find(event_.GetEventType());
  
  if (it != m_subscribers.end()) {
    for (auto& handler : it->second) {
      handler->Exec(event_);
    }
  }
}

void EventManager::QueueEvent(std::unique_ptr<Event>&& event) {
  m_eventsQueue.emplace_back(std::move(event));
}

void EventManager::DispatchEvents() {
  for (const auto& eventPtr : m_eventsQueue) {
    if (!eventPtr->isHandled) {
      TriggerEvent(*eventPtr);
    }
  }

  m_eventsQueue.clear();
}
