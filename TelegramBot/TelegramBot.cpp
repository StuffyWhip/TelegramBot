#include <stdio.h>
#include <tgbot/tgbot.h>
#include <vector>
#include <ctime>
#include <curl/curl.h>
#include "json.hpp"
using namespace TgBot;
using namespace std;
vector<string>bot_commands = { "start" , "Currency"};
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string get_request(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}


float get_currency(char what) {
    std::string response = get_request("https://www.cbr-xml-daily.ru/daily_json.js");
    auto js_obj = nlohmann::json::parse(response);

    if (what == 'U') {  
        return js_obj["Valute"]["USD"]["Value"].get<float>();
    }
    if (what == 'E') {  
        return js_obj["Valute"]["EUR"]["Value"].get<float>();
    }
    if (what == 'C') {  
        return js_obj["Valute"]["CNY"]["Value"].get<float>();
    }
    if (what == 'G') {  
        return js_obj["Valute"]["GBP"]["Value"].get<float>();
    }
 
    return -1;
}


int main() {
    Bot bot("BOT TOKEN");
    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    vector<InlineKeyboardButton::Ptr> buttons;
    InlineKeyboardButton::Ptr usd_btn(new InlineKeyboardButton), eur_btn(new InlineKeyboardButton), cny_btn(new InlineKeyboardButton),gbp_btn(new InlineKeyboardButton);
    usd_btn->text = "USD";
    eur_btn->text = "EUR";
    cny_btn->text = "CNY";
    gbp_btn->text = "GBP";
    usd_btn->callbackData = "USD";
    eur_btn->callbackData = "EUR";
    cny_btn->callbackData = "CNY";
    gbp_btn->callbackData = "GBP";
    buttons.push_back(usd_btn);
    buttons.push_back(eur_btn);
    buttons.push_back(cny_btn);
    buttons.push_back(gbp_btn);
    keyboard->inlineKeyboard.push_back(buttons);

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        std::string response = "Hi, " + message->chat->firstName + "! Use /Currency to get currency rates.";
        bot.getApi().sendMessage(message->chat->id, response);
        });

    bot.getEvents().onCommand("Currency", [&bot,&keyboard](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Choose currency:",false,0,keyboard);
        });
    bot.getEvents().onCallbackQuery([&bot, &keyboard](CallbackQuery::Ptr query) {
        if (query->data == "USD") {
            bot.getApi().sendMessage(query->message->chat->id, to_string(get_currency('U')));

        }
        if (query->data == "EUR") {
            bot.getApi().sendMessage(query->message->chat->id, to_string(get_currency('E')));
        }
        if (query->data == "CNY") {
            bot.getApi().sendMessage(query->message->chat->id, to_string(get_currency('C')));
        }
        if (query->data == "GBP") {
            bot.getApi().sendMessage(query->message->chat->id, to_string(get_currency('G')));
        }
        });



    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        for (const auto& command : bot_commands) {
            if ("/"+command == message->text) {
                return;
            }
        }
            bot.getApi().sendMessage(message->chat->id, "Sorry, can't do it yet or can't understand you :(");
        });




    try {
        cout << "Bot username: %s\n"<< bot.getApi().getMe()->username.c_str();
        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (TgException& e) {
        cout<<"error: %s\n"<< e.what();
    }
    return 0;
}