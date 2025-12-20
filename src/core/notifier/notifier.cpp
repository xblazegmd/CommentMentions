#include "notifier.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding_arm/AchievementNotifier.hpp>
#include <Geode/loader/Event.hpp>
#include <functional>

using namespace geode::prelude;

namespace notifier {
    Notification NotificationEvent::getNotification() const {
        return m_notification;
    }

    bool NotificationEvent::queue() const {
        return m_queue;
    }

    bool NotificationEvent::popQueue() const {
        return m_popQueue;
    }

    ListenerResult NotificationEventFilter::handle(std::function<Callback> callback, NotificationEvent* ev) {
        return callback(ev);
    }

    ListenerResult Notifier::onEvent(NotificationEvent* ev) {
        if (ev->popQueue()) {
            return onPopQueueEvent();
        } else if (ev->queue()) {
            return onQueueEvent(ev->getNotification());
        } else {
            return onNotificationEvent(ev->getNotification());
        }
    }

    ListenerResult Notifier::onNotificationEvent(Notification notification) {
        AchievementNotifier::sharedState()->notifyAchievement(
            notification.title.c_str(),
            notification.message.c_str(),
            "accountBtn_messages_001.png",
            true
        );
        return ListenerResult::Stop;
    }

    ListenerResult Notifier::onQueueEvent(Notification notification) {
        m_queue.push_back(notification);
        return ListenerResult::Stop;
    }

    ListenerResult Notifier::onPopQueueEvent() {
        for (const auto& notification : m_queue) {
            AchievementNotifier::sharedState()->notifyAchievement(
                notification.title.c_str(),
                notification.message.c_str(),
                "accountBtn_messages_001.png",
                true
            );
        }

        m_queue.clear();
        return ListenerResult::Stop;
    }

    void Notifier::notify(std::string title, std::string message) {
        Notification notification{ title, message };

        if (PlayLayer::get()) {
            NotificationEvent(notification, true).post();
        } else {
            NotificationEvent(notification, false).post();
        }
    }
}