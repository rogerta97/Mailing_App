#pragma once

#define CLIENTLOG(format, ...) Clientlog(__FILE__, __LINE__, format, __VA_ARGS__)

#define SERVERLOG(format, ...) Serverlog(__FILE__, __LINE__, format, __VA_ARGS__)

void Clientlog(const char file[], int line, const char* format, ...);

int ClientlogLineCount();

const char* ClientlogLineAt(int index);

void Serverlog(const char file[], int line, const char* format, ...);

int ServerlogLineCount();

const char* ServerlogLineAt(int index);

