#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <vector>
#include <log.hpp>
#include <unordered_map>
#include <services/service.hpp>

class DB {
    public:
        DB(std::string name);
        void addTag(std::shared_ptr<Service> service, const std::string &tag, std::int64_t user_id);
        void addAntiTag(const std::string &site_name, const std::string &tag, std::int64_t user_id);
        void rmTag(const std::string &site_name, const std::string &tag, std::int64_t user_id);
        void rmAntiTag(const std::string &site_name, const std::string &tag, std::int64_t user_id);
        void addHistory(const std::string &site_name, const std::vector<std::string> &data, std::int64_t user_id);
        bool isUserTableEmpty();
        bool userExist(std::int64_t user, bool is_admin = false);
        std::unordered_map<std::string, std::vector<int64_t>> getUsersByTags(const std::string &site_name);
        std::vector<std::string> getHistory(std::int64_t user_id, const std::string& site_name, const std::string& tag);
        std::vector<std::string> getAntiTagForUserAndSite(std::int64_t user_id, const std::string& site_name);
        std::string getFormattedTagsAndAntiTags(std::int64_t user_id);
        void addUser(std::int64_t id, bool is_admin = false);
        void rmUser(std::int64_t id);

    private:
        SQLite::Database db;
};
