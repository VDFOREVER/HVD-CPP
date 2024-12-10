#include <services/rule34.hpp>

Rule34::Rule34() : Service("rule34", "https://api.rule34.xxx/index.php?page=dapi&s=post&q=index&tags=", "https://rule34.xxx/index.php?page=post&s=view&id=") {};

post_data_tv Rule34::parse(const std::string& tag) {
    std::string reqData = request(url + tag).first;
    if (reqData.empty())
        return {};

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(reqData.c_str());
    if (!result)
        return {};

    post_data_tv tmp;

    for (pugi::xml_node post = doc.child("posts").child("post"); post; post = post.next_sibling("post")) {
        std::vector<std::string> file_url = { post.attribute("file_url").as_string() };
        std::vector<std::string> tags = Utils::split(post.attribute("tags").as_string(), ' ');
        std::string id = post.attribute("id").as_string();
        post_data_t data(file_url, tags, id, type);
        tmp.emplace_back(data);
    }

    return tmp;
}