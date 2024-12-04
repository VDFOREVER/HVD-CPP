#include <services/kemono.hpp>
#include <services/rule34.hpp>
#include <services/gelbooru.hpp>
#include <services/service.hpp>
#include <services/pixiv.hpp>
#include <memory>
#include <db.hpp>
#include <thread>
#include <bot.hpp>

int main() {
    std::string token(std::getenv("TOKEN"));
    std::string admin(std::getenv("ADMIN"));

    if (token.empty() || admin.empty()) {
        LOG_CRITICAL("ENV variable is NULL (TOKEN and/or ADMIN)");
        return 1;
    }

    DB db("data.db");

    std::vector<std::shared_ptr<Service>> services;
    services.push_back(std::make_shared<Rule34>());
    services.push_back(std::make_shared<Gelbooru>());
    services.push_back(std::make_unique<Kemono>());
    services.push_back(std::make_unique<Pixiv>());
    
    signal(SIGINT, [](int s) {
        exit(0);
    });

    Bot bot(token, db, services, admin);
    std::thread botThead([&]() { bot.run(); });
    std::thread parserThread([&]() { bot.parser(); });

    botThead.join();
    parserThread.join();

    return 0;
}