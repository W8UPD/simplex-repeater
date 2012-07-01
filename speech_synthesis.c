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

#include <espeak/speak_lib.h>

void vocalize(char *text, char *voicename)
{
  int synth_flags = espeakCHARS_AUTO | espeakPHONEMES | espeakENDPAUSE;
  espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
  espeak_SetVoiceByName(voicename);
  espeak_SetParameter(espeakRATE, 140, 0);
  espeak_Synth(text, 1000, 0, POS_CHARACTER, 0, synth_flags, NULL, NULL);
  espeak_Synchronize();
}