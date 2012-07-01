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

#include <festival/festival.h>
#include <libconfig.h>
#include <string.h>

extern config_setting_t *repeater_settings;
extern config_t cfg;

void vocalize(char *text)
{
  config_setting_t *voice_synthesis_settings;
  voice_synthesis_settings = config_lookup(&cfg, "repeater.voice_synthesis");
  
  int enabled;
  const char *voice;
  
  config_setting_lookup_string(voice_synthesis_settings, "voice", &voice);
  config_setting_lookup_bool(voice_synthesis_settings, "enabled", &enabled);

  if (!enabled)
    return;

  char voicename[] = "(voice_";
  strcat(voicename, voice);
  strcat(voicename, ")");  
  
  int heap_size = 210000;  // default scheme heap size
  int load_init_files = 1; // we want the festival init files loaded

  festival_initialize(load_init_files, heap_size);
  festival_eval_command(voicename);
  festival_say_text(text);
}
