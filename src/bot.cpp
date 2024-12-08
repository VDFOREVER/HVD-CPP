#include <bot.hpp>

Bot::Bot(const std::string &token, const std::string& admin) : bot(token), db("data.db"), admin(admin) {
    bot.getEvents().onCommand("help", [this](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        bot.getApi().sendMessage(message->chat->id, helpMessage);
    });

    bot.getEvents().onCommand("adduser", [this, &admin](TgBot::Message::Ptr message) {
        try {
            if (std::to_string(message->chat->id) != admin) {
                bot.getApi().sendMessage(message->chat->id, "Not Permission");
                return;
            }

            std::vector<std::string> args = Utils::split(message->text, ' ');
            if (args.size() < 2) {
                bot.getApi().sendMessage(message->chat->id, "Error: /adduser {id}");
                return;
            }

            std::int64_t id = std::stoll(args[1]);
            if (db.userExist(id)) {
                bot.getApi().sendMessage(message->chat->id, "Exist user");
                return;
            }

            db.addUser(id, services);
            bot.getApi().sendMessage(message->chat->id, "add user");
        } catch (const std::exception& e) {
            bot.getApi().sendMessage(message->chat->id, fmt::format("Error: {}", e.what()));
        }
    });

    bot.getEvents().onCommand("rmuser", [this, &admin](TgBot::Message::Ptr message) {
        try {
            if (std::to_string(message->chat->id) != admin) {
                bot.getApi().sendMessage(message->chat->id, "Not Permission");
                return;
            }

            std::vector<std::string> args = Utils::split(message->text, ' ');
            if (args.size() < 2) {
                bot.getApi().sendMessage(message->chat->id, "Error: /rmuser {id}");
                return;
            }

            std::int64_t id = std::stoll(args[1]);
            if (!db.userExist(id)) {
                bot.getApi().sendMessage(message->chat->id, "Exist user");
                return;
            }

            db.rmUser(id, services);
            bot.getApi().sendMessage(message->chat->id, "rm user");
        } catch (const std::exception& e) {
            bot.getApi().sendMessage(message->chat->id, fmt::format("Error: {}", e.what()));
        }
    });


    bot.getEvents().onCommand("addtag", [this](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Error: /addtag {service} {tag}");
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

    bot.getEvents().onCommand("rmtag", [this](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Error: /rmtag {service} {tag}");
            return;
        }

        db.rmTag(args[1], args[2], message->chat->id);
        bot.getApi().sendMessage(message->chat->id, "Rm tag");
    });

    bot.getEvents().onCommand("addantitag", [this](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Error: /addantitag {service} {tag}");
            return;
        }

        db.addAntiTag(args[1], args[2], message->chat->id);
        bot.getApi().sendMessage(message->chat->id, "add Antitag");
    });

    bot.getEvents().onCommand("rmantitag", [this](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        std::vector<std::string> args = Utils::split(message->text, ' ');
        if (args.size() < 3) {
            bot.getApi().sendMessage(message->chat->id, "Error: /rmantitag {service} {tag}");
            return;
        }

        db.rmAntiTag(args[1], args[2], message->chat->id);
        bot.getApi().sendMessage(message->chat->id, "Rm Antitag");
    });

    bot.getEvents().onCommand("taglist", [this](TgBot::Message::Ptr message) {
        if (!db.userExist(message->chat->id)) {
            bot.getApi().sendMessage(message->chat->id, "Not Fount user");
            return;
        }

        bot.getApi().sendMessage(message->chat->id, db.getFormattedTagsAndAntiTags(message->chat->id));
    });

    botThead = std::thread(&Bot::run, this);
    parserThread = std::thread(&Bot::parser, this);
}

Bot::~Bot() {
    botThead.join();
    parserThread.join();
}

void Bot::run() {
    bot.getApi().deleteWebhook();

    TgBot::TgLongPoll longPoll(bot);
    while (true) {
        try {
            longPoll.start();
        } catch (const std::exception& e) {
            LOG_ERROR("Bot: {}", e.what());
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

void Bot::sendContent(const std::vector<Send>& send, std::int64_t user_id, std::shared_ptr<Service> service) {
    for (const auto& photo : send) {
        std::filesystem::path url = photo.getPost();
        LOG_INFO("Send: {}", url.string());

        std::string data = service->request(url).first; 

        TgBot::InputFile::Ptr file = std::make_shared<TgBot::InputFile>();
        file->data = data;
        file->mimeType = Utils::getMimeType(url);
        file->fileName = url.filename();

        std::string caption = fmt::format("[{}]({})\n[original]({})", service->getService(), service->getPostURL(photo), url.string());
        static std::string mode = "MarkdownV2";
        
        try {
            if (url.extension() == ".mp4")
                bot.getApi().sendVideo(user_id, file, false, 0, 0, 0, "", caption, nullptr, nullptr, mode);
            else
                bot.getApi().sendPhoto(user_id, file, caption, nullptr, nullptr, mode);
        } catch (const std::exception& e) {
            try {
                bot.getApi().sendDocument(user_id, file, "", caption, nullptr, nullptr, mode);
            } catch (const std::exception& e) {
                bot.getApi().sendMessage(user_id, caption, nullptr, nullptr, nullptr, mode);
            }
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
            for (const auto& [tag, users] : repeatTags) {
                std::vector<PostData> posts = service->parse(tag);
                if (posts.empty())
                    continue;

                for (const auto& user: users) {
                    std::vector<std::string> history = db.getHistory(user, service->getService(), tag);
                    std::vector<std::string> antitag = db.getAntiTagForUserAndSite(user, service->getService());
                    std::vector<Send> send;
                    std::vector<std::string> newHistory;

                    for (const auto& post: posts) {
                        auto tags = post.getTags();
                        if (Utils::contains(antitag, tags))
                            continue;

                        for (const auto& content : post.getContent()) {
                            if (Utils::contains(newHistory, content) || Utils::contains(history, content))
                                continue;

                            Send sss(content, post.getID(), tag);
                            send.emplace_back(sss);
                            newHistory.push_back(content);
                        }
                    }

                    sendContent(send, user, service);
                    db.addHistory(service->getService(), newHistory, user);
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        LOG_INFO("Sleep 2h");
        std::this_thread::sleep_for(std::chrono::hours(2));
    }
}