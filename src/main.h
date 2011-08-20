#pragma once

extern char qabort;
extern char* strbuf;
extern unsigned char* speech;
void err(const char* message, ...);

void StartNewGame(char* startp);
void LoadGame(char* fn);
void StartupMenu();
