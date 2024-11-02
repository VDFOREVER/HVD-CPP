#pragma once

#include <string>
#include <vector>
#include <cpr/cpr.h>
#include <cpr/cookies.h>
#include <services/service.hpp>
#include <openssl/evp.h>
#include <random>
#include <stdexcept>

class Utils {
    public:
        template<typename... Args>
        static std::string request(const std::string& url, Args&&... args) {
            cpr::Response r = cpr::Get(cpr::Url{url}, std::forward<Args>(args)...);
            
            if (r.status_code != 200)
                return "";
                
            return r.text;
        }

        static std::vector<std::string> split(const std::string& str, char delimiter);
        static std::shared_ptr<Service> findServiceByName(const std::vector<std::shared_ptr<Service>>& services, const std::string& serviceName);
        template <typename T>
        static bool contains(const std::vector<T>& vec, const T& value) {
            return std::find(vec.begin(), vec.end(), value) != vec.end();
        }
        static std::vector<std::uint8_t> sha256(const std::string& input);
        static std::string urlsafe_b64encode(const std::vector<std::uint8_t>& hash);
        static std::string generate_urlsafe_token(std::size_t length);
};