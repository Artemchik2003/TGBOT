#include <iostream>
#include <curl/curl.h>
#include "http_request.h"

// Callback ��� ������ ������ �������
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}
// ��������� ������� GET-������
std::string sendGetRequest(const std::string& token, const std::string& mode) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // ������� URL � �����������
        std::string url = "https://timetable.lond.lg.ua/php/javaQuery.php?token=" + token + "&mode=" + mode; 
        // ������������ URL ��� GET-������
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Callback ��� ���������� ������ �������
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        // �������� �����
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        // ������� �������
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    // ��������� ������� �������
    return readBuffer;
}
// ��������� ������� POST-������, ��� ������� ������� �������
std::string sendPostRequest(const std::string& token, const std::string& mode) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // URL ��� ������
        curl_easy_setopt(curl, CURLOPT_URL, "https://timetable.lond.lg.ua/php/javaQuery.php");

        // ������� POST-����� � �����������
        std::string postFields = "token=" + token + "&mode=" + mode;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // Callback ��� ���������� ������ �������
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // �������� �����
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // ������� �������
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    // ��������� ������� �������
    return readBuffer;
}
