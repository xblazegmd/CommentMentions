#include <arc/prelude.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

using namespace geode::prelude;

const std::string BOOMLINGS = "http://www.boomlings.com/database/";
const std::string SECRET = "Wmfd2893gb7";

utils::StringMap<std::string> formatKV(
    const std::string& str,
    utils::StringMap<std::string> map,
    const std::string& sep = ":"
) {
    auto parts = string::split(str, sep);

    utils::StringMap<std::string> kv;
    for (int i = 0; i + 1 < parts.size(); i += 2) {
        kv[parts[i]] = parts[i + 1];
    }

    utils::StringMap<std::string> ret;
    for (const auto& [k, v] : kv) {
        auto it = map.find(k);
        std::string newk = it != map.end() ? it->second : k;
        ret[newk] = v;
    }

    return ret;
}

class MentionManager {
public:
    MentionManager(int levelID) : m_levelID(levelID) {};
    ~MentionManager() = default;

    void startListening() {
        m_listenerTask.spawn(
            "MentionManager::listener",
            commentListener(),
            [] {}
        );
    }
private:
    int m_levelID;
    std::vector<std::string> m_tags{ "xblazegmd", "xblaze", "blaze"};
    TaskHolder<> m_listenerTask;

    struct CommentObject {
        utils::StringMap<std::string> comment;
        utils::StringMap<std::string> author;
    };

    std::vector<CommentObject> m_previousMentions;

    arc::Future<> commentListener() {
        while (true) {
            co_await arc::sleep(asp::Duration::fromSecs(10));

            auto req = web::WebRequest()
                .userAgent("")
                .timeout(std::chrono::seconds(10))
                .bodyString("levelID=" + utils::numToString(m_levelID) + "&page=0&secret=" + SECRET);

            auto res = co_await req.post(BOOMLINGS + "getGJComments21.php");

            if (!res.ok() || res.string().isErr()) {
                log::error("Request failed: (status code: {})", res.code());
                continue;
            }

            std::string resStr = res.string().unwrap();
            auto resStrNum = utils::numFromString<int>(resStr);

            if (resStrNum.isOk() && resStrNum.unwrap() < 0) {
                log::error("Reqest failed: {}", resStr);
                continue;
            }

            log::debug("{}", resStr);
            auto comments = string::split(resStr, "|");
            for (const auto& comment : comments) {
                auto obj = formatCommentObj(comment);

                log::debug("{}", obj.comment["comment"]);
                auto s = base64::decode(obj.comment["comment"], base64::Base64Variant::Url);
                if (s.isErr()) {
                    log::error("Could not decode comment: {}", s.unwrapErr());
                    continue;
                }
                std::string string(s.unwrap().begin(), s.unwrap().end());

                log::debug("{}", string);

                if (containsMention(string)) {
                    if (isPrevious(obj)) continue;
                    obj.comment["comment"] = std::move(string);
                    log::info("Mention by {}: {}", obj.author["userName"], obj.comment["comment"]);
                    m_previousMentions.push_back(obj);
                    geode::queueInMainThread([this, obj] {
                        onMention(obj);
                    });
                }
            }
        }
    }

    void onMention(CommentObject obj) {
        AchievementNotifier::sharedState()->notifyAchievement(
            fmt::format("{} mentioned you", obj.author["userName"]).c_str(),
            obj.comment["comment"].c_str(),
            "accountBtn_pendingRequest_001.png",
            true
        );
    }

    bool containsMention(const std::string& str) {
        for (const auto& tag : m_tags)
            if (string::contains(string::toLower(str), tag)) { 
                log::debug("contains");
                return true; 
            }
        return false;
    }

    bool isPrevious(CommentObject obj) {
        for (const auto& mention : m_previousMentions) {
            auto messageID = mention.comment.find("messageID");
            if (messageID == mention.comment.end()) return false;
            if (messageID->second == obj.comment["messageID"]) { 
                log::debug("is previous");
                return true; 
            }
        }
        return false;
    }

    CommentObject formatCommentObj(const std::string& str) {
        auto split = string::split(str, ":");
        log::debug("{}", split[0]);
        log::debug("{}", split[1]);

        CommentObject ret;
        ret.comment = formatKV(split[0], {
            { "1", "levelID" },
            { "2", "comment" },
            { "3", "authorPlayerID" },
            { "4", "likes" },
            { "5", "dislikes" },
            { "6", "messageID" },
            { "7", "spam" },
            { "8", "authorAccountID" },
            { "9", "age" },
            { "10", "percent" },
            { "11", "modBadge" },
            { "12", "moderatorChatColor" },
        }, "~");
        ret.author = formatKV(split[1], {
            { "1", "userName" },
            { "9", "icon" },
            { "10", "playerColor" },
            { "11", "playerColor2" },
            { "14", "iconType" },
            { "15", "glow" },
            { "16", "accountID" },
        }, "~");

        return ret;
    }
};

static std::shared_ptr<MentionManager> g_mentionManager;

$on_game(Loaded) {
    async::spawn([] -> arc::Future<> {
        // Get daily level
        auto req = web::WebRequest()
            .userAgent("")
            .bodyString("type=21&secret=" + SECRET)
            .timeout(std::chrono::seconds(10));

        auto res = co_await req.post(BOOMLINGS + "getGJLevels21.php");
        if (!res.ok() || res.string().isErr()) {
            log::error("Request failed: (status code: {})", res.code());
            Notification::create(fmt::format("Error: {}", res.code()), NotificationIcon::Error)->show();
            co_return;
        }

        std::string resStr = res.string().unwrap();
        auto resStrNum = utils::numFromString<int>(resStr);

        if (resStrNum.isOk() && resStrNum.unwrap() < 0) {
            log::error("Reqest failed: {}", resStr);
            Notification::create(fmt::format("Error: {}", resStr), NotificationIcon::Error)->show();
            co_return;
        }

        auto daily = string::split(string::split(resStr, "#")[0], "|")[0];
        auto dailyID = formatKV(daily, {{"1", "daily"}})["daily"];
        auto intDailyID = utils::numFromString<int>(dailyID);

        if (intDailyID.isErr()) {
            Notification::create(fmt::format("Error: {}", intDailyID.unwrapErr()), NotificationIcon::Error)->show();
            co_return;
        }

        g_mentionManager = std::make_shared<MentionManager>(intDailyID.unwrap());
        g_mentionManager->startListening();
    });
}