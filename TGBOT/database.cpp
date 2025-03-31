#define SQLITECPP_ENABLE_ASSERT_HANDLER
#include "database.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>

// Підключення до бази даних
extern SQLite::Database db;
// Окрема база для розкладів
extern SQLite::Database timetableDb;
void savetimetableToDatabaseFull(const nlohmann::json& jsonResponse, const std::string& groupName) {
    try {
        // Перевіряємо, чи містить JSON поле "data" і чи воно не порожнє
        if (jsonResponse.contains("data") && !jsonResponse["data"].empty()) {
            std::cout << "Parsing timetable JSON is successful." << std::endl;

            // Створюємо єдину таблицю `schedule`, якщо вона не існує
            std::string createTableQuery =
                "CREATE TABLE IF NOT EXISTS [" + groupName + "] ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "groupName TEXT NOT NULL, " // Назва групи
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

            // Перебираємо всі отримані записи
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
                    // Перевіряємо, чи існує такий запис у базі
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
                        // Якщо запису немає – додаємо його
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
        // Перевіряємо, чи містить відповідь поле "data"
        if (jsonResponse.contains("data")) {
            std::cout << "Parsing JSON good" << std::endl;

            // Підготовка SQL-запиту для вставки даних у базу
            SQLite::Statement insertQuery(db, "INSERT OR IGNORE INTO codeGroup (ID, GroupName) VALUES (?, ?)");

            for (const auto& item : jsonResponse["data"]) {
                std::string idFromJson = item["id"].get<std::string>();
                std::string groupFromJson = item["group"].get<std::string>();

                // Лог: зберігаємо групу та ID
                std::cout << "We save the group: " << groupFromJson << " ID: " << idFromJson << std::endl;

                try {
                    // Перевіряємо, чи вже існує запис з таким ID
                    SQLite::Statement query(db, "SELECT COUNT(*) FROM codeGroup WHERE id = :groupid");
                    query.bind(":groupid", idFromJson);

                    if (query.executeStep()) {
                        int count1 = query.getColumn(0).getInt();
                        if (count1 > 0) {
                            std::cout << "Group ID " << idFromJson << " found" << std::endl;
                        }
                        else {
                            std::cout << "Group ID " << idFromJson << " not found" << std::endl;
                            // Додаємо запис у базу
                            insertQuery.bind(1, idFromJson);
                            insertQuery.bind(2, groupFromJson);
                            insertQuery.exec();
                            insertQuery.reset();  // Готуємо запит для наступної групи
                        }
                    }
                }
                catch (std::exception& e) {
                    std::cerr << "Помилка під час збереження групи: " << e.what() << std::endl;
                }
            }     
        }
        else {
            std::cerr << "Помилка: поле 'data' відсутнє у відповіді." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << std::endl;
    }
}
void updateUserStatusInDatabase(int64_t userId, const std::string& status) {
    try {
        // SQL-запит для оновлення статусу користувача за полем idTG
        SQLite::Statement query(db, "UPDATE users SET Status = ? WHERE idTG = ?");
        query.bind(1, status);       // Прив'язуємо новий статус (Blocked/Active)
        query.bind(2, userId);       // Прив'язуємо ID користувача Telegram (idTG)

        query.exec(); // Виконання запиту
        std::cout << "Status updated to " << status << " for user ID: " << userId << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to update status: " << e.what() << std::endl;
    }
}
void setUserStatusToActive(int64_t userId, const std::string& username) {
    try {
        // Спершу перевіряємо, чи існує користувач у базі даних
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE idTG = ?");
        query.bind(1, userId);

        int userCount = 0;
        if (query.executeStep()) {
            userCount = query.getColumn(0).getInt();
        }

        if (userCount > 0) {
            std::cout << "ID " << userId << " found" << std::endl;
            // Користувач існує - оновлюємо статус на "Active"
            SQLite::Statement updateQuery(db, "UPDATE users SET Status = 'Active' WHERE idTG = ?");
            updateQuery.bind(1, userId);
            updateQuery.exec();
            std::cout << "Status set to 'Active' for existing user ID: " << userId << std::endl;
        }
        else {
            // Користувача немає - додаємо новий запис з початковим статусом "Active"
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
        // SQL-запит для оновлення групи та IDGroup користувача за полем idTG
        SQLite::Statement query(db, "UPDATE users SET [Group] = ?, IDGroup = ? WHERE idTG = ?");
        query.bind(1, groupName);   // Прив'язуємо назву групи
        query.bind(2, groupId);     // Прив'язуємо ID групи
        query.bind(3, userId);      // Прив'язуємо ID користувача Telegram (idTG)

        query.exec(); // Виконання запиту
        std::cout << "Group updated to " << groupName << " (IDGroup: " << groupId << ") for user ID: " << userId << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to update group: " << e.what() << std::endl;
    }
}
int getGroupIdByName(const std::string& groupName) {
    try {
        // SQL-запит для отримання ID групи з таблиці codeGroup
        SQLite::Statement query(db, "SELECT id FROM codeGroup WHERE GroupName = ?");
        query.bind(1, groupName);  // Прив'язуємо назву групи

        if (query.executeStep()) {
            // Якщо група знайдена, повертаємо її ID
            return query.getColumn(0).getInt();
        }
        else {
            // Якщо група не знайдена, повертаємо значення -1
            return -1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Помилка під час отримання ID групи: " << e.what() << std::endl;
        return -1;  // Повертатимемо -1 у випадку помилки
    }
}
