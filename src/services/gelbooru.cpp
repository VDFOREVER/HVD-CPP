#include <services/gelbooru.hpp>

std::vector<PostData> Gelbooru::parse(const std::string& tag) {
    std::string recData = request(getURL() + tag).first;
    if (recData.empty())
         return {};

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(recData.c_str());
    if (!result)
        return {};
    
    std::vector<PostData> tmp;

    for (pugi::xml_node post = doc.child("posts").child("post"); post; post = post.next_sibling("post")) {
        std::vector<std::string> file_url = { post.child("file_url").text().as_string() };
        std::vector<std::string> tags = Utils::split(post.child("tags").text().as_string(), ' ');
        std::string id = post.child("id").text().as_string();
        
        PostData data(file_url, tags, id, getService());
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