#include <services/kemono.hpp>
#include <services/rule34.hpp>
#include <services/gelbooru.hpp>
#include <services/pixiv.hpp>
#include <bot.hpp>
#include <commands.hpp>

std::unique_ptr<Bot> bot;

void signalHandler(int signum) {
    LOG_INFO("Signal {} received", signum);

    if (signum != SIGSEGV)
        bot.reset();

    exit(signum);
}

int main() {
    std::string token(std::getenv("TOKEN"));

    if (token.empty()) {
        LOG_CRITICAL("ENV variable is NULL (TOKEN)");
        return 1;
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGSEGV, signalHandler);

    bot = std::make_unique<Bot>(token);

    bot->addCommand("help", std::vector<std::string>{}, false, cmd_help);
    bot->addCommand("adduser", std::vector<std::string>{ "id" }, true, cmd_adduser);
    bot->addCommand("deluser", std::vector<std::string>{ "id" }, true, cmd_deluser);
    bot->addCommand("addtag", std::vector<std::string>{ "service", "tag" }, false, cmd_addtag);
    bot->addCommand("deltag", std::vector<std::string>{ "service", "tag" }, false, cmd_deltag);
    bot->addCommand("addantitag", std::vector<std::string>{ "service", "tag" }, false, cmd_addantitag);
    bot->addCommand("delantitag", std::vector<std::string>{ "service", "tag" }, false, cmd_delantitag);
    bot->addCommand("taglist", std::vector<std::string>{}, false, cmd_taglist);

    bot->addService<Rule34>();
    bot->addService<Gelbooru>();
    bot->addService<Kemono>();
    bot->addService<Pixiv>();

    bot->start();

    return 0;
}