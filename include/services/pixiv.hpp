#pragma once

#include <services/service.hpp>
#include <utils.hpp>
#include <log.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Pixiv : public Service {
    public:
        std::vector<PostData> parse(const std::string& tag) override;
        std::string getService() override {return "pixiv";};
        std::string getPostURL(const Send& send) override {return "https://www.pixiv.net/en/artworks/" + send.getID();};
        std::string getURL() override {return "https://app-api.pixiv.net/v1/user/illusts?&type=illust&offset=0&user_id=";};
        void init() override;
        void refresh() override;
        std::pair<std::string, long> request(const std::string& url) override {
            cpr::Response r = cpr::Get(cpr::Url{url}, cpr::Bearer{accessToken}, cpr::Header{{"Referer", "https://www.pixiv.net/"}});
            
            if (r.status_code != 200) {
                LOG_WARN("Request Error: {} / {}", r.status_code, url);
                return std::make_pair("", r.status_code);
            }

            if (r.text.empty())
                LOG_WARN("Request empty: {}", url);
                
            return std::make_pair(r.text, r.status_code);
        };
        
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