#include <Geode/Geode.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <core/utils.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
	log::info("Successfully loaded CommentMentions");
}

$on_game(Loaded) {
	geode::createQuickPopup(
		"Daily level fetch",
		"Do you want to fetch the daily lvl id?",
		"No", "Yes",
		[](auto, bool bt2) {
			if (bt2) {
				CMentions::utils::LevelFetch lvlFetch(CMentions::utils::LevelFetchTarget::Daily);
				lvlFetch.fetchID();
			}
		}
	);
}

#include <Geode/modify/MenuLayer.hpp>
class $modify(CMentionsMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		auto testBt = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
			this,
			menu_selector(CMentionsMenuLayer::onTestBt)
		);
		testBt->setID("notify_menu"_spr);

		auto btmMenu = this->getChildByID("bottom-menu");
		btmMenu->addChild(testBt);
		btmMenu->updateLayout();

		return true;
	}

	void onTestBt(CCObject* sender) {
		CMentions::utils::notify(
			"Notification",
			"This is a test notification"
		);
	}
};