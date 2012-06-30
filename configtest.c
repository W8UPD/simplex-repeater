#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

int main(int argc, char **argv)
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
    const char *identifier, *initial_identifier, *pl_tone;
    int voice_synthesis;

    if (config_setting_lookup_bool(setting, "voice_synthesis", &voice_synthesis))
      if (voice_synthesis)
	printf("Voice Synthesis: yes\n");
      else
	printf("Voice Synthesis: no\n");

    if (config_setting_lookup_string(setting, "identifier", &identifier))
      printf("Identifier: %s\n", identifier);

    if (config_setting_lookup_string(setting, "initial_identifier", &initial_identifier))
      printf("Initial Identifier: %s\n", initial_identifier);

    if (config_setting_lookup_string(setting, "pl_tone", &pl_tone))
      printf("PL Tone: %s\n", pl_tone);
    
  }
}
