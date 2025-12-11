#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/ui/Notification.hpp>
#include <core/utils.hpp>
#include <core/comments/comments.hpp>
#include <memory>

using namespace geode::prelude;

static std::shared_ptr<comments::CommentListener> g_commentListener;

void startListener(int levelID) {
	g_commentListener = std::make_shared<comments::CommentListener>(levelID, [](auto user, auto msg) {
		log::info("Mention from @{}, '{}'", user, msg);
		CMUtils::notify(
			user + " mentioned you",
			msg
		);
	});
	g_commentListener->start();
}

$on_game(Loaded) {
	auto mod = Mod::get();
	auto useDailyLvl = mod->getSettingValue<bool>("use-daily-lvl");

	if (useDailyLvl) {
		auto lvlFetch = std::make_shared<CMUtils::LevelFetch>(CMUtils::LevelFetchTarget::Daily);
		lvlFetch->fetchID([lvlFetch](Result<int> dailyID) {
			if (dailyID.isErr()) {
				Notification::create(
					"Could not fetch daily level ID",
					NotificationIcon::Error,
					2
				)->show();
				return;
			}

			startListener(dailyID.unwrap());
		});
	} else {
		auto levelID = mod->getSettingValue<int64_t>("level-id");
		startListener(levelID);
	}
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
		CMUtils::notify(
			"Notification",
			"This is a test notification"
		);
	}
};