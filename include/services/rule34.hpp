#pragma once

#include <services/service.hpp>
#include <pugixml.hpp>
#include <utils.hpp>

class Rule34 : public Service {
    public:
        std::vector<PostData> parse(const std::string& tag) override;
        std::string getService() override {return "rule34";};
        std::string getPostURL(const Send& send) override {return "https://rule34.xxx/index.php?page=post&s=view&id=" + send.getID();};
        std::string getURL() override {return "https://api.rule34.xxx/index.php?page=dapi&s=post&q=index&tags=";};
};