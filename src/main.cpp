// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
#include <core/comments/comments.hpp>
#include <core/levelFetch/levelFetch.hpp>
#include <core/utils.hpp>

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/ui/Notification.hpp>

#include <memory>

using namespace geode::prelude;

void startListener(int levelID) {
	comments::CommentListener::sharedState() = std::make_shared<comments::CommentListener>(levelID);
	comments::CommentListener::sharedState()->start();
}

$on_game(Loaded) {
	auto mod = Mod::get();
	// Is this the users' first time using the mod?
	if (!mod->setSavedValue("shown-first-time-msg", true)) {
		FLAlertLayer::create(
			"CommentMentions",
			"Thank you for using <co>CommentMentions!</c>. Make sure to change the <cj>names option</c> in the mod's settings",
			"OK"
		)->show();
	}

	auto useDailyLvl = mod->getSettingValue<bool>("use-daily-lvl");

	if (useDailyLvl) {
		auto lvlFetch = std::make_shared<levelFetch::LevelFetch>(levelFetch::LevelFetchTarget::Daily);
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