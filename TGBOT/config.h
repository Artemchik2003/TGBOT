#ifndef CONFIG_H
#define CONFIG_H
#include <algorithm>
#include <cctype>
#include <locale>

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <map>
#include <ctime>
#include <tgbot/tgbot.h>
// Функція для завантаження змінних з .env файлу
void loadEnvFile(const std::string& fileName);

// Функція для отримання токенів
std::string getEnvVariable(const std::string& varName);
// Функція повернення тексту користувача
std::string extractCommandArgument(const std::string& messageText);
// Функція для перетворення номера дня у назву дня тижня
std::string getDayOfWeek(int weekDay);
// Функція для отримання та відправлення розкладу
std::string generateMode();
std::string generateModetimetable(const std::string& groupName);


// Функція для видалення пробілів на початку та в кінці рядка
std::string trimWhitespace(const std::string& str);
#endif // CONFIG_H
