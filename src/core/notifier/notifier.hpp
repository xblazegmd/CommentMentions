#pragma once

#include <Geode/loader/Event.hpp>
#include <string>

using namespace geode::prelude;

namespace notifier {
    struct Notification {
        std::string title;
        std::string message;
    };

    class NotificationEvent : public Event {
        protected:
            Notification m_notification;
            bool m_queue;
            bool m_popQueue;
        public:
            NotificationEvent(Notification notification, bool queue);
            NotificationEvent(bool popQueue);

            Notification getNotification() const;
            bool queue() const;
            bool popQueue() const;
    };

    class Notifier {
        protected:
            EventListener<EventFilter<NotificationEvent>> m_notification;
            std::vector<Notification> m_queue;

            ListenerResult onEvent(Notification notification);
            ListenerResult onNotificationEvent(Notification notification);
            ListenerResult onQueueEvent(Notification notification);
            ListenerResult onPopQueueEvent(Notification notification);
        public:
            static void notify(std::string title, std::string message);
    };
}