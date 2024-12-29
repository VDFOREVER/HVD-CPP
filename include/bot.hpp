#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

#include <tgbot/tgbot.h>

#include <log.hpp>
#include <db.hpp>
#include <utils.hpp>
#include <services/service.hpp>

class Bot : public TgBot::Bot {
    public:
        typedef void (*CommandHandler)(Bot&, TgBot::Message::Ptr, std::vector<std::string>);

        class Command {
            public:
                const std::string name;
                const std::vector<std::string> args;
                const bool only_admin;
                const CommandHandler handler;
                const std::string help;

                Command(std::string name, std::vector<std::string> args, bool only_admin, CommandHandler handler, std::string help) :
                    name(name),
                    args(args),
                    only_admin(only_admin),
                    handler(handler),
                    help(help) {};

                Command(std::string name, std::vector<std::string> args, bool only_admin, CommandHandler handler) :
                    Command(name, args, only_admin, handler, "") {};

                std::string get_help() {
                    std::string help_str = "";
                    std::string args_str = "";

                    for (const auto& arg : args)
                        args_str += fmt::format("<{}> ", arg);

                    help_str += fmt::format("\t/{} {}", name, args_str);

                    if (help.size() > 0)
                        help_str += fmt::format(":\t{}", help);

                    return help_str;
                }
        };

        Bot(const std::string& token);
        ~Bot();

        void command_handler(TgBot::Message::Ptr message);

        void start();
        void stop();

        template<typename T>
        void addService() {
            services.push_back(std::make_shared<T>());
        }

        template<typename... Args>
        void addCommand(Args&&... args) {
            commands.push_back(std::make_shared<Command>(args...));
        }

        void sendMessage(int64_t id, const std::string &text) {
            getApi().sendMessage(id, text);
        }

        template<typename... Args>
        void sendMessage(int64_t id, std::string fmt, Args&&... args) {
            const std::string text = fmt::vformat(fmt, fmt::vargs<Args...>{{args...}});
            sendMessage(id, text);
        }

        template<typename... Args>
        void replyMessage(TgBot::Message::Ptr message, std::string fmt, Args&&... args) {
            sendMessage(message->chat->id, fmt, args...);
        }

        std::string helpMessage() {
            std::string result = "Available commands:\n";

            for (const auto& command : commands)
                result += command->get_help() + "\n";

            return result;
        }

        std::vector<std::string> availableServices() {
            std::vector<std::string> result;

            for (const auto& service : services)
                result.push_back(service->type);

            return result;
        }

        DB& getDB() {
            return db;
        }

        std::shared_ptr<Service> findServiceByName(const std::string& name);

    private:
        DB db;

        std::atomic<bool> is_running = false;

        std::vector<std::shared_ptr<Service>> services;
        std::vector<std::shared_ptr<Command>> commands;

        TgBot::InlineKeyboardMarkup::Ptr keyboard;

        TgBot::InlineKeyboardButton::Ptr addButton(const std::string text, const std::string callback, std::vector<TgBot::InlineKeyboardButton::Ptr>& vec);
        void sendContent(const send_t& send, std::int64_t user_id, std::shared_ptr<Service> service);
        void mainloop();
        void update_services();
};