#pragma once

#include <services/service.hpp>
#include <utils.hpp>
#include <log.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Pixiv : public Service {
    public:
        Pixiv();

        post_data_tv parse(const std::string& tag) override;
        void refresh() override;
        std::pair<std::string, long> request(const std::string& url) override;

    private:
        void UpdateTokens(const std::string& jsons);
        std::string accessToken;
        std::string refreshToken;

        const std::string USER_AGENT = "PixivAndroidApp/5.0.234 (Android 11; Pixel 5)";
        const std::string REDIRECT_URI = "https://app-api.pixiv.net/web/v1/users/auth/pixiv/callback";
        const std::string LOGIN_URL = "https://app-api.pixiv.net/web/v1/login";
        const std::string CLIENT_ID = "MOBrBDS8blbauoSck0ZfDbtuzpyT";
        const std::string CLIENT_SECRET = "lsACyCD94FhDUtGTXi3QzcFE2uU1hqtDaKeqrdwj";
        const std::string AUTH_TOKEN_URL = "https://oauth.secure.pixiv.net/auth/token";
};