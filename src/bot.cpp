#include <bot.hpp>

Bot::Bot(const std::string &token) : TgBot::Bot(token), db("data.db") {
    getEvents().onUnknownCommand([this](TgBot::Message::Ptr message) {
        this->command_handler(message);
    });
}

void Bot::start() {
    LOG_INFO("Bot started");
    is_running = true;
    mainloop();
}

void Bot::stop() {
    LOG_INFO("Bot stopped");
    is_running = false;
}

Bot::~Bot() {
    stop();
}

void Bot::command_handler(TgBot::Message::Ptr message) {
    LOG_INFO("Command: {} UserID: {} User: {} {}", message->text, message->chat->id, message->chat->firstName, message->chat->lastName);

    if (message->text.empty())
        return;

    std::vector<std::string> args = Utils::split(message->text, ' ');

    std::string name = args[0].substr(1);
    args.erase(args.begin());

    for (const auto& command : commands) {
        if (name == command->name) {
            if (!db.userExist(message->chat->id, command->only_admin)) {
                if (!db.isUserTableEmpty()) {
                    replyMessage(message, "You don't have permission for using this command");
                    return;
                }

                replyMessage(message, "Users table is empty, adding you as admin");
                db.addUser(message->chat->id, true);
            }

            if (args.size() != command->args.size()) {
                replyMessage(message, "Wrong number of arguments: expected {} got {}\nUsage: {}", command->args.size(), args.size(), command->get_help());
                return;
            }

            try {
                command->handler(*this, message, args);
            } catch (const std::exception& e) {
                replyMessage(message, "CommandError: {}", e.what());
            }

            return;
        }
    }

    replyMessage(message, "Unknown command: {}", message->text);
}

void Bot::sendContent(const send_t& send, std::int64_t user_id, std::shared_ptr<Service> service) {
    std::filesystem::path url = send.content;
    LOG_INFO("Send: {}", url.string());

    std::string data = service->request(url).first;

    TgBot::InputFile::Ptr file = std::make_shared<TgBot::InputFile>();
    file->data = data;
    file->mimeType = Utils::getMimeType(url);
    file->fileName = url.filename();

    std::string caption = fmt::format("[{}]({})\n[original]({})", service->type, service->buildPostURL(send), url.string());
    static std::string mode = "MarkdownV2";

    try {
        if (url.extension() == ".mp4")
            getApi().sendVideo(user_id, file, false, 0, 0, 0, "", caption, nullptr, nullptr, mode);
        else
            getApi().sendPhoto(user_id, file, caption, nullptr, nullptr, mode);
    } catch (const std::exception& e) {
        try {
            getApi().sendDocument(user_id, file, "", caption, nullptr, nullptr, mode);
        } catch (const std::exception& e) {
            getApi().sendMessage(user_id, caption, nullptr, nullptr, nullptr, mode);
        }
    }
}

void Bot::mainloop() {
    // Not really good, but also not really bad
    // Can cause segfaults and some other memory access errors
    auto update_services_threaded = [this]() {
        std::thread(&Bot::update_services, this).detach();
    };

    auto last_update = std::chrono::steady_clock::now();
    update_services_threaded();

    getApi().deleteWebhook();

    TgBot::TgLongPoll longPoll((TgBot::Bot&)*this);
    while (true) {
        try {
            longPoll.start();
        } catch (const std::exception& e) {
            LOG_ERROR("Bot: {}", e.what());
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::hours>(now - last_update).count();
        if (diff >= 2) {
            update_services_threaded();
            last_update = now;
            LOG_INFO("Sleep 2h");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::this_thread::yield();
    }
}

void Bot::update_services() {
    for (const auto& service : services) {
        LOG_INFO("Refresh: {}", service->type);
        service->refresh();

        auto repeatTags = db.getUsersByTags(service->type);
        for (const auto& [tag, users] : repeatTags) {
            post_data_tv posts = service->parse(tag);
            if (posts.empty())
                continue;

            for (const auto& user: users) {
                std::vector<std::string> history = db.getHistory(user, service->type);
                std::vector<std::string> antitag = db.getAntiTagForUserAndSite(user, service->type);

                for (const auto& post: posts) {
                    auto tags = post.tags;
                    if (Utils::contains(antitag, tags) || Utils::contains(history, post.id))
                        continue;

                    for (const auto& content : post.content) {
                        if ((db.getScore(service, user) < post.score) && db.getScore(service, user) != 0)
                            continue; 

                        send_t tmp;
                        tmp.content = content;
                        tmp.id = post.id;
                        tmp.tag = tag;

                        sendContent(tmp, user, service);
                    }

                    db.addHistory(service->type, post.id, user);
                }

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    LOG_INFO("Refreshing complete");
}

std::shared_ptr<Service> Bot::findServiceByName(const std::string& name) {
    for (const auto& service : services) {
        if (service->type == name)
            return service;
    }

    return nullptr;
}
