#pragma once

#include <services/service.hpp>
#include <utils.hpp>
#include <nlohmann/json.hpp>
#include <log.hpp>
using json = nlohmann::json;

class Kemono : public Service {
    public:
        Kemono();

        post_data_tv parse(const std::string& tag) override;

        std::string buildPostURL(const send_t& send) override {
            std::vector<std::string> tags = Utils::split(send.tag, '/');
            if (tags.size() < 2 || tags.size() > 2) {
                LOG_WARN("Error split");
                return BASE_URL;
            }

            return fmt::format("{}{}/user/{}/post/{}", postUrl, tags[0], tags[1], send.id);
        };

    private:
        const std::string BASE_URL = "https://kemono.su/";
};