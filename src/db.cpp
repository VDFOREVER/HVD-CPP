#include <db.hpp>

DB::DB(std::string name) : db(name, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE) {
    try {
        db.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, is_admin BOOL);");
        db.exec("CREATE TABLE IF NOT EXISTS tags (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, site TEXT, tag TEXT, UNIQUE(user_id, site, tag), FOREIGN KEY (user_id) REFERENCES User(id));");
        db.exec("CREATE TABLE IF NOT EXISTS anti_tags (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, site TEXT, tag TEXT, UNIQUE(user_id, site, tag), FOREIGN KEY (user_id) REFERENCES User(id));");
        db.exec("CREATE TABLE IF NOT EXISTS history (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, site TEXT, history TEXT, UNIQUE(user_id, site, history), FOREIGN KEY (user_id) REFERENCES User(id));");
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::addUser(std::int64_t id, bool is_admin) {
    try {
        SQLite::Statement query(db, "INSERT OR IGNORE INTO users (id, is_admin) VALUES (?, ?);");
        query.bind(1, id);
        query.bind(2, is_admin);
        query.exec();

    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::rmUser(std::int64_t id) {
    try {
        SQLite::Statement tagsQuery(db, "DELETE FROM tags WHERE user_id = ?;");
        tagsQuery.bind(1, id);
        tagsQuery.exec();

        SQLite::Statement antiTagsQuery(db, "DELETE FROM anti_tags WHERE user_id = ?;");
        antiTagsQuery.bind(1, id);
        antiTagsQuery.exec();

        SQLite::Statement userQuery(db, "DELETE FROM users WHERE id = ?;");
        userQuery.bind(1, id);
        userQuery.exec();
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to remove users with id {}: {}", id, e.what());
    }
}

void DB::addTag(std::shared_ptr<Service> service, const std::string &tag, std::int64_t user_id) {
    try {
        std::string site = service->type;

        for (const auto& post : service->parse(tag))
            addHistory(site, post.id, user_id);

        SQLite::Statement query(db, "INSERT OR IGNORE INTO tags (user_id, site, tag) VALUES (?, ?, ?);");
        query.bind(1, user_id);
        query.bind(2, site);
        query.bind(3, tag);

        query.exec();
    } catch (const std::exception& e) {
        LOG_ERROR("Add Tag: {}", e.what());
    }
}

void DB::addAntiTag(const std::string &site, const std::string &tag, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "INSERT OR IGNORE INTO anti_tags (user_id, site, tag) VALUES (?, ?, ?);");
        query.bind(1, user_id);
        query.bind(2, site);
        query.bind(3, tag);
        query.exec();
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::rmTag(const std::string &site, const std::string &tag, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "DELETE FROM tags WHERE user_id = ? AND site = ? AND tag = ?;");
        query.bind(1, user_id);
        query.bind(2, site);
        query.bind(3, tag);
        query.exec();
    } catch (const std::exception& e) {
        LOG_ERROR("RmTag: {}", e.what());
    }
}

void DB::rmAntiTag(const std::string &site, const std::string &tag, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "DELETE FROM anti_tags WHERE user_id = ? AND site = ? AND tag = ?;");
        query.bind(1, user_id);
        query.bind(2, site);
        query.bind(3, tag);
        query.exec();
    } catch (const std::exception& e) {
        LOG_ERROR("RmAntiTag: {}", e.what());
    }
}

void DB::addHistory(const std::string &site, const std::string &data, std::int64_t user_id) {
    if (data.empty())
        return;

    try {
        SQLite::Statement query(db, "INSERT OR IGNORE INTO history (history, user_id, site) VALUES (?, ?, ?);");
        query.bind(1, data);
        query.bind(2, user_id);
        query.bind(3, site);
        query.exec();
    } catch (const std::exception& e) {
        LOG_ERROR("Error adding history: {}", e.what());
    }
}

