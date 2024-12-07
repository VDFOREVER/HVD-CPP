#include <services/kemono.hpp>
#include <services/rule34.hpp>
#include <services/gelbooru.hpp>
#include <services/pixiv.hpp>
#include <bot.hpp>

std::unique_ptr<Bot> bot;

void signalHandler(int signum) {
    bot.reset();

    exit(signum);
}

int main() {
    std::string token(std::getenv("TOKEN"));
    std::string admin(std::getenv("ADMIN"));

    if (token.empty() || admin.empty()) {
        LOG_CRITICAL("ENV variable is NULL (TOKEN and/or ADMIN)");
        return 1;
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGSEGV, signalHandler);

    bot = std::make_unique<Bot>(token, admin);
    bot->addService<Rule34>();
    bot->addService<Gelbooru>();
    bot->addService<Kemono>();
    bot->addService<Pixiv>();

    return 0;
}