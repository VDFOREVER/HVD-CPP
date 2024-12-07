#pragma once

#include <string>
#include <vector>
#include <log.hpp>
#include <db.hpp>
#include <tgbot/tgbot.h>
#include <utils.hpp>
#include <services/service.hpp>
#include <filesystem>
#include <memory>

class Bot {
    public:
        Bot(const std::string& token, const std::string& admin);
        ~Bot();

        template<typename T>
        void addService() {
            services.push_back(std::make_shared<T>());
        }

    private:
        TgBot::Bot bot;
        DB db;
        std::string admin;
        std::vector<std::shared_ptr<Service>> services;
        std::thread botThead;
        std::thread parserThread;
        void sendContent(const std::vector<Send>& send, std::int64_t user_id, std::shared_ptr<Service> service);
        void parser();
        void run();
        const std::string helpMessage =
            "/help\n"
            "/addtag {service} {tag}\n"
            "/rmtag {service} {tag}\n"
            "/addantitag {service} {tag}\n"
            "/rmantitag {service} {tag}\n"
            "/taglist\n\n"
            "Available services:\n"
            "    rule34\n"
            "    gelbooru\n"
            "    kemono\n"
            "    pixiv\n";
};