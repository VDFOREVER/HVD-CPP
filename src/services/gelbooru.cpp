#include <services/gelbooru.hpp>

Gelbooru::Gelbooru() : Service("gelbooru", "https://gelbooru.com/index.php?page=dapi&s=post&q=index&tags=", "https://gelbooru.com/index.php?page=post&s=view&id=") {};

post_data_tv Gelbooru::parse(const std::string& tag) {
    std::string recData = request(url + tag).first;
    if (recData.empty())
         return {};

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(recData.c_str());
    if (!result)
        return {};

    post_data_tv tmp;

    for (pugi::xml_node post = doc.child("posts").child("post"); post; post = post.next_sibling("post")) {
        std::vector<std::string> file_url = { post.child("file_url").text().as_string() };
        std::vector<std::string> tags = Utils::split(post.child("tags").text().as_string(), ' ');
        std::string id = post.child("id").text().as_string();

        post_data_t data(file_url, tags, id, type);
        tmp.emplace_back(data);
    }

    return tmp;
}

std::pair<std::string, long> Gelbooru::request(const std::string& url) {
    cpr::Response r = cpr::Get(cpr::Url{url}, cpr::Cookie("fringeBenefits", "yup"));

    if (r.status_code != 200) {
        LOG_WARN("Request Error: {} / {}", r.status_code, url);
        return std::make_pair("", r.status_code);
    }

    if (r.text.empty())
        LOG_WARN("Request empty: {}", url);

    return std::make_pair(r.text, r.status_code);
};