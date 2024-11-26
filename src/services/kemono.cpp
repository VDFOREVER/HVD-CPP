#include <services/kemono.hpp>

std::vector<PostData> Kemono::parse(const std::string& tag) {
    std::vector<std::string> tags = Utils::split(tag, '/');
    if (tags.size() < 2 || tags.size() > 2) {
        LOG_WARN("Error split");
        return {};
    }

    std::pair<std::string, long> request = Utils::request(fmt::format("{}{}/user/{}", getURL(), tags[0], tags[1]));
    if (request.first.empty())
        return {};

    std::vector<PostData> tmp;

    try {
        json data = json::parse(request.first);

        for (const auto& item : data) {
            std::string id = item.at("id");

            for (const auto& attachment : item.at("attachments")) {
                std::string file_path = BASE_URL + attachment.at("path").get<std::string>();
                std::vector<std::string> content = {file_path};
                std::vector<std::string> tags = {};

                PostData data(content, tags, id, getService());
                tmp.emplace_back(data);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Type error: {}", e.what());
    }

    return tmp;
}