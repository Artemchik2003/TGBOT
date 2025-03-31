#include "http_request.h" //  POST and GET
#include "database.h"
#include "config.h"
#include <nlohmann/json.hpp>  // JSON бібліотека для парсингу

using json = nlohmann::json;
// Функція для перетворення номера дня у назву дня тижня
std::string getDayOfWeek(int weekDay) {
    switch (weekDay) {
    case 1: return "Понеділок";
    case 2: return "Вівторок";
    case 3: return "Середа";
    case 4: return "Четвер";
    case 5: return "П'ятниця";
    case 6: return "Субота";
    case 0: return "Неділя";
    default: return "Невідомий день";
    }
}
void sendScheduleReplyKeyboard(TgBot::Bot& bot, TgBot::Message::Ptr message) {
    TgBot::ReplyKeyboardMarkup::Ptr keyboard(new TgBot::ReplyKeyboardMarkup);
    keyboard->resizeKeyboard = true; // Клавіатура автоматично адаптується до кількості кнопок

    // Створення рядків з кнопками
    std::vector<TgBot::KeyboardButton::Ptr> row1;
    std::vector<TgBot::KeyboardButton::Ptr> row2;

    TgBot::KeyboardButton::Ptr buttonMonday(new TgBot::KeyboardButton);
    buttonMonday->text = "Понеділок";
    row1.push_back(buttonMonday);

    TgBot::KeyboardButton::Ptr buttonTuesday(new TgBot::KeyboardButton);
    buttonTuesday->text = "Вівторок";
    row1.push_back(buttonTuesday);

    TgBot::KeyboardButton::Ptr buttonWednesday(new TgBot::KeyboardButton);
    buttonWednesday->text = "Середа";
    row2.push_back(buttonWednesday);

    TgBot::KeyboardButton::Ptr buttonThursday(new TgBot::KeyboardButton);
    buttonThursday->text = "Четвер";
    row2.push_back(buttonThursday);

    TgBot::KeyboardButton::Ptr buttonFriday(new TgBot::KeyboardButton);
    buttonFriday->text = "П'ятниця";
    row2.push_back(buttonFriday);

    // Додаємо рядки кнопок до клавіатури
    keyboard->keyboard.push_back(row1);
    keyboard->keyboard.push_back(row2);

    // Відправка повідомлення з клавіатурою
    bot.getApi().sendMessage(message->chat->id, "Оберіть день тижня для перегляду розкладу:", false, 0, keyboard);
}
//Функція отримання повної дати
std::string getFullDate() {
    // Получение текущей даты
    time_t now = time(0);
    tm ltm = {};
    localtime_s(&ltm, &now);
    int day = ltm.tm_mday;
    int month = ltm.tm_mon + 1; // Приводим месяц к диапазону 1-12
    int year = 1900 + ltm.tm_year;

    // Проверяем, что месяц в диапазоне 1-12
    if (month < 1 || month > 12) {
        return "Невідома дата";
    }
    // Массив месяцев
    const std::string months[] = {
        "січня", "лютого", "березня", "квітня", "травня", "червня",
        "липня", "серпня", "вересня", "жовтня", "листопада", "грудня"
    };
    // Формируем дату
    std::string fullDate = std::to_string(day) + " " + months[month - 1] + " " + std::to_string(year) + " року";

    return fullDate;
}

