#pragma once

#include <services/service.hpp>
#include <pugixml.hpp>
#include <utils.hpp>
#include <cpr/cookies.h>

class Gelbooru : public Service {
    public:
        Gelbooru();
        post_data_tv parse(const std::string& tag) override;
        std::pair<std::string, long> request(const std::string& url) override;
};