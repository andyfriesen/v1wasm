#pragma once

#include <string>

void sound_init();
void playeffect(int efc);
void playsong(const std::string& songName);
void stopsong();
void stopsound();
extern char speed, moneycheat;
