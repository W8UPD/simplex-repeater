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

#include <hamlib/rig.h>
#include <string.h>
#include <stdlib.h>
#include <libconfig.h>

extern config_setting_t *repeater_settings;
extern config_t cfg;

int initialize_rig()
{
  RIG *my_rig;            /* handle to rig (nstance) */
  int retcode;

  config_setting_t *rig_control_settings;
  rig_control_settings = config_lookup(&cfg, "repeater.rig_control");

  int rig_id, baud_rate, data_bits, stop_bits;
  const char *serial_port, *frequency;
  
  config_setting_lookup_int(rig_control_settings, "rig_id", &rig_id);
  config_setting_lookup_int(rig_control_settings, "baud_rate", &baud_rate);
  config_setting_lookup_int(rig_control_settings, "data_bits", &data_bits);
  config_setting_lookup_int(rig_control_settings, "stop_bits", &stop_bits);
  config_setting_lookup_string(rig_control_settings, "frequency", &frequency);
  config_setting_lookup_string(rig_control_settings, "serial_port", &serial_port);
  
  printf("frequency: %s\n", frequency);
  
  my_rig = rig_init(rig_id);

  if (!my_rig) {
    fprintf(stderr,"Unknown rig num: %d\n", rig_id);
    fprintf(stderr,"Please check riglist.h\n");
    exit(1); /* whoops! something went wrong (mem alloc?) */
  }

  strncpy(my_rig->state.rigport.pathname, "/dev/ttyS0", FILPATHLEN - 1);
  my_rig->state.rigport.parm.serial.rate = baud_rate;
  my_rig->state.rigport.parm.serial.data_bits = data_bits;
  my_rig->state.rigport.parm.serial.stop_bits = stop_bits;

  retcode = rig_open(my_rig);
  if (retcode != RIG_OK) {
    printf("rig_open: error = %s\n", rigerror(retcode));
    exit(2);
  }

  return rig_set_freq(my_rig, RIG_VFO_CURR, atoi(frequency));
}
