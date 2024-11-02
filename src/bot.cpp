#include <bot.hpp>

Bot::Bot(const std::string &token, DB& db, std::vector<std::shared_ptr<Service>>& services, const std::string& admin) : bot(token), db(db), services(services), admin(admin) {
    bot.getEvents().onCommand("help", [this, &db](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        bot.getApi().sendMessage(message->chat->id, helpMessage);
    });

    bot.getEvents().onCommand("adduser", [this, &db, &services, &admin](TgBot::Message::Ptr message) {
        if (std::to_string(message->chat->id) != admin) {
            bot.getApi().sendMessage(message->chat->id, "Not Permission");
            return;
        }

        if (db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Exist user");
            return;
        }

        db.addUser(message->chat->id, services);
        bot.getApi().sendMessage(message->chat->id, "add user");
    });

    bot.getEvents().onCommand("addtag", [this, &db, &services](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Erroe: /addtag {service} {tag}");
            return;
        }

        auto service = Utils::findServiceByName(services, args[1]);
        if (service == nullptr) {
            bot.getApi().sendMessage(message->chat->id, "Service not found");
            return;
        }

        db.addTag(service, args[2], message->chat->id);
        bot.getApi().sendMessage(message->chat->id, "add tag");
    });

    bot.getEvents().onCommand("rmtag", [this, &db](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Erroe: /rmtag {service} {tag}");
            return;
        }

        db.rmTag(args[1], args[2], message->chat->id);
        bot.getApi().sendMessage(message->chat->id, "Rm tag");
    });

    bot.getEvents().onCommand("addantitag", [this, &db](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Erroe: /addantitag {service} {tag}");
            return;
        }

        db.addAntiTag(args[1], args[2], message->chat->id);
        bot.getApi().sendMessage(message->chat->id, "add Antitag");
    });

    bot.getEvents().onCommand("rmantitag", [this, &db](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Erroe: /rmantitag {service} {tag}");
            return;
        }

        db.rmAntiTag(args[1], args[2], message->chat->id);
        bot.getApi().sendMessage(message->chat->id, "Rm Antitag");
    });

    bot.getEvents().onCommand("taglist", [this, &db](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        bot.getApi().sendMessage(message->chat->id, db.getFormattedTagsAndAntiTags(message->chat->id));
    });
}

void Bot::run() {
    try {
        bot.getApi().deleteWebhook();

        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void Bot::sendImages(const std::vector<Send>& send, std::int64_t user_id, std::shared_ptr<Service> service) {
    for (const auto& photo : send) {
        LOG_INFO("Send: {}", photo.getPost());
        try {
            std::string caption = fmt::format("[{}]({})", service->getService(), service->getPostURL(photo));
            bot.getApi().sendPhoto(user_id, photo.getPost(), caption, nullptr, nullptr, "MarkdownV2");
        } catch (const std::exception& e) {
            LOG_WARN("Erroe send image: {}", e.what());
            bot.getApi().sendMessage(user_id, photo.getPost());
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void Bot::parser() {
    for (const auto& service : services) {
        LOG_INFO("Initialization: {}", service->getService());
        service->init();
    }

    while (true) {
        for (const auto& service : services) {
            LOG_INFO("Refresh: {}", service->getService());
            service->refresh();

            auto repeatTags = db.getUsersByTags(service->getService());
            for (const auto& data : repeatTags) {
                std::vector<PostData> posts = service->parse(data.first);
                if (posts.empty())
                    continue;

                for (const auto& user: data.second) {
                    auto history = db.getHistoryForUserAndSite(user, service->getService());
                    auto antitag = db.getAntiTagForUserAndSite(user, service->getService());
                    std::vector<Send> send;
                    std::vector<std::string> newHistory;

                    for (const auto& post: posts) {
                        for (const auto& content : post.getContent()) {
                            if (Utils::contains(newHistory, content) || Utils::contains(history, content) || Utils::contains(antitag, data.first))
                                continue;

                            LOG_INFO("{}", content);
                            Send sss(content, post.getID(), data.first);
                            send.emplace_back(sss);
                            newHistory.push_back(content);
                        }
                    }
                    sendImages(send, user, service);
                    db.addHistory(service->getService(), newHistory, user);
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        LOG_INFO("Sleep 2h");
        std::this_thread::sleep_for(std::chrono::hours(2));
    }
}