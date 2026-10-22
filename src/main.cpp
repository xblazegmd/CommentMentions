
#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>

#include <MentionManager.hpp>
#include <utils.hpp>

#include <arc/prelude.hpp>
#include <string>

using namespace geode::prelude;

$on_game(Loaded) {
    if (!Mod::get()->setSavedValue("random-ahh-popup", true)) {
        FLAlertLayer::create(
            "User Mentions",
            "Thank you for using <cy>User Mentions!</c>. I recommend you take a look at the mod's settings <cj>/Xblaze</c>",
            "OK"
        )->show();
    }

    async::spawn([] -> arc::Future<> {
        // Internet check
        bool connected;
        if (Mod::get()->getSettingValue<bool>("internal-internet-check")) {
            auto connectedOpt = co_await async::waitForMainThread([] {
                return GameToolbox::doWeHaveInternet();
            });
            connected = connectedOpt.has_value() && *connectedOpt;
        } else {
            connected = co_await doWeHaveInternetCM();
        }

        if (!connected) {
            log::error("No internet connection!");

            co_await pauseUntilWeHaveInternet(); // Wait until we have internet

            // Continue like usual
            log::error("Back online ;)");
        }

        auto mentionManager = MentionManager::get();

        // Load levels and group chats
        co_await mentionManager->getLevels();
        co_await arc::sleep(asp::Duration::fromSecs(1));
        co_await mentionManager->getGroupChats();

        // Start looking for mentions
        mentionManager->start();
    });
}
