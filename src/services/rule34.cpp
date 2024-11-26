#include <services/rule34.hpp>

std::vector<PostData> Rule34::parse(const std::string& tag) {
    std::pair<std::string, long>  request = Utils::request(getURL() + tag);
    if (request.first.empty())
         return {};

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(request.first.c_str());
    if (!result)
        return {};
    
    std::vector<PostData> tmp;

    for (pugi::xml_node post = doc.child("posts").child("post"); post; post = post.next_sibling("post")) {
        std::vector<std::string> file_url = { post.attribute("file_url").as_string() };
        std::vector<std::string> tags = Utils::split(post.attribute("tags").as_string(), ' ');
        std::string id = post.attribute("id").as_string();
        PostData data(file_url, tags, id, getService());
        tmp.emplace_back(data);
    }

    return tmp;
}