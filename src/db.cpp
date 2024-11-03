#include <db.hpp>

DB::DB(std::string name) : db(name, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE) {
    try {
        db.exec("CREATE TABLE IF NOT EXISTS User (id INTEGER PRIMARY KEY);");
        db.exec("CREATE TABLE IF NOT EXISTS Tags (tag_id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, site_name TEXT, tag TEXT, history TEXT, UNIQUE(user_id, site_name, tag), FOREIGN KEY (user_id) REFERENCES User(id));");
        db.exec("CREATE TABLE IF NOT EXISTS AntiTags (antitag_id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, site_name TEXT, antitag TEXT, UNIQUE(user_id, site_name, antitag), FOREIGN KEY (user_id) REFERENCES User(id));");
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::addUser(std::int64_t id, const std::vector<std::shared_ptr<Service>>& services) {
    try {
        db.exec(fmt::format("INSERT OR IGNORE INTO User (id) VALUES ({});", id));
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::addTag(std::shared_ptr<Service> service, const std::string &tag, std::int64_t user_id) {
    try {
        std::string site_name = service->getService();
        
        std::string history = "";
        auto sss = service->parse(tag);
        if (!sss.empty())
            history = service->parse(tag).at(0).getContent().at(0);

        db.exec(fmt::format("INSERT OR IGNORE INTO Tags (user_id, site_name, tag, history) VALUES ({}, '{}', '{}', '{}');", user_id, site_name, tag, history));
    } catch (const std::exception& e) {
        LOG_ERROR("Add Tag: {}", e.what());
    }
}

void DB::addAntiTag(const std::string &site_name, const std::string &tag, std::int64_t user_id) {
    try {
        db.exec(fmt::format("INSERT OR IGNORE INTO AntiTags (user_id, site_name, antitag) VALUES ({}, '{}', '{}');", user_id, site_name, tag));
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::rmTag(const std::string &site_name, const std::string &tag, std::int64_t user_id) {
    try {
        db.exec(fmt::format("DELETE FROM Tags WHERE user_id = {} AND site_name = '{}' AND tag = '{}';", user_id, site_name, tag));
    } catch (const std::exception& e) {
        LOG_ERROR("RmTag: {}", e.what());
    }
}

void DB::rmAntiTag(const std::string &site_name, const std::string &tag, std::int64_t user_id) {
    try {
        db.exec(fmt::format("DELETE FROM AntiTags WHERE user_id = {} AND site_name = '{}' AND antitag = '{}';", user_id, site_name, tag));
    } catch (const std::exception& e) {
        LOG_ERROR("RmAntiTag: {}", e.what());
    }
}

void DB::updateHistory(const std::string &site_name, const std::vector<std::string> &data, std::int64_t user_id) {
    try {
        if (!data.empty())
            return;

        std::string history = data.at(0);
        db.exec(fmt::format("UPDATE Tags SET history = '{}' WHERE user_id = {} AND site_name = '{}';", history, user_id, site_name));
    } catch (const std::exception& e) {
        LOG_ERROR("Error updating history: {}", e.what());
    }
}

bool DB::userExist(std::int64_t user) {
    try {
        SQLite::Statement query(db, "SELECT COUNT(*) FROM User WHERE id = ?");
        query.bind(1, user);

        query.executeStep();
        int count = query.getColumn(0).getInt();
        return count > 0;
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
        return false;
    }
}

std::unordered_map<std::string, std::vector<std::int64_t>> DB::getUsersByTags(const std::string &site_name) {
    std::unordered_map<std::string, std::vector<std::int64_t>> tagToUsers;

    try {
        SQLite::Statement tagQuery(db,
            "SELECT Tags.tag, User.id "
            "FROM User "
            "JOIN Tags ON User.id = Tags.user_id "
            "WHERE Tags.site_name = ?"
        );
        tagQuery.bind(1, site_name);

        while (tagQuery.executeStep()) {
            std::string tag = tagQuery.getColumn(0).getText();
            std::int64_t user_id = tagQuery.getColumn(1).getInt64();

            tagToUsers[tag].push_back(user_id);
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Error fetching users by tags for site '{}': {}", site_name, e.what());
    }

    return tagToUsers;
}

std::string DB::getHistoryForUserSiteAndTag(std::int64_t user_id, const std::string& site_name, const std::string& tag) {
    std::string history;

    try {
        SQLite::Statement query(db, 
            "SELECT history "
            "FROM Tags "
            "WHERE user_id = ? AND site_name = ? AND tag = ?"
        );
        
        query.bind(1, user_id);
        query.bind(2, site_name);
        query.bind(3, tag);
        
        if (query.executeStep()) {
            history = query.getColumn(0).getString();
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error retrieving history: {}", e.what());
    }
    
    return history;
}


std::vector<std::string> DB::getAntiTagForUserAndSite(std::int64_t user_id, const std::string& site_name) {
    std::vector<std::string> antitag_entries;
    
    try {
        SQLite::Statement query(db, 
            "SELECT AntiTags.antitag "
            "FROM AntiTags "
            "WHERE AntiTags.user_id = ? AND AntiTags.site_name = ?"
        );
        
        query.bind(1, user_id);
        query.bind(2, site_name);
        
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
        SQLite::Statement siteQuery(db, "SELECT DISTINCT site_name FROM Tags WHERE user_id = ?");
        siteQuery.bind(1, user_id);

        while (siteQuery.executeStep()) {
            std::string site_name = siteQuery.getColumn(0).getText();

            std::vector<std::string> tags;
            SQLite::Statement tagQuery(db, "SELECT tag FROM Tags WHERE user_id = ? AND site_name = ?");
            tagQuery.bind(1, user_id);
            tagQuery.bind(2, site_name);
            
            while (tagQuery.executeStep()) {
                tags.push_back(tagQuery.getColumn(0).getText());
            }

            std::vector<std::string> antiTags;
            SQLite::Statement antiTagQuery(db, "SELECT antitag FROM AntiTags WHERE user_id = ? AND site_name = ?");
            antiTagQuery.bind(1, user_id);
            antiTagQuery.bind(2, site_name);
            
            while (antiTagQuery.executeStep()) {
                antiTags.push_back(antiTagQuery.getColumn(0).getText());
            }

            result << "-----" << site_name << " Tags-----\n";
            for (const auto &tag : tags) {
                result << tag << "\n";
            }

            result << "-----" << site_name << " Anti-tags-----\n";
            for (const auto &antiTag : antiTags) {
                result << antiTag << "\n";
            }

            result << "\n";
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Error fetching tags and anti-tags: {}", e.what());
    }

    return result.str();
}