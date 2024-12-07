#pragma once

#include <services/service.hpp>
#include <pugixml.hpp>
#include <utils.hpp>
#include <cpr/cookies.h>

class Gelbooru : public Service {
    public:
        std::vector<PostData> parse(const std::string& tag) override;
        std::string getService() override {return "gelbooru";};
        std::string getPostURL(const Send& send) override {return "https://gelbooru.com/index.php?page=post&s=view&id=" + send.getID();};
        std::string getURL() override {return "https://gelbooru.com/index.php?page=dapi&s=post&q=index&tags=";};
        std::pair<std::string, long> request(const std::string& url) override;
};