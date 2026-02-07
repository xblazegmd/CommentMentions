#include "notifier.hpp"

#include <Geode/binding_arm/PlayLayer.hpp>
#include <arc/future/Future.hpp>
#include <arc/sync/Notify.hpp>
#include <arc/time/Sleep.hpp>
#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/utils/coro.hpp>
#include <asp/time/Duration.hpp>
#include <functional>

using namespace geode::prelude;

namespace notifier {
    arc::Future<> Notifier::checkIfCanNotify() {
        while (true) {
            if (!PlayLayer::get()) m_notifySendAll.notifyOne();
            co_await arc::sleep(asp::Duration::fromMillis(200));
        }
    }

    arc::Future<> Notifier::sendAllCheckerIdk() {
        while (true) {
            co_await m_notifySendAll.notified();
            if (!m_notifications.empty()) {
                for (const auto& notification : m_notifications) {
                    sendNotification(notification.title, notification.message);
                }
            }
        }
    }
    // Notification NotificationEvent::getNotification() const {
    //     return m_notification;
    // }

    // bool NotificationEvent::queue() const {
    //     return m_queue;
    // }

    // bool NotificationEvent::popQueue() const {
    //     return m_popQueue;
    // }

    // ListenerResult NotificationEventFilter::handle(std::function<Callback> callback, NotificationEvent* ev) {
    //     return callback(ev);
    // }

    // ListenerResult Notifier::onEvent(NotificationEvent* ev) {
    //     if (ev->popQueue()) {
    //         return onPopQueueEvent();
    //     } else if (ev->queue()) {
    //         return onQueueEvent(ev->getNotification());
    //     } else {
    //         return onNotificationEvent(ev->getNotification());
    //     }
    // }

    // ListenerResult Notifier::onNotificationEvent(Notification notification) {
    //     showNotification(notification);
    //     return ListenerResult::Stop;
    // }

    // ListenerResult Notifier::onQueueEvent(Notification notification) {
    //     m_queue.push_back(notification);
    //     return ListenerResult::Stop;
    // }

    // ListenerResult Notifier::onPopQueueEvent() {
    //     for (const auto& notification : m_queue) {
    //         showNotification(notification);
    //     }

    //     m_queue.clear();
    //     return ListenerResult::Stop;
    // }

    // void Notifier::notify(std::string title, std::string message) {
    //     Notification notification{ title, message };

    //     if (PlayLayer::get()) {
    //         NotificationEvent(notification, true).post();
    //         m_canPopQueue = std::get<0>(coro::spawn << canPopQueue());
    //     } else {
    //         NotificationEvent(notification, false).post();
    //     }
    // }

    // void Notifier::showNotification(Notification const& notification) {
    //     AchievementNotifier::sharedState()->notifyAchievement(
    //         notification.title.c_str(),
    //         notification.message.c_str(),
    //         "accountBtn_pendingRequest_001.png",
    //         true
    //     );
    // }

    // Task<void> Notifier::canPopQueue() {
    //     while (true) {
    //         if (!PlayLayer::get()) {
    //             NotificationEvent(true).post();
    //             co_return;
    //         };
    //         co_await coro::sleep(0.5f);
    //     }
    // }
}