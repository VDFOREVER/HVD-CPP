cmake_minimum_required(VERSION 3.10)

project(bot)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE include ${OPENSSL_INCLUDE_DIR})

add_subdirectory(subprojects/cpr)
add_subdirectory(subprojects/pugixml)
add_subdirectory(subprojects/json)
add_subdirectory(subprojects/sqlitecpp)
add_subdirectory(subprojects/tgbot)

target_link_libraries(${PROJECT_NAME} cpr pugixml nlohmann_json SQLiteCpp TgBot ${OPENSSL_LIBRARIES})