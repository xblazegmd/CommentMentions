// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <core/utils.hpp>
#include <core/comments/comments.hpp>
#include <core/history/history.hpp>
#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/ui/Notification.hpp>
#include <algorithm>
#include <memory>

using namespace geode::prelude;

static std::shared_ptr<comments::CommentListener> g_commentListener;

static void onMention(std::string user, std::string msg, std::string msgID) {
	auto his = history::loadHistory();
	if (his.isErr()) {
		log::error("Could not load mention history: {}", his.unwrapErr());
		Notification::create(
			"Could not load mention history",
			NotificationIcon::Error,
			2
		)->show();
	}
	auto hist = his.unwrap();
	if (std::ranges::contains(hist.history, msgID)) return;

	auto res = history::updateHistory({ msgID });
	if (res.isErr()) {
		log::error("Could not save mention to history: {}", his.unwrapErr());
		Notification::create(
			"Could not save mention to history",
			NotificationIcon::Error,
			2
		)->show();
	}

	log::info("Mention from @{}, '{}'", user, msg);
	CMUtils::notify(
		user + " mentioned you",
		msg
	);
}

void startListener(int levelID) {
	g_commentListener = std::make_shared<comments::CommentListener>(levelID, onMention);
	g_commentListener->start();
}

$execute {
	// Is the mention history initialized?
	auto res = history::loadHistory();
	if (res.isErr() && res.unwrapErr() == "Unable to open file: No such file or directory") {
		log::info("Initializing mention history");
		auto writeRes = history::writeHistory({});
		if (writeRes.isErr()) {
			log::error("Could not initialize mention history: {}", writeRes.unwrapErr());
			Notification::create(
				"Could not save mention to history",
				NotificationIcon::Error,
				2
			)->show();
			return;
		}
		log::info("Initialized mention history");
	}
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
		// CMUtils::notify(
		// 	"Notification",
		// 	"This is a test notification"
		// );
	}
};