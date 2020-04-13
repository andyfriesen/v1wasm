#pragma once

#include <string>

void sound_init();
void playeffect(int efc);
void playsong(const std::string& songName);
void stopsong();
void stopsound();
unsigned char getVolume();
void setVolume(unsigned char v);
extern char speed, moneycheat;
