#pragma once

#include <services/service.hpp>
#include <utils.hpp>
#include <nlohmann/json.hpp>
#include <log.hpp>
using json = nlohmann::json;

class Kemono : public Service {
    public:
        std::vector<PostData> parse(const std::string& tag) override;
        std::string getService() override {return "kemono";};
        std::string getPostURL(const Send& send) override {
            std::vector<std::string> tags = Utils::split(send.getTag(), '/');
            if (tags.size() < 2 || tags.size() > 2) {
                LOG_WARN("Error split");
                return BASE_URL;
            }

            return fmt::format("https://kemono.su/{}/user/{}/post/{}", tags[0], tags[1],send.getID());
        };
        std::string getURL() override {return "https://kemono.su/api/v1/";};

    private:
        const std::string BASE_URL = "https://kemono.su/";
};