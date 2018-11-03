
#include <windows.h>
#include <stdio.h>
#include "Log.h"

#include <vector>
#include <string>

static std::vector<std::string> client_lines;
static std::vector<std::string> server_lines;

void Clientlog(const char file[], int line, const char* format, ...)
{
	static char tmp_string[4096];
	static char tmp_string2[4096];
	static va_list  ap;

	const char *basefile = file;
	const size_t filelen = strlen(file);
	for (size_t i = 0; i < filelen; ++i) {
		if (file[i] == '\\' || file[i] == '/') {
			basefile = file + i + 1;
		}
	}

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, 4096, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, 4096, "%s(%d) : %s\n", basefile, line, tmp_string);
	
	// Windows debug output
	OutputDebugString(tmp_string2);

	// Standard output
	printf("%s", tmp_string2);

	// Store the log in memory
	client_lines.push_back(tmp_string2);
}


void Serverlog(const char file[], int line, const char* format, ...)
{
	static char tmp_string[4096];
	static char tmp_string2[4096];
	static va_list  ap;

	const char *basefile = file;
	const size_t filelen = strlen(file);
	for (size_t i = 0; i < filelen; ++i) {
		if (file[i] == '\\' || file[i] == '/') {
			basefile = file + i + 1;
		}
	}

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, 4096, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, 4096, "%s(%d) : %s\n", basefile, line, tmp_string);

	// Windows debug output
	OutputDebugString(tmp_string2);

	// Standard output
	printf("%s", tmp_string2);

	// Store the log in memory
	server_lines.push_back(tmp_string2);
}

int ClientlogLineCount()
{
	return (int)client_lines.size();
}

const char * ClientlogLineAt(int index)
{
	return client_lines[index].c_str();
}

int ServerlogLineCount()
{
	return (int)server_lines.size();
}

const char * ServerlogLineAt(int index)
{
	return server_lines[index].c_str();
}

