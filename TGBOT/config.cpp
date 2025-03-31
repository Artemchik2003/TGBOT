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
    // ������ ������ ����� ���� �������
    size_t spacePos = messageText.find(' ');
    if (spacePos != std::string::npos) {
        // �������� �������� ���� ������� ������
        std::string argument = messageText.substr(spacePos + 1);

        // ��������� �� ������ � ���������
        argument.erase(remove(argument.begin(), argument.end(), ' '), argument.end());

        return argument;
    }
    // ���� ���� ������, ��������� ������� �����
    return "";
}

// ������� ��� ���������� ��������� "mode" time
std::string generateMode() {
    // ��������� ������� ����
    time_t now = time(0);
    tm ltm = {};  // ��������� ����� ��� ���������� ����
    localtime_s(&ltm, &now);  // ������������� localtime_s ��� �������

    int day = ltm.tm_mday;
    int month = ltm.tm_mon + 1; // tm_mon ���������� � 0, ���� ������ 1
    int year = 1900 + ltm.tm_year;

    // ���������� ����������� ���� �� ����� ����
    std::string academicYear;
    if (month > 7 || (month == 7 && day > 15)) {
        academicYear = std::to_string(year) + "/" + std::to_string(year + 1);
    }
    else {
        academicYear = std::to_string(year - 1) + "/" + std::to_string(year);
    }

    // ���������� ��������: ����� ������� - 1, �������� - 2
    int semester = (month >= 7 && month <= 12) ? 1 : 2;

    // ���������� ���� � ������ dd.mm.yyyy
    std::string formattedDate = (day < 10 ? "0" : "") + std::to_string(day) + "." +
        (month < 10 ? "0" : "") + std::to_string(month) + "." +
        std::to_string(year);

    // ���������� ����� "mode"
    std::string mode = "getNumWeekDay&year=" + academicYear + "&semestr=" + std::to_string(semester) + "&date=" + formattedDate;

    return mode;
}
// ������� ��� ���������� ��������� "mode" timetablesemester
std::string generateModetimetable(const std::string& groupName) {
    // ��������� ������� ����
    time_t now = time(0);
    tm ltm = {};
    localtime_s(&ltm, &now);  // ������������� localtime_s ��� �������

    int year = 1900 + ltm.tm_year; // �������� ��
    int month = ltm.tm_mon + 1;    // ̳���� (tm_mon ���������� � 0, ���� ������ 1)

    // ���������� ����������� ���� �� ����� ����
    std::string academicYear;
    if (month > 7 || (month == 7 && ltm.tm_mday > 15)) {
        academicYear = std::to_string(year) + "/" + std::to_string(year + 1);
    }
    else {
        academicYear = std::to_string(year - 1) + "/" + std::to_string(year);
    }

    // ���������� ��������: ����� ������� - 1, �������� - 2
    int semester = (month >= 7 && month <= 12) ? 1 : 2;
   
    // ���������� ����� "mode" � ������������� groupName
    std::string mode = "getData&status=s&idGroup=" + groupName + "&year=" + 
        academicYear + "&semestr=" + std::to_string(semester);
    return mode;
}
// ������� ��� ��������� ������ �� ������� �� � ���� �����
std::string trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}