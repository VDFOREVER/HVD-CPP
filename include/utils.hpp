#pragma once

#include <string>
#include <vector>
#include <services/service.hpp>
#include <openssl/evp.h>
#include <random>
#include <stdexcept>
#include <log.hpp>

class Utils {
    public:
        static std::vector<std::string> split(const std::string& str, char delimiter);
        template <typename T>
        static bool contains(const std::vector<T>& vec, const T& value) {
            return std::find(vec.begin(), vec.end(), value) != vec.end();
        }
        template <typename T, typename V>
        static bool contains(const std::vector<T>& v1, const std::vector<V>& v2) {
            for (const auto& element : v1) {
                if (std::find(v2.begin(), v2.end(), element) != v2.end())
                    return true;
            }
            return false;
        }
        static std::vector<std::uint8_t> sha256(const std::string& input);
        static std::string urlsafe_b64encode(const std::vector<std::uint8_t>& hash);
        static std::string generate_urlsafe_token(std::size_t length);
        static std::string getMimeType(const std::filesystem::path& path);
};