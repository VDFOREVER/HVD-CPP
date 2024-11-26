#include <services/gelbooru.hpp>

std::vector<PostData> Gelbooru::parse(const std::string& tag) {
    std::pair<std::string, long>  request = Utils::request(getURL() + tag, cpr::Cookie("fringeBenefits", "yup"));
    if (request.first.empty())
         return {};

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(request.first.c_str());
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