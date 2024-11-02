#pragma once

#include <string>
#include <vector>

class Send {
    public:
        Send(const std::string& post, const std::string& id, const std::string& tag): post(post), id(id), tag(tag) {}
        std::string getPost() const { return post; }
        std::string getID() const { return id; }
        std::string getTag() const { return tag; }

    private:
        std::string post;
        std::string id;
        std::string tag;
};

class PostData {
    public:
        PostData(std::vector<std::string>& content, std::vector<std::string>& tags, std::string& id, std::string service) : content(content), tags(tags), id(id), service(service) {}
        std::vector<std::string> getContent() const { return content; }
        std::vector<std::string> getTags() const { return tags; }
        std::string getID() const { return id; }
        std::string getService() const { return service; }

    private:
        std::vector<std::string> content;
        std::vector<std::string> tags;
        std::string id;
        std::string service;
};

class Service {
    public:
        virtual std::vector<PostData> parse(const std::string& tag) = 0;
        virtual std::string getService() = 0;
        virtual std::string getPostURL(const Send& send) = 0;
        virtual std::string getURL() = 0;
        virtual void init() {};
        virtual void refresh() {};
};