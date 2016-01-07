#ifndef _DASHBOARD_H_
#define _DASHBOARD_H_

#include <string>
#include <curl/curl.h>

// Defining error code
#define DASHBOARD_CONNECTION_ERROR -1
#define DASHBOARD_FALSE 0
#define DASHBOARD_TRUE 1

//TODO: implement error code for each function

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
    static bool isRunningHolder;  //Do not check this variable directly!

    static void (*callback)(bool, float, float, float);
    // Parsing the data received on the URL /status
    static size_t dataParserPosition(char* buf, size_t size, size_t nmemb,
            void* up);
    static void initialize(std::string baseURL,
            void (*callbackGetPosition)(bool, float, float, float));
    static void cleanAll();
    static void setBaseURL(std::string url);  //xxx.xxx.xxx.xxx:xxxx
    static void setCallback(void (*callbackGetPosition)(bool, float, float, float));
    static bool getPosition();
    static bool setPosition(float x, float y, float z);
    static bool sendGCodeCommand(char *command);

    static size_t dataParserStatus(char* buf, size_t size, size_t nmemb,
            void* up);
    static int isRunning();  //Test if the machine is running (the state)
};

#endif
