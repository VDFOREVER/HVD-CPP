#pragma once

#include <bot.hpp>

#define BOT_CMD(name) void cmd_##name(Bot& bot, TgBot::Message::Ptr message, std::vector<std::string> args)

BOT_CMD(help);
BOT_CMD(adduser);
BOT_CMD(deluser);
BOT_CMD(addtag);
BOT_CMD(deltag);
BOT_CMD(addantitag);
BOT_CMD(delantitag);
BOT_CMD(taglist);
BOT_CMD(scorelimit);