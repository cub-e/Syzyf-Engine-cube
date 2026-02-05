#include <events/EventManager.h>

#include <spdlog/spdlog.h>

void EventManager::Shutdown()
{
   m_subscribers.clear();
}

void EventManager::Subscribe(const std::string& eventId, std::unique_ptr<EventHandlerWrapperInterface>&& handler)
{
    auto subscribers = m_subscribers.find(eventId);
    if (subscribers != m_subscribers.end()) {
        auto& handlers = subscribers->second;
        for (auto& it : handlers) {
            if (it->GetType() == handler->GetType()) {
              spdlog::warn("Attempting to double-register callback");
              return;
            }
        }
        handlers.emplace_back(std::move(handler));
    } else {
        m_subscribers[eventId].emplace_back(std::move(handler));
    }
}

void EventManager::Unsubscribe(const std::string& eventId, const std::string& handlerName)
{
    auto& handlers = m_subscribers[eventId];
    for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        if (it->get()->GetType() == handlerName) {
            handlers.erase(it);
            return;
        }
    }
}

void EventManager::TriggerEvent(const Event& event_)
{
    for (auto& handler : m_subscribers[event_.GetEventType()]) {
        handler->Exec(event_);
    }
}

void EventManager::QueueEvent(std::unique_ptr<Event>&& event)
{
    m_eventsQueue.emplace_back(std::move(event));
}

void EventManager::DispatchEvents()
{
    for (auto eventIt = m_eventsQueue.begin(); eventIt != m_eventsQueue.end();) {
        if (!eventIt->get()->isHandled) {
            TriggerEvent(*eventIt->get());
            eventIt = m_eventsQueue.erase(eventIt);
        } else {
            ++eventIt;
        }
    }
}
