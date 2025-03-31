#define SQLITECPP_ENABLE_ASSERT_HANDLER
#include "database.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>

// ϳ��������� �� ���� �����
extern SQLite::Database db;
// ������ ���� ��� ��������
extern SQLite::Database timetableDb;
void savetimetableToDatabaseFull(const nlohmann::json& jsonResponse, const std::string& groupName) {
    try {
        // ����������, �� ������ JSON ���� "data" � �� ���� �� ������
        if (jsonResponse.contains("data") && !jsonResponse["data"].empty()) {
            std::cout << "Parsing timetable JSON is successful." << std::endl;

            // ��������� ����� ������� `schedule`, ���� ���� �� ����
            std::string createTableQuery =
                "CREATE TABLE IF NOT EXISTS [" + groupName + "] ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "groupName TEXT NOT NULL, " // ����� �����
                "lesson TEXT, "
                "dayWeek TEXT, "
                "dayN TEXT, "
                "num TEXT, "
                "chet TEXT, "
                "numWeek TEXT, "
                "type TEXT, "
                "pp TEXT, "
                "room TEXT, "
                "dateL TEXT"
                ");";

           
            timetableDb.exec(createTableQuery);
            std::cout << "Table [" << groupName << "] created in timetable.db" << std::endl;

            // ���������� �� ������� ������
            for (const auto& item : jsonResponse["data"]) {
                std::string lesson = item.value("lesson", "");
                std::string dayWeek = item.value("dayWeek", "");
                std::string dayN = item.value("dayN", "");
                std::string num = item.value("num", "");
                std::string chet = item.value("chet", "");
                std::string numWeek = item.value("numWeek", "");
                std::string type = item.value("type", "");
                std::string pp = item.value("pp", "-");
                std::string room = item.value("room", "");
                std::string dateL = item.value("dateL", "");

                try {
                    // ����������, �� ���� ����� ����� � ���
                    SQLite::Statement checkQuery(timetableDb,
                        "SELECT COUNT(*) FROM [" + groupName + "] WHERE "
                        "groupName = ? AND lesson = ? AND dayWeek = ? AND dayN = ? AND "
                        "num = ? AND chet = ? AND numWeek = ? AND type = ? AND pp = ? AND "
                        "room = ? AND dateL = ?;");

                    checkQuery.bind(1, groupName);
                    checkQuery.bind(2, lesson);
                    checkQuery.bind(3, dayWeek);
                    checkQuery.bind(4, dayN);
                    checkQuery.bind(5, num);
                    checkQuery.bind(6, chet);
                    checkQuery.bind(7, numWeek);
                    checkQuery.bind(8, type);
                    checkQuery.bind(9, pp);
                    checkQuery.bind(10, room);
                    checkQuery.bind(11, dateL);

                    if (checkQuery.executeStep() && checkQuery.getColumn(0).getInt() == 0) {
                        // ���� ������ ���� � ������ ����
                        SQLite::Statement insertQuery(timetableDb,
                            "INSERT INTO [" + groupName + "] (groupName, lesson, dayWeek, dayN, num, chet, numWeek, type, pp, room, dateL) "
                            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

                        insertQuery.bind(1, groupName);
                        insertQuery.bind(2, lesson);
                        insertQuery.bind(3, dayWeek);
                        insertQuery.bind(4, dayN);
                        insertQuery.bind(5, num);
                        insertQuery.bind(6, chet);
                        insertQuery.bind(7, numWeek);
                        insertQuery.bind(8, type);
                        insertQuery.bind(9, pp);
                        insertQuery.bind(10, room);
                        insertQuery.bind(11, dateL);

                        insertQuery.exec();
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error while checking or inserting record: " << e.what() << std::endl;
                }
            }

            std::cout << "Timetable data successfully saved into the table [" << groupName << "]." << std::endl;
        }
        else {
            std::cerr << "Error: 'data' field is missing or empty in the JSON response." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error while saving timetable data for group [" << groupName << "]: " << e.what() << std::endl;
    }
}

void saveGroupsToDatabase(const nlohmann::json& jsonResponse) {
    try {
        // ����������, �� ������ ������� ���� "data"
        if (jsonResponse.contains("data")) {
            std::cout << "Parsing JSON good" << std::endl;

            // ϳ�������� SQL-������ ��� ������� ����� � ����
            SQLite::Statement insertQuery(db, "INSERT OR IGNORE INTO codeGroup (ID, GroupName) VALUES (?, ?)");

            for (const auto& item : jsonResponse["data"]) {
                std::string idFromJson = item["id"].get<std::string>();
                std::string groupFromJson = item["group"].get<std::string>();

                // ���: �������� ����� �� ID
                std::cout << "We save the group: " << groupFromJson << " ID: " << idFromJson << std::endl;

                try {
                    // ����������, �� ��� ���� ����� � ����� ID
                    SQLite::Statement query(db, "SELECT COUNT(*) FROM codeGroup WHERE id = :groupid");
                    query.bind(":groupid", idFromJson);

                    if (query.executeStep()) {
                        int count1 = query.getColumn(0).getInt();
                        if (count1 > 0) {
                            std::cout << "Group ID " << idFromJson << " found" << std::endl;
                        }
                        else {
                            std::cout << "Group ID " << idFromJson << " not found" << std::endl;
                            // ������ ����� � ����
                            insertQuery.bind(1, idFromJson);
                            insertQuery.bind(2, groupFromJson);
                            insertQuery.exec();
                            insertQuery.reset();  // ������ ����� ��� �������� �����
                        }
                    }
                }
                catch (std::exception& e) {
                    std::cerr << "������� �� ��� ���������� �����: " << e.what() << std::endl;
                }
            }     
        }
        else {
            std::cerr << "�������: ���� 'data' ������ � ������." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "�������: " << e.what() << std::endl;
    }
}
void updateUserStatusInDatabase(int64_t userId, const std::string& status) {
    try {
        // SQL-����� ��� ��������� ������� ����������� �� ����� idTG
        SQLite::Statement query(db, "UPDATE users SET Status = ? WHERE idTG = ?");
        query.bind(1, status);       // ����'����� ����� ������ (Blocked/Active)
        query.bind(2, userId);       // ����'����� ID ����������� Telegram (idTG)

        query.exec(); // ��������� ������
        std::cout << "Status updated to " << status << " for user ID: " << userId << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to update status: " << e.what() << std::endl;
    }
}
void setUserStatusToActive(int64_t userId, const std::string& username) {
    try {
        // ������ ����������, �� ���� ���������� � ��� �����
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE idTG = ?");
        query.bind(1, userId);

        int userCount = 0;
        if (query.executeStep()) {
            userCount = query.getColumn(0).getInt();
        }

        if (userCount > 0) {
            std::cout << "ID " << userId << " found" << std::endl;
            // ���������� ���� - ��������� ������ �� "Active"
            SQLite::Statement updateQuery(db, "UPDATE users SET Status = 'Active' WHERE idTG = ?");
            updateQuery.bind(1, userId);
            updateQuery.exec();
            std::cout << "Status set to 'Active' for existing user ID: " << userId << std::endl;
        }
        else {
            // ����������� ���� - ������ ����� ����� � ���������� �������� "Active"
            SQLite::Statement insertQuery(db, "INSERT INTO users (idTG, username, Status) VALUES (?, ?, 'Active')");
            insertQuery.bind(1, userId);
            insertQuery.bind(2, username);
            insertQuery.exec();
            std::cout << "New user added with ID: " << userId << " and status 'Active'" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to update or insert user status: " << e.what() << std::endl;
    }
}
void updateUserGroupInDatabase(int64_t userId, const std::string& groupName, int groupId) {
    try {
        // SQL-����� ��� ��������� ����� �� IDGroup ����������� �� ����� idTG
        SQLite::Statement query(db, "UPDATE users SET [Group] = ?, IDGroup = ? WHERE idTG = ?");
        query.bind(1, groupName);   // ����'����� ����� �����
        query.bind(2, groupId);     // ����'����� ID �����
        query.bind(3, userId);      // ����'����� ID ����������� Telegram (idTG)

        query.exec(); // ��������� ������
        std::cout << "Group updated to " << groupName << " (IDGroup: " << groupId << ") for user ID: " << userId << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to update group: " << e.what() << std::endl;
    }
}
int getGroupIdByName(const std::string& groupName) {
    try {
        // SQL-����� ��� ��������� ID ����� � ������� codeGroup
        SQLite::Statement query(db, "SELECT id FROM codeGroup WHERE GroupName = ?");
        query.bind(1, groupName);  // ����'����� ����� �����

        if (query.executeStep()) {
            // ���� ����� ��������, ��������� �� ID
            return query.getColumn(0).getInt();
        }
        else {
            // ���� ����� �� ��������, ��������� �������� -1
            return -1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "������� �� ��� ��������� ID �����: " << e.what() << std::endl;
        return -1;  // ������������� -1 � ������� �������
    }
}
