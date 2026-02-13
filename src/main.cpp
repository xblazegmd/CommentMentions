// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
#include <arc/future/Future.hpp>
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

static std::shared_ptr<CommentMentions::CommentManager> g_commentManager = nullptr;

$on_game(Loaded) {
	// Is this the users' first time using the mod?
	// if (!mod->setSavedValue("shown-first-time-msg", true)) {
	// 	FLAlertLayer::create(
	// 		"CommentMentions",
	// 		"Thank you for using <co>CommentMentions!</c>. Make sure to change the <cj>tags option</c> in the mod's settings",
	// 		"OK"
	// 	)->show();
	// }
	async::spawn([] -> arc::Future<> {
		g_commentManager = std::make_shared<CommentMentions::CommentManager>();

		bool useDailyLvl = Mod::get()->getSettingValue<bool>("use-daily-lvl");
		if (useDailyLvl) {
			auto handle = async::spawn(CommentMentions::getSpecialID("21"));
			auto id = co_await CommentMentions::getSpecialID("21");
			if (id.isOk()) g_commentManager->addTargetID(id.unwrap());
			else log::error("Error when fetching daily ID: {}", id.unwrapErr());
		}

		auto fixedID = Mod::get()->getSettingValue<int64_t>("fixed-id");
		g_commentManager->addTargetID(fixedID);

		g_commentManager->startAll();
	});
}