bool DB::isUserTableEmpty() {
    try {
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users;");
        query.executeStep();
        return query.getColumn(0).getInt() == 0;
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
        return true;
    }
}

bool DB::userExist(std::int64_t user, bool is_admin) {
    try {
        SQLite::Statement query(db, "SELECT COUNT(*), is_admin FROM users WHERE id = ?");
        query.bind(1, user);

        query.executeStep();
        return query.getColumn(0).getInt() > 0 && (!is_admin || query.getColumn(1).getInt() == 1);
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
        return false;
    }
}

std::unordered_map<std::string, std::vector<std::int64_t>> DB::getUsersByTags(const std::string &site) {
    std::unordered_map<std::string, std::vector<std::int64_t>> tagToUsers;

    try {
        SQLite::Statement tagQuery(db,
            "SELECT tags.tag, users.id "
            "FROM users "
            "JOIN tags ON users.id = tags.user_id "
            "WHERE tags.site = ?"
        );
        tagQuery.bind(1, site);

        while (tagQuery.executeStep()) {
            std::string tag = tagQuery.getColumn(0).getText();
            std::int64_t user_id = tagQuery.getColumn(1).getInt64();

            tagToUsers[tag].push_back(user_id);
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Error fetching users by tags for site '{}': {}", site, e.what());
    }

    return tagToUsers;
}

std::vector<std::string> DB::getHistory(std::int64_t user_id, const std::string& site, const std::string& tag) {
    std::vector<std::string> history_entries;

    try {
        SQLite::Statement query(db,
            "SELECT history.history "
            "FROM history "
            "WHERE history.user_id = ? AND history.site = ?"
        );

        query.bind(1, user_id);
        query.bind(2, site);

        while (query.executeStep()) {
            history_entries.push_back(query.getColumn(0).getString());
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error retrieving history: {}", e.what());
    }

    return history_entries;
}


std::vector<std::string> DB::getAntiTagForUserAndSite(std::int64_t user_id, const std::string& site) {
    std::vector<std::string> antitag_entries;

    try {
        SQLite::Statement query(db,
            "SELECT anti_tags.tag "
            "FROM anti_tags "
            "WHERE anti_tags.user_id = ? AND anti_tags.site = ?"
        );

        query.bind(1, user_id);
        query.bind(2, site);

        while (query.executeStep()) {
            antitag_entries.push_back(query.getColumn(0).getString());
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error retrieving antitag: {}", e.what());
    }

    return antitag_entries;
}

std::string DB::getFormattedTagsAndAntiTags(std::int64_t user_id) {
    std::stringstream result;

    try {
        SQLite::Statement siteQuery(db, "SELECT DISTINCT site FROM tags WHERE user_id = ?");
        siteQuery.bind(1, user_id);

        while (siteQuery.executeStep()) {
            std::string site = siteQuery.getColumn(0).getText();

            std::vector<std::string> tags;
            SQLite::Statement tagQuery(db, "SELECT tag FROM tags WHERE user_id = ? AND site = ?");
            tagQuery.bind(1, user_id);
            tagQuery.bind(2, site);

            while (tagQuery.executeStep()) {
                tags.push_back(tagQuery.getColumn(0).getText());
            }

            std::vector<std::string> antiTags;
            SQLite::Statement antiTagQuery(db, "SELECT tag FROM anti_tags WHERE user_id = ? AND site = ?");
            antiTagQuery.bind(1, user_id);
            antiTagQuery.bind(2, site);

            while (antiTagQuery.executeStep()) {
                antiTags.push_back(antiTagQuery.getColumn(0).getText());
            }

            result << "-----" << site << " Tags-----\n";
            for (const auto &tag : tags) {
                result << tag << "\n";
            }

            result << "-----" << site << " Anti-tags-----\n";
            for (const auto &antiTag : antiTags) {
                result << antiTag << "\n";
            }

            result << "\n";
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Error fetching tags and anti-tags: {}", e.what());
    }

    std::string res = result.str();

    return res.empty() ? "Empty" : res;
}