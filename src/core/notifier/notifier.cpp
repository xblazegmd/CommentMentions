#include "notifier.hpp"

#include <arc/prelude.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <asp/time/Duration.hpp>

using namespace geode::prelude;

namespace CommentMentions {
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
                    co_await async::waitForMainThread<void>([this, notification]() {
                        sendNotification(notification.title, notification.message);
                    });
                }
                lock->clear();
            }
        }
    }

    void Notifier::sendNotification(const std::string& title, const std::string& msg) {
        AchievementNotifier::sharedState()->notifyAchievement(
            title.c_str(),
            msg.c_str(),
            "currencyOrbIcon_001.png",
            true
        );
    }
}