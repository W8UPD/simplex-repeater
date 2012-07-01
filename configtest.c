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
#include <string.h>
#include <libconfig.h>
#include "speech_synthesis.h"

int conf(int argc, char **argv)
{
  config_t cfg;
  config_setting_t *setting;

  config_init(&cfg);
  
  // Parse config file. If there's errors, die early.
  if (!config_read_file(&cfg, "repeater.cfg")) {
    fprintf(stderr, "%s:%d - %s\n",
	    config_error_file(&cfg),
	    config_error_line(&cfg),
	    config_error_text(&cfg));
    config_destroy(&cfg);
    return(EXIT_FAILURE);
  }

  setting = config_lookup(&cfg, "repeater");
  if (setting != NULL) {
    const char *identifier, *initial_identifier;
    int voice_synthesis;
    double pl_tone;

    if (config_setting_lookup_bool(setting, "voice_synthesis", &voice_synthesis)) {
      if (voice_synthesis)
	printf("Voice Synthesis: yes\n");
      else
	printf("Voice Synthesis: no\n");
    }

    if (config_setting_lookup_string(setting, "identifier", &identifier))
      printf("Identifier: %s\n", identifier);

    if (config_setting_lookup_string(setting, "initial_identifier", &initial_identifier))
      printf("Initial Identifier: %s\n", initial_identifier);

    if (config_setting_lookup_float(setting, "pl_tone", &pl_tone))
      printf("PL Tone: %.2f\n", pl_tone);
    
  }

  // TODO: Split this into a function, using the above "stuff".
  config_setting_t *commands;
  //  vocalize("Welcome to the W 8 U P D simplex repeater.", "default");
  //  vocalize("Here are all the commands I can perform.", "default");

  commands = config_lookup(&cfg, "repeater.commands");
  for (int i = 0; i < config_setting_length(commands); i++) {
    config_setting_t *command = config_setting_get_elem(commands, i);
    char result[1000];

    if (config_setting_get_bool(config_setting_get_member(command, "enabled"))) {
      if (config_setting_get_member(command, "argument") != NULL) {
	// We take an argument after the prefix.
	char *command_summary = "%s by dialing %s followed by the %s.";
	snprintf(result, sizeof(result), command_summary,
		 config_setting_get_string(config_setting_get_member(command, "summary")),
		 config_setting_get_string(config_setting_get_member(command, "prefix")),
		 config_setting_get_string(config_setting_get_member(command, "argument")));
      } else {
	// Just the prefix, we don't take any additional information.
	char *command_summary = "%s by dialing %s.";
	snprintf(result, sizeof(result), command_summary,
		 config_setting_get_string(config_setting_get_member(command, "summary")),
		 config_setting_get_string(config_setting_get_member(command, "prefix")));
      }

      strcat(result, "\n");
      printf(result);
      vocalize(result, "klatt");
    }
  }

  return 0;
}
