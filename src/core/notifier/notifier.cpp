#include "notifier.hpp"

#include <arc/prelude.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <asp/time/Duration.hpp>

using namespace geode::prelude;

namespace notifier {
    Notifier::Notifier() {
        auto handle = async::spawn([this]() -> arc::Future<> {
            auto lock = co_await m_notifications.lock();
            lock->reserve(500);
        });
        handle.blockOn();

        async::spawn(checkIfCanNotify());
        async::spawn(trySendAll());
    }

    void Notifier::notify(const std::string& title, const std::string& message) {
        async::spawn([this, title, message]() -> arc::Future<> {
            auto lock = co_await m_notifications.lock();
            lock->emplace_back(Notification{ title, message });
        });
    }

    arc::Future<> Notifier::checkIfCanNotify() {
        while (true) {
            co_await async::waitForMainThread<void>([this]() {
                if (!PlayLayer::get()) m_notifySendAll.notifyOne();
            });
            co_await arc::sleep(asp::Duration::fromMillis(300));
        }
    }

    arc::Future<> Notifier::trySendAll() {
        while (true) {
            co_await  m_notifySendAll.notified();
            auto lock = co_await m_notifications.lock();
            if (!lock->empty()) {
                for (const auto& notification : *lock) {
                    sendNotification(notification.title, notification.message);
                }
                lock->clear();
            }
        }
    }

    void Notifier::sendNotification(const std::string& title, const std::string& msg) {
        AchievementNotifier::sharedState()->notifyAchievement(
            title.c_str(),
            msg.c_str(),
            "accountBtn_pendingRequest_001.png",
            true
        );
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