#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>

#include "dashboard.h"

std::string Dashboard::baseURL = "";
CURL* Dashboard::curl = NULL;
curl_slist* Dashboard::headers = NULL;
void(* Dashboard::callback)(bool, float, float, float);

void Dashboard::initialize(std::string baseURL,
        void (*callbackGetPosition)(bool, float, float, float))
{
    //TODO: open connection for test
    baseURL = baseURL;
    callback = callbackGetPosition;
    curl_global_init(CURL_GLOBAL_ALL);  //In windows, init the winsock stuff
    curl = curl_easy_init();

    headers = NULL;
    curl_slist_append(headers, "Accept: application/json");
    curl_slist_append(headers, "Content-Type: application/json");
    curl_slist_append(headers, "Charsets: UTF-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
}

void Dashboard::cleanAll()
{
    //TODO: close connection
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);  //Not thread safe!!!
    curl_global_cleanup();
}

void Dashboard::setBaseURL(std::string url)
{
    baseURL = url;
}

// Callback must be: void getPosition(bool status, float x, float y, float z);
void Dashboard::setCallback(void (*callbackGetPosition)(bool, float, float, float))
{
    callback = callbackGetPosition;
}

size_t Dashboard::dataParser(char* buf, size_t size, size_t nmemb, void* up)
{
    float x = 0, y = 0, z = 0;
    std::string data;

    if(callback == NULL)
        return 0;

    for(size_t c = 0; c < size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }

    if(buf == NULL)
        callback(false, x, y, z);

    rapidjson::Document document;
    document.Parse(buf);

    if(!document.IsObject())
        callback(false, x, y, z);

    if(!document.HasMember("data") || !document["data"].IsObject())
        callback(false, x, y, z);

    if(!document["data"].HasMember("status") ||
            !document["data"]["status"].IsObject())
    {
        callback(false, x, y, z);
    }

    if(!document["data"]["status"].HasMember("posx") &&
       !document["data"]["status"].HasMember("posy") &&
       !document["data"]["status"].HasMember("posz"))
    {
        callback(false, x, y, z);
    }

    x = document["data"]["status"]["posx"].GetDouble();
    y = document["data"]["status"]["posy"].GetDouble();
    z = document["data"]["status"]["posz"].GetDouble();

    callback(true, x, y, z);
    return size*nmemb; //tell curl how many bytes we handled
}

bool Dashboard::getPosition()
{
    std::string url = baseURL;
    url.append("/status");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &dataParser);

    return (curl_easy_perform(curl) == CURLE_OK);
}

bool Dashboard::setPosition(float x, float y, float z)
{
    CURLcode res;
    std::string url = baseURL;
    url.append("/code");

    char postField[300];  //TODO: see if enough
    sprintf(postField, "runtime=g&cmd=G0X%fY%fZ%f", x, y, z);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postField);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 0);

    res = curl_easy_perform(curl);
    return res == CURLE_OK;
}
