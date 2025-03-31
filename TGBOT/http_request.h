#pragma once
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <tgbot/tgbot.h>

std::string sendPostRequest(const std::string& token, const std::string& mode);
std::string sendGetRequest(const std::string& token, const std::string& mode);  //прототип функції GET-запиту

#endif // HTTP_REQUEST_H
