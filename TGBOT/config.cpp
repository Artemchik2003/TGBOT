#include "config.h"

std::map<std::string, std::string> envVariables;

void loadEnvFile(const std::string& fileName) {
    std::ifstream envFile(fileName);
    if (!envFile.is_open()) {
        std::cerr << "Failed to open file .env" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(envFile, line)) {
        size_t delimPos = line.find('=');
        if (delimPos != std::string::npos) {
            std::string key = line.substr(0, delimPos);
            std::string value = line.substr(delimPos + 1);
            envVariables[key] = value;
        }
    }
}

std::string getEnvVariable(const std::string& varName) {
    if (envVariables.find(varName) != envVariables.end()) {
        return envVariables[varName];
    }
    else {
        std::cerr << "Changeable " << varName << " not found in the .env file" << std::endl;
        return "";
    }
}
std::string extractCommandArgument(const std::string& messageText) {
    // Знайти перший пробіл після команди
    size_t spacePos = messageText.find(' ');
    if (spacePos != std::string::npos) {
        // Отримуємо аргумент після першого пробілу
        std::string argument = messageText.substr(spacePos + 1);

        // Видаляємо всі пробіли з аргументу
        argument.erase(remove(argument.begin(), argument.end(), ' '), argument.end());

        return argument;
    }
    // Якщо немає пробілу, повертаємо порожній рядок
    return "";
}

// Функція для формування параметра "mode" time
std::string generateMode() {
    // Отримання поточної дати
    time_t now = time(0);
    tm ltm = {};  // створюємо змінну для збереження часу
    localtime_s(&ltm, &now);  // використовуємо localtime_s для безпеки

    int day = ltm.tm_mday;
    int month = ltm.tm_mon + 1; // tm_mon починається з 0, тому додаємо 1
    int year = 1900 + ltm.tm_year;

    // Визначення навчального року на основі дати
    std::string academicYear;
    if (month > 7 || (month == 7 && day > 15)) {
        academicYear = std::to_string(year) + "/" + std::to_string(year + 1);
    }
    else {
        academicYear = std::to_string(year - 1) + "/" + std::to_string(year);
    }

    // Визначення семестру: осінній семестр - 1, весняний - 2
    int semester = (month >= 7 && month <= 12) ? 1 : 2;

    // Формування дати в форматі dd.mm.yyyy
    std::string formattedDate = (day < 10 ? "0" : "") + std::to_string(day) + "." +
        (month < 10 ? "0" : "") + std::to_string(month) + "." +
        std::to_string(year);

    // Формування рядка "mode"
    std::string mode = "getNumWeekDay&year=" + academicYear + "&semestr=" + std::to_string(semester) + "&date=" + formattedDate;

    return mode;
}
// Функція для формування параметра "mode" timetablesemester
std::string generateModetimetable(const std::string& groupName) {
    // Отримання поточної дати
    time_t now = time(0);
    tm ltm = {};
    localtime_s(&ltm, &now);  // Використовуємо localtime_s для безпеки

    int year = 1900 + ltm.tm_year; // Поточний рік
    int month = ltm.tm_mon + 1;    // Місяць (tm_mon починається з 0, тому додаємо 1)

    // Визначення навчального року на основі дати
    std::string academicYear;
    if (month > 7 || (month == 7 && ltm.tm_mday > 15)) {
        academicYear = std::to_string(year) + "/" + std::to_string(year + 1);
    }
    else {
        academicYear = std::to_string(year - 1) + "/" + std::to_string(year);
    }

    // Визначення семестру: осінній семестр - 1, весняний - 2
    int semester = (month >= 7 && month <= 12) ? 1 : 2;
   
    // Формування рядка "mode" з використанням groupName
    std::string mode = "getData&status=s&idGroup=" + groupName + "&year=" + 
        academicYear + "&semestr=" + std::to_string(semester);
    return mode;
}
// Функція для видалення пробілів на початку та в кінці рядка
std::string trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}