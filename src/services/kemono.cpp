#include <services/kemono.hpp>

Kemono::Kemono() : Service("kemono", "https://kemono.su/api/v1/", "https://kemono.su/") {}

post_data_tv Kemono::parse(const std::string& tag) {
    std::vector<std::string> tags = Utils::split(tag, '/');
    if (tags.size() < 2 || tags.size() > 2) {
        LOG_WARN("Error split");
        return {};
    }

    std::string reqData = request(fmt::format("{}{}/user/{}", url, tags[0], tags[1])).first;
    if (reqData.empty())
        return {};

    post_data_tv tmp;

    try {
        json data = json::parse(reqData);

        for (const auto& item : data) {
            std::string id = item.at("id");

            for (const auto& attachment : item.at("attachments")) {
                std::string file_path = BASE_URL + attachment.at("path").get<std::string>();
                std::vector<std::string> content = {file_path};
                std::vector<std::string> tags = {};

                post_data_t data(content, tags, id, type);
                tmp.emplace_back(data);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Type error: {}", e.what());
    }

    return tmp;
}