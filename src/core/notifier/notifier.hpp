#pragma once

#include <arc/future/Future.hpp>
#include <arc/sync/Mutex.hpp>
#include <arc/sync/Notify.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/Task.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace notifier {
    struct Notification {
        std::string title;
        std::string message;
    };

    class Notifier {
    public:
        Notifier();
        void notify(const std::string& title, const std::string& message);
    private:
        arc::Mutex<std::vector<Notification>> m_notifications;
        arc::Notify m_notifySendAll;

        arc::Future<> checkIfCanNotify();
        arc::Future<> trySendAll();
        void sendNotification(const std::string& title, const std::string& msg);
    };

    // class NotificationEvent : public Event {
    //     protected:
    //         Notification m_notification;
    //         bool m_queue;
    //         bool m_popQueue;
    //     public:
    //         NotificationEvent(Notification notification, bool queue) :
    //             m_notification(notification),
    //             m_queue(queue),
    //             m_popQueue(false)
    //         {};
    //         NotificationEvent(bool popQueue) :
    //             m_popQueue(popQueue)
    //         {};

    //         Notification getNotification() const;
    //         bool queue() const;
    //         bool popQueue() const;
    // };

    // // I think this is not needed at all but idk how else to define the notifier without this
    // class NotificationEventFilter : public EventFilter<NotificationEvent> {
    //     public:
    //         using Callback = ListenerResult(NotificationEvent*);

    //         ListenerResult handle(std::function<Callback> callback, NotificationEvent* ev);
    // };

    // class Notifier {
    //     protected:
    //         EventListener<NotificationEventFilter> m_notification = {
    //             this, &Notifier::onEvent, NotificationEventFilter()
    //         };
    //         std::vector<Notification> m_queue;
    //         Task<void> m_canPopQueue;

    //         ListenerResult onEvent(NotificationEvent* ev);
    //         ListenerResult onNotificationEvent(Notification notification);
    //         ListenerResult onQueueEvent(Notification notification);
    //         ListenerResult onPopQueueEvent();

    //         void showNotification(Notification const& notification);
    //         Task<void> canPopQueue();
    //     public:
    //         void notify(std::string title, std::string message);
    // };
}