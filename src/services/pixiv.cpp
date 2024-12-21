#include <services/pixiv.hpp>

Pixiv::Pixiv() : Service("pixiv", "https://app-api.pixiv.net/v1/user/illusts?&type=illust&offset=0&user_id=", "https://www.pixiv.net/en/artworks/") {
    std::string codeVerifier = Utils::generate_urlsafe_token(32);
    std::string codeChallenge = Utils::urlsafe_b64encode(Utils::sha256(codeVerifier));
    LOG_INFO("{}?code_challenge={}&code_challenge_method=S256&client=pixiv-android", LOGIN_URL, codeChallenge);

    std::string code;
    std::cout << "Input code: ";
    std::cin >> code;

    std::vector<cpr::Pair> payload = {
        {"code_verifier", codeVerifier},
        {"code", code},
        {"grant_type", "authorization_code"},
        {"redirect_uri", REDIRECT_URI},
        {"client_id", CLIENT_ID},
        {"client_secret", CLIENT_SECRET},
        {"include_policy", "true"}
    };

    cpr::Response response = cpr::Post(
        cpr::Url{AUTH_TOKEN_URL},
        cpr::Payload{payload.begin(), payload.end()},
        cpr::Header{{"User-Agent", USER_AGENT}}
    );

    UpdateTokens(response.text);
}

post_data_tv Pixiv::parse(const std::string& tag) {
    if (accessToken.empty()) {
        LOG_ERROR("Access token is empty");
        return {};
    }

    std::pair<std::string, long> reqData = request(url + tag);

    if (reqData.second == 400) {
        LOG_WARN("force refresh token");
        refresh();
        reqData = request(url + tag);
    }

    if (reqData.first.empty())
         return {};

    post_data_tv tmp;
    try {
        json data = json::parse(reqData.first);

        for (const auto& illusts : data.at("illusts")) {
            std::string id = std::to_string(illusts.at("id").get<int>());
            json metaSinglePage = illusts.at("meta_single_page");
            std::vector<std::string> content;
            if (!metaSinglePage.empty())
                content = { metaSinglePage.at("original_image_url").get<std::string>() };
            else {
                json metaPages = illusts.at("meta_pages");
                for (const auto& images: metaPages) {
                    std::string image = images.at("image_urls").at("original").get<std::string>();
                    content.push_back(image);
                }
            }
            std::vector<std::string> tagv;
            json tags = illusts.at("tags");
            for (const auto& tag: tags) {
                std::string ctag = tag.at("name").get<std::string>();
                tagv.push_back(ctag);
            }

            int score = illusts.at("total_bookmarks").get<int>();

            tmp.emplace_back(content, tagv, id, type, score);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Type error: {}", e.what());
    }

    return tmp;
}

void Pixiv::refresh() {
    std::vector<cpr::Pair> payload = {
        {"grant_type", "refresh_token"},
        {"refresh_token", refreshToken},
        {"client_id", CLIENT_ID},
        {"client_secret", CLIENT_SECRET},
        {"include_policy", "true"}
    };

    cpr::Response response = cpr::Post(
        cpr::Url{AUTH_TOKEN_URL},
        cpr::Payload{payload.begin(), payload.end()},
        cpr::Header{{"User-Agent", USER_AGENT}}
    );

    UpdateTokens(response.text);
}

void Pixiv::UpdateTokens(const std::string& jsons) {
    try {
        json data = json::parse(jsons);
        accessToken = data.at("access_token").get<std::string>();
        refreshToken = data.at("refresh_token").get<std::string>();
    } catch (const std::exception& e) {
        LOG_ERROR("Type error: {}", e.what());
    }
}

std::pair<std::string, long> Pixiv::request(const std::string& url) {
    cpr::Response r = cpr::Get(cpr::Url{url}, cpr::Bearer{accessToken}, cpr::Header{{"Referer", "https://www.pixiv.net/"}});

    if (r.status_code != 200) {
        LOG_WARN("Request Error: {} / {}", r.status_code, url);
        return std::make_pair("", r.status_code);
    }

    if (r.text.empty())
        LOG_WARN("Request empty: {}", url);

    return std::make_pair(r.text, r.status_code);
};