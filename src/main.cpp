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
		[](auto, bool btn2) {
			if (btn2) {
				auto* lvlFetch = new CMutils::LevelFetch(CMutils::LevelFetchTarget::Daily);
				lvlFetch->fetchID();
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