#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "speech_synthesis.h"

CURL *curl;
CURLcode curl_res;

typedef struct {
  char *city;
  char *temp_f;
  char *conditions;
  char *humidity;
  char *wind;
} gweather_t;

gweather_t parseit() {
  gweather_t weather;
  xmlDocPtr doc = xmlReadFile("/tmp/gweather.xml", NULL, 0);
  if (!doc) {
    printf("Failed to parse XML file.");
    exit(1);
  }

  xmlXPathContext *context = xmlXPathNewContext(doc);

  xmlXPathObjectPtr city = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/forecast_information/city", context);
  weather.city = xmlGetProp(city->nodesetval->nodeTab[0], "data");

  xmlXPathObjectPtr temp_f = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/temp_f", context);
  weather.temp_f = xmlGetProp(temp_f->nodesetval->nodeTab[0], "data");

  xmlXPathObjectPtr conditions = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/condition", context);
  weather.conditions = xmlGetProp(conditions->nodesetval->nodeTab[0], "data");

  xmlXPathObjectPtr wind = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/wind_condition", context);
  weather.wind = xmlGetProp(wind->nodesetval->nodeTab[0], "data");

  xmlXPathObjectPtr humidity = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/humidity", context);
  weather.humidity = xmlGetProp(humidity->nodesetval->nodeTab[0], "data");

  xmlFreeDoc(doc);
  xmlCleanupParser();
  return weather;
}

void fetch_weather(char *location) {
  FILE *tmp = fopen("/tmp/gweather.xml", "w");
  if (!tmp) {
    printf("Could not open /tmp/gweather.json for some odd reason.\n");
  }

  char url[100] = "https://www.google.com/ig/api?weather=";
  strcat(url, location);

  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmp);
  curl_res = curl_easy_perform(curl);
  if (curl_res == 0) {
    printf("Fetched google weather JSON\n");
  } else {
    printf("Error.\n");
  }

  fclose(tmp);  
  curl_easy_cleanup(curl);
  gweather_t weather = parseit();
  char result[1000];
  snprintf(result, sizeof(result), "Weather for %s. Currently: %s degrees fahrenheit. %s. %s. %s.",
	   weather.city, weather.temp_f, weather.conditions, weather.wind, weather.humidity);
  vocalize(result, "klatt");
}

int main(void) {
  fetch_weather("44203");
  return 0;
}
