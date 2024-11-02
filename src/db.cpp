#include <db.hpp>

DB::DB(std::string name) : db(name, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE) {
    try {
        db.exec("CREATE TABLE IF NOT EXISTS User (id INTEGER PRIMARY KEY);");
        db.exec("CREATE TABLE IF NOT EXISTS Site (site_id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, name TEXT, UNIQUE(user_id, name), FOREIGN KEY (user_id) REFERENCES User(id));");
        db.exec("CREATE TABLE IF NOT EXISTS Tags (tag_id INTEGER PRIMARY KEY AUTOINCREMENT, site_id INTEGER, tag TEXT, UNIQUE(site_id, tag), FOREIGN KEY (site_id) REFERENCES Site(site_id));");
        db.exec("CREATE TABLE IF NOT EXISTS AntiTags (antitag_id INTEGER PRIMARY KEY AUTOINCREMENT,site_id INTEGER, antitag TEXT, UNIQUE(site_id, antitag),FOREIGN KEY (site_id) REFERENCES Site(site_id));");
        db.exec("CREATE TABLE IF NOT EXISTS History (history_id INTEGER PRIMARY KEY AUTOINCREMENT,site_id INTEGER, entry TEXT, UNIQUE(site_id, entry), FOREIGN KEY (site_id) REFERENCES Site(site_id));");
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::addUser(std::int64_t id, const std::vector<std::shared_ptr<Service>>& services) {
    try {
        db.exec(fmt::format("INSERT OR IGNORE INTO User (id) VALUES ({});", id));
        for(const auto& service: services) {
            db.exec(fmt::format("INSERT OR IGNORE INTO Site (user_id, name) VALUES ({}, '{}');", id, service->getService()));
        }

    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::addTag(std::shared_ptr<Service> service, const std::string &tag, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "SELECT site_id FROM Site WHERE user_id = ? AND name = ?");
        query.bind(1, user_id);
        query.bind(2, service->getService());

        if (query.executeStep()) {
            int site_id = query.getColumn(0).getInt();

            db.exec(fmt::format("INSERT OR IGNORE INTO Tags (site_id, tag) VALUES ({}, '{}');", site_id, tag));

            std::vector<std::string> posts;
            for (const auto& post : service->parse(tag)) {
                auto content = post.getContent();
                std::copy(content.begin(), content.end(), std::back_inserter(posts));
            }

            if (!posts.empty())
                addHistory(service->getService(), posts, user_id);
        } else {
            LOG_ERROR("Site '{}' not found for user {}", service->getService(), user_id);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Add Tag: {}", e.what());
    }
}

void DB::addAntiTag(const std::string &site_name, const std::string &tag, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "SELECT site_id FROM Site WHERE user_id = ? AND name = ?");
        query.bind(1, user_id);
        query.bind(2, site_name);

        if (query.executeStep()) {
            int site_id = query.getColumn(0).getInt();

            db.exec(fmt::format("INSERT OR IGNORE INTO AntiTags (site_id, antitag) VALUES ({}, '{}');", site_id, tag));
        } else {
            LOG_ERROR("Site '{}' not found for user {}", site_name, user_id);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
    }
}

void DB::rmTag(const std::string &site_name, const std::string &tag, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "SELECT site_id FROM Site WHERE user_id = ? AND name = ?");
        query.bind(1, user_id);
        query.bind(2, site_name);

        if (query.executeStep()) {
            int site_id = query.getColumn(0).getInt();

            db.exec(fmt::format("DELETE FROM Tags WHERE site_id = {} AND tag = '{}';", site_id, tag));
        } else {
            LOG_ERROR("Site '{}' not found for user {}", site_name, user_id);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("RmTag: {}", e.what());
    }
}

void DB::rmAntiTag(const std::string &site_name, const std::string &tag, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "SELECT site_id FROM Site WHERE user_id = ? AND name = ?");
        query.bind(1, user_id);
        query.bind(2, site_name);

        if (query.executeStep()) {
            int site_id = query.getColumn(0).getInt();

            db.exec(fmt::format("DELETE FROM AntiTags WHERE site_id = {} AND antitag = '{}';", site_id, tag));
        } else {
            LOG_ERROR("Site '{}' not found for user {}", site_name, user_id);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("RmAntiTag: {}", e.what());
    }
}

