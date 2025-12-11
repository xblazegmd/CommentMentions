#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/ui/Notification.hpp>
#include <core/utils.hpp>
#include <core/comments/comments.hpp>
#include <memory>

using namespace geode::prelude;

static std::shared_ptr<comments::CommentListener> g_commentListener;

$on_mod(Loaded) {
	log::info("Successfully loaded CommentMentions");
}

$on_game(Loaded) {
	geode::createQuickPopup(
		"Start listener",
		"Do you want to start the mention listener?",
		"No", "Yes",
		[](auto, bool btn2) {
			if (btn2) {
				auto lvlFetch = std::make_shared<CMutils::LevelFetch>(CMutils::LevelFetchTarget::Daily);
				lvlFetch->fetchID([lvlFetch](Result<int> dailyID) {
					if (dailyID.isErr()) {
						Notification::create(
							"Could not fetch daily level ID",
							NotificationIcon::Error,
							2
						)->show();
						return;
					}


					g_commentListener = std::make_shared<comments::CommentListener>(dailyID.unwrap(), [](auto user, auto msg) {
						log::info("Mention from @{}, '{}'", user, msg);
						CMutils::notify(
							user + " mentioned you",
							msg
						);
					});
					g_commentListener->start();
				});
			}
		}
	);
}

#include <Geode/modify/MenuLayer.hpp>
class $modify(CMMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		auto testBt = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
			this,
			menu_selector(CMMenuLayer::onTestBt)
		);
		testBt->setID("notify_menu"_spr);

		auto btmMenu = this->getChildByID("bottom-menu");
		btmMenu->addChild(testBt);
		btmMenu->updateLayout();

		return true;
	}

	void onTestBt(CCObject* sender) {
		CMutils::notify(
			"Notification",
			"This is a test notification"
		);
	}
};