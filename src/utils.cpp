#include <utils.hpp>

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(str);

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::shared_ptr<Service> Utils::findServiceByName(const std::vector<std::shared_ptr<Service>>& services, const std::string& serviceName) {
    for (const auto& service : services) {
        if (service->getService() == serviceName) {
            return service;
        }
    }

    return nullptr;
}

std::vector<uint8_t> Utils::sha256(const std::string& input) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create context");
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize digest");
    }

    if (EVP_DigestUpdate(ctx, input.c_str(), input.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to update digest");
    }

    std::vector<uint8_t> hash(EVP_MD_size(EVP_sha256()));
    unsigned int length = 0;
    if (EVP_DigestFinal_ex(ctx, hash.data(), &length) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize digest");
    }

    EVP_MD_CTX_free(ctx);

    return hash;
}


std::string Utils::urlsafe_b64encode(const std::vector<std::uint8_t>& hash) {
    const std::size_t encoded_size = 4 * ((hash.size() + 2) / 3);

    std::vector<char> encoded(encoded_size + 1);
    EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encoded.data()), hash.data(), hash.size());

    std::string encoded_str(encoded.data());
    for (auto& ch : encoded_str) {
        if (ch == '+') ch = '-';
        else if (ch == '/') ch = '_';
    }

    encoded_str.erase(encoded_str.find_last_not_of('=') + 1);

    return encoded_str;
}

std::string Utils::generate_urlsafe_token(std::size_t length) {
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789-_";

    std::string token;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> dist(0, sizeof(alphanum) - 2);

    for (std::size_t i = 0; i < length; ++i) {
        token += alphanum[dist(generator)];
    }

    return token;
}