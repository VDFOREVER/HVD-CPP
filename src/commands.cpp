#include <commands.hpp>

BOT_CMD(help) {
    std::string help_str = bot.helpMessage();

    help_str += "\n\n";
    help_str += "Available services:\n";
    for (const auto& name : bot.availableServices())
        help_str += fmt::format(" - {}\n", name);

    bot.replyMessage(message, help_str.c_str());
}

BOT_CMD(adduser) {
    std::int64_t id = std::stoll(args[0]);
    if (bot.getDB().userExist(id)) {
        bot.replyMessage(message, "User `{}` already exists", id);
        return;
    }

    bot.getDB().addUser(id);
    bot.replyMessage(message, "User `{}` successfully added", id);
}

BOT_CMD(deluser) {
    std::int64_t id = std::stoll(args[0]);
    if (!bot.getDB().userExist(id)) {
        bot.replyMessage(message, "User `{}` don't exists", id);
        return;
    }

    bot.getDB().rmUser(id);
    bot.replyMessage(message, "User `{}` successfully removed", id);
}

BOT_CMD(addtag) {
    auto service = bot.findServiceByName(args[0]);
    if (service == nullptr) {
        bot.replyMessage(message, "Service `{}` not found", args[0]);
        return;
    }

    bot.getDB().addTag(service, args[1], message->chat->id);
    bot.replyMessage(message, "Tag `{}` added to service `{}`", args[1], args[0]);
}

BOT_CMD(deltag) {
    bot.getDB().rmTag(args[0], args[1], message->chat->id);
    bot.replyMessage(message, "Tag `{}` removed from service `{}`", args[1], args[0]);
}

BOT_CMD(addantitag) {
    auto service = bot.findServiceByName(args[0]);
    if (service == nullptr) {
        bot.replyMessage(message, "Service `{}` not found", args[0]);
        return;
    }

    bot.getDB().addAntiTag(args[0], args[1], message->chat->id);
    bot.replyMessage(message, "Anti-tag `{}` added to service `{}`", args[1], args[0]);
}

BOT_CMD(delantitag) {
    bot.getDB().rmAntiTag(args[0], args[1], message->chat->id);
    bot.replyMessage(message, "Anti-tag `{}` removed from service `{}`", args[1], args[0]);
}

BOT_CMD(taglist) {
    bot.replyMessage(message, bot.getDB().getFormattedTagsAndAntiTags(message->chat->id));
}