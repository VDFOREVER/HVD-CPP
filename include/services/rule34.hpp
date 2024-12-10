#pragma once

#include <services/service.hpp>
#include <pugixml.hpp>
#include <utils.hpp>

class Rule34 : public Service {
    public:
        Rule34();
        post_data_tv parse(const std::string& tag) override;
};