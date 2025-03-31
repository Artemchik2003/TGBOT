#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <nlohmann/json.hpp>
#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>

// Функція для запису даних у базу
void saveGroupsToDatabase(const nlohmann::json& jsonResponse);
// Функція для запису cтатусу
void updateUserStatusInDatabase(int64_t userId, const std::string& status);
// Функція для запису cтатусу Active
void setUserStatusToActive(int64_t userId, const std::string& username);
//function update on group in database
void updateUserGroupInDatabase(int64_t userId, const std::string& groupName, int groupId);

int getGroupIdByName(const std::string& groupName);
//Функція для запису даних у базу

void savetimetableToDatabaseFull(const nlohmann::json& jsonResponse, const std::string& groupName);
#endif // DATABASE_H