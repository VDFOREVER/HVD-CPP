#pragma once

#include <format>
#include <iostream>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BOLDRED "\033[1m\033[31m"

#define LOG_INFO(...)       std::cout << GREEN << "[INFO] " << RESET << std::format(__VA_ARGS__) << std::endl;
#define LOG_WARN(...)       std::cout << YELLOW << "[WARN] " << RESET << std::format(__VA_ARGS__) << std::endl;
#define LOG_ERROR(...)      std::cerr << RED << "[ERROR] " << RESET << std::format(__VA_ARGS__) << std::endl;
#define LOG_CRITICAL(...)   std::cerr << BOLDRED << "[CRITICAL] " << RESET << std::format(__VA_ARGS__) << std::endl;
#define LOG_FATAL(...)   std::cerr << BOLDRED << "[FATAL] " << RESET << std::format(__VA_ARGS__) << std::endl;