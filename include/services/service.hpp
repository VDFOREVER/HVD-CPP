#pragma once

#include <string>
#include <vector>
#include <log.hpp>
#include <cpr/cpr.h>

typedef struct {
    std::string post;
    std::string id;
    std::string tag;
} send_t;

typedef struct {
    std::vector<std::string> content;
    std::vector<std::string> tags;
    std::string id;
    std::string service;
} post_data_t;

typedef std::vector<post_data_t>   post_data_tv;
typedef std::vector<send_t>        send_tv;

class Service {
    public:
        const std::string type;
        const std::string url;
        const std::string postUrl;

        Service(std::string type, std::string url, std::string postUrl) : type(type), url(url), postUrl(postUrl) {};

        virtual void refresh() {};
        virtual post_data_tv parse(const std::string& tag) = 0;
        virtual std::string buildPostURL(const send_t& send) {
            return postUrl + send.id;
        }
        virtual std::pair<std::string, long> request(const std::string& url) {
            cpr::Response r = cpr::Get(cpr::Url{url});

            if (r.status_code != 200) {
                LOG_WARN("Request Error: {} / {}", r.status_code, url);
                return std::make_pair("", r.status_code);
            }

            if (r.text.empty())
                LOG_WARN("Request empty: {}", url);

            return std::make_pair(r.text, r.status_code);
        };
};