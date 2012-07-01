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

#include <pthread.h>
#include <unistd.h>
#include "speech_synthesis.h"
#include "repeater.h"
#include "config.h"
#include "weather.h"
#include "hamlib.h"

extern config_setting_t *repeater_settings;
config_t cfg;

void *test(void * nothingness){
  while (1) {
    printf("hi\n");
    sleep(2);
  }
}

int main(int argc, char **args)
{
  pthread_t repeat_thread, test_thread;
  int a, b;

  setup_config(); // Populate 'setting' variable.

  // Just a test.
  const char *identifier;
  if (config_setting_lookup_string(repeater_settings, "identifier", &identifier))
    printf("Identifier: %s\n", identifier);

  // hamlib stuff
  initialize_rig();

  fetch_weather("44325");

  // TODO: This should be its own thread.
  a = pthread_create(&repeat_thread, NULL, &repeat, NULL);
  b = pthread_create(&test_thread, NULL, &test, NULL);
  pthread_join(repeat_thread, NULL);
  pthread_join(test_thread, NULL);
  pthread_exit(NULL);
}
