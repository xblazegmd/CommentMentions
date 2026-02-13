#pragma once

#include <arc/future/Future.hpp>
#include <arc/sync/Mutex.hpp>
#include <arc/sync/Notify.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/Task.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace CommentMentions {
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
}