void DB::addHistory(const std::string &site_name, const std::vector<std::string> &data, std::int64_t user_id) {
    try {
        SQLite::Statement query(db, "SELECT site_id FROM Site WHERE user_id = ? AND name = ?");
        query.bind(1, user_id);
        query.bind(2, site_name);

        if (query.executeStep()) {
            int site_id = query.getColumn(0).getInt();

            for (const auto &entry : data) {
                db.exec(fmt::format("INSERT OR IGNORE INTO History (site_id, entry) VALUES ({}, '{}');", site_id, entry));
            }
        } else {
            LOG_ERROR("Site '{}' not found for user {}", site_name, user_id);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error adding history: {}", e.what());
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
        SQLite::Statement siteQuery(db,
            "SELECT site_id FROM Site WHERE name = ?"
        );
        siteQuery.bind(1, site_name);

        if (!siteQuery.executeStep()) {
            LOG_ERROR("Site '{}' not found.", site_name);
            return tagToUsers;
        }

        std::int64_t site_id = siteQuery.getColumn(0).getInt64();

        SQLite::Statement tagQuery(db,
            "SELECT Tags.tag, User.id "
            "FROM User "
            "JOIN Site ON User.id = Site.user_id "
            "JOIN Tags ON Site.site_id = Tags.site_id "
            "WHERE Site.site_id = ?"
        );
        tagQuery.bind(1, site_id);

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

std::vector<std::string> DB::getHistoryForUserAndSite(std::int64_t user_id, const std::string& site_name) {
    std::vector<std::string> history_entries;
    
    try {
        SQLite::Statement query(db, 
            "SELECT History.entry "
            "FROM History "
            "JOIN Site ON History.site_id = Site.site_id "
            "WHERE Site.user_id = ? AND Site.name = ?;"
        );
        
        query.bind(1, user_id);
        query.bind(2, site_name);
        
        while (query.executeStep()) {
            history_entries.push_back(query.getColumn(0).getString());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error retrieving history: " << e.what() << std::endl;
    }
    
    return history_entries;
}

std::vector<std::string> DB::getAntiTagForUserAndSite(std::int64_t user_id, const std::string& site_name) {
    std::vector<std::string> antitag_entries;
    
    try {
        SQLite::Statement query(db, 
            "SELECT AntiTags.antitag "
            "FROM AntiTags "
            "JOIN Site ON AntiTags.site_id = Site.site_id "
            "WHERE Site.user_id = ? AND Site.name = ?;"
        );
        
        query.bind(1, user_id);
        query.bind(2, site_name);
        
        while (query.executeStep()) {
            antitag_entries.push_back(query.getColumn(0).getString());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error retrieving antitag: " << e.what() << std::endl;
    }
    
    return antitag_entries;
}
std::string DB::getFormattedTagsAndAntiTags(std::int64_t user_id) {
    std::stringstream result;
    
    try {
        SQLite::Statement siteQuery(db, "SELECT name FROM Site WHERE user_id = ?");
        siteQuery.bind(1, user_id);

        while (siteQuery.executeStep()) {
            std::string site_name = siteQuery.getColumn(0).getText();

            std::vector<std::string> tags;
            SQLite::Statement tagQuery(db, "SELECT tag FROM Tags WHERE site_id = (SELECT site_id FROM Site WHERE user_id = ? AND name = ?)");
            tagQuery.bind(1, user_id);
            tagQuery.bind(2, site_name);
            
            while (tagQuery.executeStep()) {
                tags.push_back(tagQuery.getColumn(0).getText());
            }

            std::vector<std::string> antiTags;
            SQLite::Statement antiTagQuery(db, "SELECT antitag FROM AntiTags WHERE site_id = (SELECT site_id FROM Site WHERE user_id = ? AND name = ?)");
            antiTagQuery.bind(1, user_id);
            antiTagQuery.bind(2, site_name);
            
            while (antiTagQuery.executeStep()) {
                antiTags.push_back(antiTagQuery.getColumn(0).getText());
            }

            result << "-----" << site_name << " Tag-----\n";
            for (const auto &tag : tags) {
                result << tag << "\n";
            }

            result << "-----" << site_name << " Anti tag-----\n";
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