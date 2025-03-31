#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <nlohmann/json.hpp>
#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>

// ������� ��� ������ ����� � ����
void saveGroupsToDatabase(const nlohmann::json& jsonResponse);
// ������� ��� ������ c������
void updateUserStatusInDatabase(int64_t userId, const std::string& status);
// ������� ��� ������ c������ Active
void setUserStatusToActive(int64_t userId, const std::string& username);
//function update on group in database
void updateUserGroupInDatabase(int64_t userId, const std::string& groupName, int groupId);

int getGroupIdByName(const std::string& groupName);
//������� ��� ������ ����� � ����

void savetimetableToDatabaseFull(const nlohmann::json& jsonResponse, const std::string& groupName);
#endif // DATABASE_H