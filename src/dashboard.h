#ifndef _DASHBOARD_H_
#define _DASHBOARD_H_

#include <string>
#include <curl/curl.h>

/**
 * Used to communicate to the machine.
 * Because of the callback functions, everything is static. Plus this class
 * is unique in this program.
 */
class Dashboard
{
public:
    static std::string baseURL;
    static CURL *curl;
    static struct curl_slist *headers;

    static void (*callback)(bool, float, float, float);
    // Parsing the data received on the URL /status
    static size_t dataParser(char* buf, size_t size, size_t nmemb, void* up);
    static void initialize(std::string baseURL,
            void (*callbackGetPosition)(bool, float, float, float));
    static void cleanAll();
    static void setBaseURL(std::string url);  //xxx.xxx.xxx.xxx:xxxx
    static void setCallback(void (*callbackGetPosition)(bool, float, float, float));
    static bool getPosition();
    static bool setPosition(float x, float y, float z);

};

#endif
