#include <services/pixiv.hpp>

std::vector<PostData> Pixiv::parse(const std::string& tag) {
    LOG_INFO("{}", accessToken);

    if (accessToken.empty()) {
        LOG_ERROR("Access token is empty");
        return {};
    }

    std::string request = Utils::request(getURL() + tag, cpr::Bearer{accessToken});
    if (request.empty())
         return {};

    std::vector<PostData> tmp;
    try {
        json data = json::parse(request);

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

            PostData data(content, tagv, id, getService());
            tmp.emplace_back(data);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Type error: {}", e.what());
    }

    return tmp;
}

void Pixiv::init() {
    std::string codeVerifier = Utils::generate_urlsafe_token(32);
    std::string codeChallenge = Utils::urlsafe_b64encode(Utils::sha256(codeVerifier));
    LOG_INFO("{}?code_challenge={}&code_challenge_method=S256&client=pixiv-android", LOGIN_URL, codeChallenge);

    std::string code;
    LOG_INFO("Input code: ")
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