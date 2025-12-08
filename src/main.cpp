#include <Geode/Geode.hpp>
#include <Geode/loader/ModEvent.hpp>
#include <core/utils.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
	log::info("Successfully loaded CommentMentions");
	CMentions::utils::notify("Notification", "This notification's useless rn");
}