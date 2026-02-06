// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
#include <chrono>
#include <core/comments/comments.hpp>
#include <core/utils.hpp>

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/string.hpp>

#include <memory>

using namespace geode::prelude;

static std::shared_ptr<comments::CommentManager> g_commentListener;

void startListener(int levelID) {
	g_commentListener = std::make_shared<comments::CommentManager>(levelID);
	g_commentListener->start();
}

$on_game(Loaded) {
	auto mod = Mod::get();
	// Is this the users' first time using the mod?
	// if (!mod->setSavedValue("shown-first-time-msg", true)) {
	// 	FLAlertLayer::create(
	// 		"CommentMentions",
	// 		"Thank you for using <co>CommentMentions!</c>. Make sure to change the <cj>tags option</c> in the mod's settings",
	// 		"OK"
	// 	)->show();
	// }

	auto useDailyLvl = mod->getSettingValue<bool>("use-daily-lvl");

	if (useDailyLvl) {
		std::string ftype = "21"; // TODO: Weekly and event level support
		auto req = web::WebRequest()
			.userAgent("")
			.timeout(std::chrono::seconds(10))
			.bodyString("type=" + ftype + "&secret=" + CMUtils::SECRET);

		// THANK YOU FOR THE NEW ASYNC SYSTEM GEODE
		// Bro tasks were so annoying man, I love this change
		async::spawn(req.post(CMUtils::BOOMLINGS + "getGJLevels21.php"), [](web::WebResponse res) {
			if (res.ok() && CMUtils::stringIsOk(res.string())) {
        		auto levels = string::split(res.string().unwrap(), "#");
        		auto levelsSplit = string::split(levels[0], "|");
        		auto dailyLevel = string::split(levelsSplit[0], ":");

        		for (int i = 0; i < dailyLevel.size(); i += 2) {
        		    if (dailyLevel[i] == "1") {
        		        int dailyID = numFromString<int>(dailyLevel[i + 1]).unwrapOr(0); // TODO: Fallback to fixed ID
						startListener(dailyID);
        		    }
        		}
			} else {
				Notification::create(
					"Could not fetch daily level ID",
					NotificationIcon::Error,
					2
				)->show();
			}
		});
		// auto lvlFetch = std::make_shared<levelFetch::LevelFetch>(levelFetch::LevelFetchTarget::Daily);
		// lvlFetch->fetchID([lvlFetch](Result<int> dailyID) {
		// 	if (dailyID.isErr()) {
		// 		Notification::create(
		// 			"Could not fetch daily level ID",
		// 			NotificationIcon::Error,
		// 			2
		// 		)->show();
		// 		return;
		// 	}

		// 	startListener(dailyID.unwrap());
		// });
	} else {
		auto levelID = mod->getSettingValue<int64_t>("level-id");
		startListener(levelID);
	}
}