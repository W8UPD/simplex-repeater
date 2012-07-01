/*
  W8UPD Simplex Repeater - A simplex repeater
  Copyright (C) 2012 W8UPD

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
  xmlChar *city;
  xmlChar *temp_f;
  xmlChar *conditions;
  xmlChar *humidity;
  xmlChar *wind;
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
  weather.city = xmlGetProp(city->nodesetval->nodeTab[0], (xmlChar*)"data");

  xmlXPathObjectPtr temp_f = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/temp_f", context);
  weather.temp_f = xmlGetProp(temp_f->nodesetval->nodeTab[0], (xmlChar*)"data");

  xmlXPathObjectPtr conditions = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/condition", context);
  weather.conditions = xmlGetProp(conditions->nodesetval->nodeTab[0], (xmlChar*)"data");

  xmlXPathObjectPtr wind = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/wind_condition", context);
  weather.wind = xmlGetProp(wind->nodesetval->nodeTab[0], (xmlChar*)"data");

  xmlXPathObjectPtr humidity = xmlXPathEvalExpression((xmlChar*)"/xml_api_reply/weather/current_conditions/humidity", context);
  weather.humidity = xmlGetProp(humidity->nodesetval->nodeTab[0], (xmlChar*)"data");

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