// Підключення до бази даних або створення бази (файл 'example.db')
SQLite::Database db("example.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
// Окрема база для розкладів
SQLite::Database timetableDb("timetable.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
int main() {
    loadEnvFile("Secret.env");
    std::string botToken = getEnvVariable("BOT_TOKEN");
    TgBot::Bot bot(botToken);
    //Кнопкове меню /next week \|/
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);

    TgBot::InlineKeyboardButton::Ptr button1(new TgBot::InlineKeyboardButton);
    button1->text = "Понеділок";
    button1->callbackData = "Понеділок";
    TgBot::InlineKeyboardButton::Ptr button2(new TgBot::InlineKeyboardButton);
    button2->text = "Вівторок";
    button2->callbackData = "Вівторок";
    TgBot::InlineKeyboardButton::Ptr button3(new TgBot::InlineKeyboardButton);
    button3->text = "Середа";
    button3->callbackData = "Середа";
    TgBot::InlineKeyboardButton::Ptr button4(new TgBot::InlineKeyboardButton);
    button4->text = "Четвер";
    button4->callbackData = "Четвер";
    TgBot::InlineKeyboardButton::Ptr button5(new TgBot::InlineKeyboardButton);
    button5->text = "П'ятниця";
    button5->callbackData = "П'ятниця";

    keyboard->inlineKeyboard.push_back({ button1, button2});
    keyboard->inlineKeyboard.push_back({ button3, button4, button5 });
    //Кнопкове меню /next week /|\

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        std::string username = message->from->firstName; // Отримуємо ім'я користувача
        setUserStatusToActive(message->chat->id, username);
        std::string greeting = "Привіт " + username + "!\n"
            "Для реєстрації використовуй команду /group\n"
            "Для отримання клавіатури можна ще раз прописати /start, якщо ти вже був зареєстрований.";
        try {
            bot.getApi().sendMessage(message->chat->id, greeting);
        }
        catch (TgBot::TgException& e) {
            // Перевіряємо чи помилка в тому, що бот заблокований
            if (std::string(e.what()).find("Forbidden: bot was blocked by the user") != std::string::npos) {
                std::cerr << "Bot blocked by user with ID: " << message->chat->id << std::endl;
                // Оновлення статусу користувача в базі даних на "Blocked"
                updateUserStatusInDatabase(message->chat->id, "Blocked");
            }
            else {
                std::cerr << "error: " << e.what() << std::endl;
            }
        }
        });

    // Обробник команди /group
    bot.getEvents().onCommand("group", [&bot](TgBot::Message::Ptr message) {
        printf("User wrote: %s\n", message->text.c_str());  // Виводимо текст команди
        // Отримуємо токен бота та бази даних
        std::string dbToken = getEnvVariable("DB_TOKEN");
        std::string groupName = extractCommandArgument(message->text);
        if (!groupName.empty()) {

            try {
                std::string response = sendGetRequest(dbToken, "getListGroup");

                // Парсимо JSON-відповідь
                nlohmann::json jsonResponse = nlohmann::json::parse(response);

                // Викликаємо функцію для збереження даних у базу
                saveGroupsToDatabase(jsonResponse);

                std::cout << "Groups have been successfully saved to the database." << std::endl;

            }
            catch (const std::exception& e) {
                std::cerr << "Помилка: " << e.what() << std::endl;
                bot.getApi().sendMessage(message->chat->id, "Сталася помилка під час обробки.");
            }
            try {
                SQLite::Statement query(db, "SELECT COUNT(*) FROM codeGroup WHERE GroupName = :groupUser");
                query.bind(":groupUser", groupName);

                int groupId = getGroupIdByName(groupName);
                if (groupId != -1) { // Группа знайдена
                    std::cout << "Group " << groupName << " found with ID: " << groupId << std::endl;

                    // Оновлюємо інформацію користувача у таблиці users
                    updateUserGroupInDatabase(message->chat->id, groupName, groupId);

                    // Надсилаємо повідомлення користувачу про успішне оновлення
                    bot.getApi().sendMessage(message->chat->id, "Група зареєстрована: " + groupName);

                    // Надсилаємо клавіатуру з розкладом користувачу
                    sendScheduleReplyKeyboard(bot, message);
                }
                else {
                    std::cout << "Group " << groupName << " not found" << std::endl;
                    bot.getApi().sendMessage(message->chat->id, "Дана група не знайдена, або введена не правильно.");
                }
            }
            catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        else {
            // Якщо після команди /group не було введено групу
            std::string response = "Ця команда призначена для реєстрації або зміни твоєї групи. Для реєстрації використовуй /group група. Приклади:\n"
                "/group КІ - 21д\n"
                "/group ЕЛ - 21д\n"
                "/group IД - 21з";


            try {
                bot.getApi().sendMessage(message->chat->id, response);
            }
            catch (TgBot::TgException& e) {
                // Перевіряємо чи помилка в тому, що бот заблокований
                if (std::string(e.what()).find("Forbidden: bot was blocked by the user") != std::string::npos) {
                    std::cerr << "Bot blocked by user with ID: " << message->chat->id << std::endl;
                    // Оновлення статусу користувача в базі даних на "Blocked"
                    updateUserStatusInDatabase(message->chat->id, "Blocked");
                }
                else {
                    std::cerr << "error: " << e.what() << std::endl;
                }
            }
        }
        });
    // Обробка натискань на кнопки з назвами днів тижня
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {

        if (message->text == "Понеділок" || message->text == "Вівторок" || message->text == "Середа" ||
            message->text == "Четвер" || message->text == "П'ятниця") {

            try {
                // Отримуємо групу користувача
                SQLite::Statement groupNameQuery(db, "SELECT users.[Group] FROM users WHERE idTG = :idGroup");
                groupNameQuery.bind(":idGroup", message->chat->id);

                std::string groupName;
                if (groupNameQuery.executeStep()) {
                    groupName = groupNameQuery.getColumn(0).getString();
                }
                else {
                    throw std::runtime_error("Error: Group name not found for IDGroup.");
                }

                // Отримуємо номер тижня і тип
                std::string dbToken = getEnvVariable("DB_TOKEN");
                std::string response = sendGetRequest(dbToken, generateMode());
                json jsonResponse = json::parse(response);

                std::string weekNum = jsonResponse["weekNum"];
                std::string type = jsonResponse["weekPn"];
                std::string fullType = (type == "п") ? "Парний" : "Непарний";


                // Підготовка SQL-запиту з параметром :dayWeek
                SQLite::Statement query(timetableDb, R"(
    SELECT 
        t.lessonNum, 
        t.beginTime, 
        t.endTime, 
        g.lesson, 
        g.type,
        g.room
    FROM 
        (SELECT time.dayType, time.lessonNum, time.beginTime, time.endTime
         FROM [time]
         WHERE (((time.dayType) = :dayType))
        ) AS t
    LEFT JOIN 
       (SELECT num, lesson, numWeek, type, room
             FROM [)" + groupName + R"(] 
             WHERE dayWeek = :dayWeek  
             AND (
             REPLACE(numWeek, ' ', '') = ')"+ weekNum + R"('          -- точна відповідність
             OR REPLACE(numWeek, ' ', '') LIKE ')" + weekNum + R"(,%' -- починається з 1
             OR REPLACE(numWeek, ' ', '') LIKE '%,)" + weekNum + R"(,%' -- знаходиться посередині
             OR REPLACE(numWeek, ' ', '') LIKE '%,)" + weekNum + R"('   -- закінчується на 1
            )) AS g
    ON 
        t.lessonNum = g.num
    WHERE 
        g.lesson IS NOT NULL
    ORDER BY 
        t.lessonNum;
)");
                  
                // Отримання дня тижня від користувача
                std::string DAYWEEK = message->text;

                std::string weekInfo = weekNum + " " + fullType + "\n" + DAYWEEK;

                if (DAYWEEK == "Вівторок") {
                    query.bind(":dayType", DAYWEEK);
                }
                else {
                    query.bind(":dayType", "Будні дні");
                }
                // Прив'язуємо значення змінної DAYWEEK до параметра :dayWeek
                query.bind(":dayWeek", DAYWEEK);

                // Формуємо розклад
                std::ostringstream timetable;
                while (query.executeStep()) {
                    int lessonNum = query.getColumn("lessonNum").getInt();
                    std::string beginTime = query.getColumn("beginTime").getText();
                    std::string endTime = query.getColumn("endTime").getText();
                    std::string lesson = query.getColumn("lesson").getText();
                    std::string type = query.getColumn("type").getText();
                    std::string room = query.getColumn("room").getText();
                    timetable << lessonNum << " " << beginTime << "–" << endTime << "\n";
                    if (!lesson.empty()) {
                        timetable << lesson << " " << type <<" "<< room <<"\n";
                    }

                    timetable << "- - - - - - - - - -\n";

                }
                // Формуємо фінальне повідомлення
                std::string formattedMessage = "Номер тижня: " + weekInfo + "\n- - - - - - - - - -\n";

                if (!timetable.str().empty()) {
                    formattedMessage += timetable.str();
                }
                else {
                    formattedMessage += "Заняття відсутні";
                }

                // Додаємо надпис для вівторка
                if (DAYWEEK == "Вівторок") {
                    formattedMessage += "\n+ Кураторська година 12:00–12:30";
                }



                // Відправка повідомлення
                bot.getApi().sendMessage(message->chat->id, formattedMessage);
              //  bot.getApi().sendMessage(message->chat->id, "Ваша група : " + groupName);


            }
            catch (const std::exception& e) {
                std::cerr << "Error occurred: " << e.what() << std::endl;
                std::string errorMessage = e.what();

                // Перевіряємо, чи помилка в тому, що таблиці не існує
                if (errorMessage.find("no such table") != std::string::npos) {
                    // Виділяємо назву таблиці (групи) з тексту помилки
                    std::size_t tableNameStart = errorMessage.find(": ") + 2; // Після ": "
                    if (tableNameStart != std::string::npos) {
                        std::string tableName = errorMessage.substr(tableNameStart);

                        // Лог про помилку
                        std::cerr << "Table not found: " << tableName << std::endl;
                        bot.getApi().sendMessage(message->chat->id, "Для того щоб отримати розклад по вашій группі скористайтесь командою /get_timetable та спробуйте ще раз.");
                    }
                    else {
                        std::cerr << "Failed to extract table name from error message: " << errorMessage << std::endl;
                    }
                }
                else {
                    std::cerr << "Error: " << errorMessage << std::endl;
                    bot.getApi().sendMessage(message->chat->id, "Сталася помилка при отриманні даних.");
                }
            }

        }
        });
    // Обробка натискань на кнопки з назвами днів тижня /next week
    bot.getEvents().onCallbackQuery([&bot, &keyboard](TgBot::CallbackQuery::Ptr query) {
       
        try {
            // Отримуємо групу користувача
            SQLite::Statement groupNameQuery(db, "SELECT users.[Group] FROM users WHERE idTG = :idGroup");
            groupNameQuery.bind(":idGroup", query->message->chat->id);

            std::string groupName;
            if (groupNameQuery.executeStep()) {
                groupName = groupNameQuery.getColumn(0).getString();
            }
            else {
                throw std::runtime_error("Error: Group name not found for IDGroup.");
            }

            // Отримуємо номер тижня і тип
            std::string dbToken = getEnvVariable("DB_TOKEN");
            std::string response = sendGetRequest(dbToken, generateMode());
            json jsonResponse = json::parse(response);

            
            int weekNumber = std::stoi(jsonResponse["weekNum"].get<std::string>()); // Преобразуем в int
            weekNumber += 1; // Увеличиваем на 1
            std::string weekNum = std::to_string(weekNumber); // Преобразуем обратно в строку
            std::string type = jsonResponse["weekPn"];
            std::string fullType = (type == "п") ? "Парний" : "Непарний";


            // Підготовка SQL-запиту з параметром :dayWeek
            SQLite::Statement query1(timetableDb, R"(
    SELECT 
        t.lessonNum, 
        t.beginTime, 
        t.endTime, 
        g.lesson, 
        g.type,
        g.room
    FROM 
        (SELECT time.dayType, time.lessonNum, time.beginTime, time.endTime
         FROM [time]
         WHERE (((time.dayType) = :dayType))
        ) AS t
    LEFT JOIN 
       (SELECT num, lesson, numWeek, type, room
             FROM [)" + groupName + R"(] 
             WHERE dayWeek = :dayWeek  
             AND (
             REPLACE(numWeek, ' ', '') = ')" + weekNum + R"('          -- точна відповідність
             OR REPLACE(numWeek, ' ', '') LIKE ')" + weekNum + R"(,%' -- починається з 1
             OR REPLACE(numWeek, ' ', '') LIKE '%,)" + weekNum + R"(,%' -- знаходиться посередині
             OR REPLACE(numWeek, ' ', '') LIKE '%,)" + weekNum + R"('   -- закінчується на 1
            )) AS g
    ON 
        t.lessonNum = g.num
    WHERE 
        g.lesson IS NOT NULL
    ORDER BY 
        t.lessonNum;
)");
            // Получаем выбранный день недели из callbackData
            std::string selectedDay = query->data;
            

            std::string weekInfo = weekNum + " " + fullType + "\n" + selectedDay;

            if (selectedDay == "Вівторок") {
                query1.bind(":dayType", selectedDay);
            }
            else {
                query1.bind(":dayType", "Будні дні");
            }
            // Прив'язуємо значення змінної DAYWEEK до параметра :dayWeek
            query1.bind(":dayWeek", selectedDay);

            // Формуємо розклад
            std::ostringstream timetable;
            while (query1.executeStep()) {
                int lessonNum = query1.getColumn("lessonNum").getInt();
                std::string beginTime = query1.getColumn("beginTime").getText();
                std::string endTime = query1.getColumn("endTime").getText();
                std::string lesson = query1.getColumn("lesson").getText();
                std::string type = query1.getColumn("type").getText();
                std::string room = query1.getColumn("room").getText();
                timetable << lessonNum << " " << beginTime << "–" << endTime << "\n";
                if (!lesson.empty()) {
                    timetable << lesson << " " << type << " " << room << "\n";
                }

                timetable << "- - - - - - - - - -\n";

            }
            // Формуємо фінальне повідомлення
            std::string formattedMessage = "Номер тижня: " + weekInfo + "\n- - - - - - - - - -\n";

            if (!timetable.str().empty()) {
                formattedMessage += timetable.str();
            }
            else {
                formattedMessage += "Заняття відсутні";
            }

            // Додаємо надпис для вівторка
            if (selectedDay == "Вівторок") {
                formattedMessage += "\n+ Кураторська година 12:00–12:30";
            }



            // Відправка повідомлення
            bot.getApi().sendMessage(query->message->chat->id, formattedMessage);
            //  bot.getApi().sendMessage(message->chat->id, "Ваша група : " + groupName);
             // Подтверждаем получение callback-запроса
            bot.getApi().answerCallbackQuery(query->id);
          
        }
        catch (const std::exception& e) {
            std::cerr << "Error occurred: " << e.what() << std::endl;
            std::string errorMessage = e.what();

            // Перевіряємо, чи помилка в тому, що таблиці не існує
            if (errorMessage.find("no such table") != std::string::npos) {
                // Виділяємо назву таблиці (групи) з тексту помилки
                std::size_t tableNameStart = errorMessage.find(": ") + 2; // Після ": "
                if (tableNameStart != std::string::npos) {
                    std::string tableName = errorMessage.substr(tableNameStart);

                    // Лог про помилку
                    std::cerr << "Table not found: " << tableName << std::endl;
                    bot.getApi().sendMessage(query->message->chat->id, "Для того щоб отримати розклад по вашій группі скористайтесь командою /get_timetable та спробуйте ще раз.");
                    // Подтверждаем получение callback-запроса
                    bot.getApi().answerCallbackQuery(query->id);
                }
                else {
                    std::cerr << "Failed to extract table name from error message: " << errorMessage << std::endl;
                }
            }
            else {
                std::cerr << "Error: " << errorMessage << std::endl;
                bot.getApi().sendMessage(query->message->chat->id, "Сталася помилка при отриманні даних.");
                // Подтверждаем получение callback-запроса
                bot.getApi().answerCallbackQuery(query->id);
            }
        }
    });
    // Команда /schedule для отримання розкладу дзвінків
    bot.getEvents().onCommand("schedule", [&bot](TgBot::Message::Ptr message) {
        printf("User requested schedule\n");
        std::string dbToken = getEnvVariable("DB_TOKEN");
        std::string mode = "getLessonTime"; // Режим для отримання розкладу дзвінків

        // Викликаємо функцію GET-запиту і отримуємо відповідь від API
        std::string response = sendGetRequest(dbToken, mode);

        // Парсимо JSON відповідь
        try {
            // Парсимо JSON відповідь
            auto jsonResponse = json::parse(response);
            std::string formattedResponse = "Розклад дзвінків:\n";

            // Форматуємо дані для кожного дня
            if (jsonResponse.contains("data_everyday")) {
                formattedResponse += "Будні дні:\n";
                for (const auto& lesson : jsonResponse["data_everyday"]) {
                    std::string lessonNum = lesson["num"].get<std::string>();
                    formattedResponse += "Пара " + lessonNum + ": ";
                    formattedResponse += lesson["beginTime"].get<std::string>() + " - ";
                    formattedResponse += lesson["endTime"].get<std::string>() + "\n";
                }
            }
            if (jsonResponse.contains("data_tuesday")) {
                formattedResponse += "\nВівторок:\n";
                for (const auto& lesson : jsonResponse["data_tuesday"]) {
                    std::string lessonNum = lesson["num"].get<std::string>();
                    // Перевірка кураторської години
                    if (lessonNum.find("курат.год") != std::string::npos) {
                        formattedResponse += "Кураторська година: ";
                    }
                    else {
                        formattedResponse += "Пара " + lessonNum + ": ";
                    }
                    formattedResponse += lesson["beginTime"].get<std::string>() + " - ";
                    formattedResponse += lesson["endTime"].get<std::string>() + "\n";
                }
            }
            try {
                // Відправляємо відформатований розклад дзвінків користувачу
                bot.getApi().sendMessage(message->chat->id, formattedResponse);
            }
            catch (TgBot::TgException& e) {
                // Перевіряємо чи помилка в тому, що бот заблокований
                if (std::string(e.what()).find("Forbidden: bot was blocked by the user") != std::string::npos) {
                    std::cerr << "Bot blocked by user with ID: " << message->chat->id << std::endl;
                    // Оновлення статусу користувача в базі даних на "Blocked"
                    updateUserStatusInDatabase(message->chat->id, "Blocked");
                }
                else {
                    std::cerr << "error: " << e.what() << std::endl;
                }
            }

        }
        catch (json::parse_error& e) {
            bot.getApi().sendMessage(message->chat->id, "Сталася помилка при парсингу розкладу.");
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
        }
        });
    // Обробник команди /time
    bot.getEvents().onCommand("time", [&bot](TgBot::Message::Ptr message) {
        printf("User requested time\n");
        // Отримуємо токен бази даних
        std::string dbToken = getEnvVariable("DB_TOKEN");

        // Параметри для запиту - отримання номера тижня та дня
        std::string mode = generateMode();
        std::string response = sendGetRequest(dbToken, mode);

        // Парсинг відповіді для номера тижня та дня
        json jsonResponse = json::parse(response);
        std::string weekNum = jsonResponse["weekNum"];
        int weekDay = std::stoi(jsonResponse["weekDay"].get<std::string>());
        std::string type = jsonResponse["weekPn"];
        // Переведення типу тижня в повну форму
        std::string fullType = (type == "п") ? "Парний" : "Непарний";
        // Перетворення номера дня у назву дня
        std::string actualDay = getDayOfWeek(weekDay);

        std::string fullDate = getFullDate();


        // Форматування повідомлення
        std::string timeMessage = "Поточний час : \n📅 Дата : " + fullDate + "\n" +
            "📆 День тижня : " + actualDay + "\n" +
            "📚Номер тижня: " + weekNum + "\n" +
            "📝 " + fullType + "\n\n" +
            "Зробіть кожну хвилинку продуктивною! 🚀";
        

        try {
            // Відправка результату запиту в чат
            bot.getApi().sendMessage(message->chat->id, timeMessage);
        }
        catch (TgBot::TgException& e) {
            // Перевіряємо чи помилка в тому, що бот заблокований
            if (std::string(e.what()).find("Forbidden: bot was blocked by the user") != std::string::npos) {
                std::cerr << "Bot blocked by user with ID: " << message->chat->id << std::endl;
                // Оновлення статусу користувача в базі даних на "Blocked"
                updateUserStatusInDatabase(message->chat->id, "Blocked");
            }
            else {
                std::cerr << "error: " << e.what() << std::endl;
            }
        }

        });
    // Обробник команди /get_timetable
    bot.getEvents().onCommand("get_timetable", [&bot](TgBot::Message::Ptr message) {
        printf("User requested get_timetable\n");

        try {
            // Отримуємо всі IDGroup для користувача
            SQLite::Statement query(db, "SELECT users.[IDGroup], users.[Group] FROM users WHERE idTG = :chatId");
            query.bind(":chatId", message->chat->id);

            if (!query.executeStep()) {
                bot.getApi().sendMessage(message->chat->id, "❌ У вашому профілі не знайдено жодної групи.");
                return;
            }

            // Отримуємо IDGroup та назви груп користувача
            std::string idGroupsStr = query.getColumn(0).getString();
            std::string groupNamesStr = query.getColumn(1).getString();

            // Розбиваємо рядок IDGroup на список значень
            std::vector<std::string> idGroups;
            std::stringstream ss(idGroupsStr);
            std::string idGroup;
            while (std::getline(ss, idGroup, ',')) {
                idGroups.push_back(trimWhitespace(idGroup)); // Видаляємо зайві пробіли
            }

            // Токен бази даних
            std::string dbToken = getEnvVariable("DB_TOKEN");

            // Виконуємо запит для кожної групи
            for (const auto& groupid : idGroups) {
                // Формуємо mode для запиту
                std::string mode = generateModetimetable(groupid);

                // Виконуємо запит до сервера
                std::string response = sendGetRequest(dbToken, mode);

                // Парсимо JSON-відповідь
                nlohmann::json jsonResponse = nlohmann::json::parse(response);

                // Зберігаємо розклад у таблицю `schedule`
                savetimetableToDatabaseFull(jsonResponse, groupNamesStr);
            }
            bot.getApi().sendMessage(message->chat->id, "✅ Розклад для груп: " + groupNamesStr + " успішно отримано!");
        }
        catch (const std::exception& e) {
            std::cerr << "Database error: " << e.what() << std::endl;
            bot.getApi().sendMessage(message->chat->id, "❌ Сталася помилка під час доступу до бази даних.");
        }
        });
    // Обробник команди /add_group
    bot.getEvents().onCommand("add_group", [&bot](TgBot::Message::Ptr message){
        printf("User requested add_group\n");
        try {
            // Отримуємо аргумент команди (назву групи)
            std::string groupName = extractCommandArgument(message->text);

            // Якщо користувач не ввів групу, виводимо підказку
            if (groupName.empty()) {
                bot.getApi().sendMessage(message->chat->id, "Додайте дисципліну в групу за наступним прикладом: \n/add_group В-КНІ-206д\n/add_group В-ІТП-106д");
                return;
            }

            // Перевіряємо, чи існує така група в базі та отримуємо її ID
            SQLite::Statement checkQuery(db, "SELECT id FROM [codeGroup] WHERE GroupName = :groupName");
            checkQuery.bind(":groupName", groupName);

            std::string groupId;
            if (checkQuery.executeStep()) {
                groupId = checkQuery.getColumn(0).getString();
            }
            else {
                bot.getApi().sendMessage(message->chat->id, "❌ Код групи " + groupName + " не знайдено, перевірте правильність вводу!");
                return;
            }

            // Отримуємо поточні групи користувача
            SQLite::Statement query(db, "SELECT users.[Group], users.[IDGroup] FROM users WHERE idTG = :chatId");
            query.bind(":chatId", message->chat->id);

            std::string currentGroups, currentGroupIds;
            if (query.executeStep()) {
                currentGroups = query.getColumn(0).getString();
                currentGroupIds = query.getColumn(1).getString();
            }
            else {
                bot.getApi().sendMessage(message->chat->id, "❌ Ви ще не зареєстровані в жодній групі.");
                return;
            }

            // Перевіряємо, чи група вже додана
            if (currentGroups.find(groupName) != std::string::npos) {
                bot.getApi().sendMessage(message->chat->id, "⚠️ Група " + groupName + " вже додана.");
                return;
            }

            // Оновлюємо список груп та ID груп у базі
            std::string updatedGroups = currentGroups.empty() ? groupName : currentGroups + ", " + groupName;
            std::string updatedGroupIds = currentGroupIds.empty() ? groupId : currentGroupIds + ", " + groupId;

            SQLite::Statement updateQuery(db, "UPDATE users SET [Group] = :newGroups, [IDGroup] = :newGroupIds WHERE idTG = :chatId");
            updateQuery.bind(":newGroups", updatedGroups);
            updateQuery.bind(":newGroupIds", updatedGroupIds);
            updateQuery.bind(":chatId", message->chat->id);
            updateQuery.exec();

            bot.getApi().sendMessage(message->chat->id, "✅ Група успішно додана! Ваші групи: " + updatedGroups);
        }
        catch (const std::exception& e) {
            std::cerr << "Database error: " << e.what() << std::endl;
            bot.getApi().sendMessage(message->chat->id, "❌ Сталася помилка при оновленні даних.");
        }

       }); 
    // Обробник команди /help
    bot.getEvents().onCommand("help", [&bot](TgBot::Message::Ptr message) {
        printf("User requested help\n");
        std::string helpMessage =
            "🤖 *Доступні команди бота :*\n\n"
            "📌 */group* – Реєстрація користувача в системі та вибір основної групи.\n"
            "📌 */next_week* - Переглянути розклад на наступний тиждень.\n"
            "📌 */schedule* – Переглянути розклад дзвінків.\n"
            "📌 */time* – Отримати інформацію про поточний день.\n"
            "📌 */get_timetable* – Отримати актуальний розклад занять для вашої групи та дисциплін.\n"
            "📌 */add_group* – Додати нову дисципліну до вашого розкладу.\n\n"
            "💡 *Порада:* Якщо вам потрібно змінити список дисциплін або основну групу, скористайтеся командою */group*, щоб оновити ваші налаштування!";
        
     
        try {
            // Відправка результату в чат
            bot.getApi().sendMessage(message->chat->id, helpMessage, false, 0, nullptr, "Markdown");
        }
        catch (TgBot::TgException& e) {
            // Перевіряємо чи помилка в тому, що бот заблокований
            if (std::string(e.what()).find("Forbidden: bot was blocked by the user") != std::string::npos) {
                std::cerr << "Bot blocked by user with ID: " << message->chat->id << std::endl;
                // Оновлення статусу користувача в базі даних на "Blocked"
                updateUserStatusInDatabase(message->chat->id, "Blocked");
            }
            else {
                std::cerr << "error: " << e.what() << std::endl;
            }
        }
        });
    // Обробник команди /next_week
    bot.getEvents().onCommand("next_week", [&bot, keyboard](TgBot::Message::Ptr message) {
        printf("User requested next_week\n");   
        bot.getApi().sendMessage(message->chat->id, "Розклад на наступний тиждень",false, 0, keyboard);
        });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (TgBot::TgException& e) {
        printf("Error: %s\n", e.what());
    }
    return 0;
}