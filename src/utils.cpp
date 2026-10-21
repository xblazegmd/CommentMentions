#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <fmt/format.h>
#include <fmt/compile.h>
#include <arc/prelude.hpp>
#include <string>

using namespace geode::prelude;

arc::Future<ServerResponse> requestGDServersCM(
    const std::string& endpoint,
    const std::string& body,
    int timeout
) {
    auto req = web::WebRequest()
        .userAgent("")
        .bodyString(body)
        .timeout(std::chrono::seconds(timeout));

    auto res = co_await req.post(fmt::format("https://www.boomlings.com/database/{}", endpoint));
    if (!res.ok()) {
        log::error("Failed to request endpoint '{}' ({}): {}", endpoint, res.code(), res.errorMessage());
        co_return Err(res.code());
    }

    auto ret = res.string();

    if (ret.isErr()) {
        log::error("Could not get response from endpoint '{}': {}", endpoint, ret.unwrapErr());
        co_return Err(0);
    }
    auto unwrapped = ret.unwrap();
    auto num = utils::numFromString<int>(unwrapped);
    if (num.isOk() && num.unwrap() < 0) {
        co_return Err(num.unwrap());
    }

    co_return Ok(unwrapped);
}

std::unordered_map<std::string, std::string> formatResponseCM(const std::string& response, std::string sep) {
    auto pieces = string::split(response, sep);
    std::unordered_map<std::string, std::string> ret;
    ret.reserve(pieces.size() / 2);
    for (size_t i = 0; i < pieces.size(); i += 2) {
        ret.emplace(pieces[i], pieces[i + 1]);
    }
    return ret;
}

std::string buildBodyStringCM(std::initializer_list<std::pair<std::string, std::string>> body) {
    fmt::memory_buffer out;

    for (const auto& [k, v] : body)
        fmt::format_to(std::back_inserter(out), FMT_COMPILE("{}={}&"), k, v);

    out.resize(out.size() - 1);
    return fmt::to_string(out);
}

arc::Future<bool> doWeHaveInternetCM(const std::string& url) {
    auto check = co_await web::WebRequest()
        .userAgent("GeometryDash/2.2091")
        .timeout(std::chrono::seconds(10))
        .get(url);
    co_return check.ok();
}

utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep
) {
    auto kv = formatResponseCM(str, sep);

    utils::StringMap<std::string> ret;
    for (const auto& [k, v] : kv) {
        std::string nk = map.contains(k) ? map[k] : k;
        ret[nk] = v;
    }
    return ret;
}

std::vector<std::string> getListSetting(const std::string& setting) {
    auto value = Mod::get()->getSettingValue<std::string>(setting);
    auto split = string::split(value, ",");

    std::vector<std::string> ret;
    for (const auto& item : split) {
        ret.push_back(string::trim(item));
    }
    return ret;
}

void setListSetting(const std::string& setting, const std::vector<std::string> value) {
    auto dat = string::join(value, ", ");
    Mod::get()->setSettingValue(setting, dat);
}

arc::Future<Result<int>> getSpecialID(LevelType type) {
    auto res = co_await requestGDServersCM("getGJLevels21.php", buildBodyStringCM({
        { "type", utils::numToString(static_cast<int>(type)) },
        { "secret", SECRET }
    }));
    if (res.isErr()) {
        log::error("{}", res.unwrapErr());
        co_return Err("{}", res.unwrapErr());
    }

    auto daily = string::split(string::split(res.unwrap(), "#")[0], "|")[0];
    auto dailyID = formatKV(daily, {{"1", "daily"}})["daily"];
    auto intDailyID = stoicm(dailyID);

    if (intDailyID.isErr()) {
        co_return Err(intDailyID.unwrapErr());
    }

    co_return Ok(intDailyID.unwrap());
}

arc::Future<> pauseUntilWeHaveInternet() {
    while (true) {
        if (co_await doWeHaveInternetCM()) break;
        co_await arc::sleep(asp::Duration::fromSecs(5));
    }
}
