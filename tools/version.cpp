#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifdef OS_WINDOWS
#include <windows.h>
#include <tlhelp32.h>
#include <io.h>
#endif

#ifdef OS_LINUX
#include <unistd.h>
#endif

#define VERSION_DB		"version.db"
#define VERSION_H		"version.h"
#define THIS_PROGRAM	"version.exe"

using namespace std;

static char header_body[] =
{
	"/* This file is generated automatically. Do not edit directly */" \
	"\n\n" \
	"#define VERSION_MAJOR\t\t%d\n" \
	"#define VERSION_MINOR\t\t%d\n" \
	"#define VERSION_PATCH\t\t%d\n" \
	"#define VERSION_BUILD\t\t%d\n" \
	"#define VERSION_STR\t\t\t\"%s\"\n" \
	"#define BUILD_DATETIME\t\t\"%s\""
};

int header_body_len = (sizeof(header_body) / sizeof(header_body[0]));
FILE *header = NULL;
FILE *history = NULL;
static int major;
static int minor;
static int patch;
static int build;

#ifdef OS_WINDOWS
DWORD getPid(const char *imageName)
{
	HANDLE hSnapshot = NULL;
	LPPROCESSENTRY32 entry = NULL;
	DWORD pid = -1;
	size_t len;
	
	if(imageName == NULL)
		return -1;
	
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnapshot == NULL)
		return -1;
	
	entry = (LPPROCESSENTRY32)malloc(sizeof(PROCESSENTRY32));
	if(entry == NULL)
		return -1;
	
	entry->dwSize = sizeof(PROCESSENTRY32);
	
	if(!Process32First(hSnapshot, entry))
		return -1;
	
	len = strlen(imageName);
	for(;;)
	{
		if(strncmp(const_cast<char*>(imageName), const_cast<char const*>(entry->szExeFile), len) == 0)
		{
			pid = entry->th32ProcessID;
			break;
		}
		
		if(!Process32Next(hSnapshot, entry))
			break;
	}
	
	if(hSnapshot)
		CloseHandle(hSnapshot);
	
	return pid;
}

int winmain(int argc, char **argv)
{
	struct _SYSTEMTIME time;
	char buff[256];
	char time_buff[256];
	char line[8];
	char current_dir[256];
	DWORD qt_creator_pid;
	DWORD our_pid;
	int lineno = 1;
	
	GetCurrentDirectoryA(256, current_dir);
#ifndef TEST
	qt_creator_pid = getPid("qtcreator.exe");
	if(qt_creator_pid > 0)
	{
		GetWindowThreadProcessId(GetForegroundWindow(), &our_pid);
		if(qt_creator_pid != our_pid)
		{
			FreeConsole();
			MessageBoxA(0, "This tool can be used only by qtcreator", "Error", MB_ICONERROR);
			return 1;
		}
	} else 
	{
		fprintf(stdout, "Failed to get pid of a process.\n");
		return 1;
	}
#endif

	if(argc != 2)
	{
		fprintf(stdout, "usage: %s <--inc-major|--inc-minor|--inc-patch>\n", THIS_PROGRAM);
		return 1;
	}
	
	fprintf(stdout, "[%s] Generating version header ...\n", THIS_PROGRAM);
	fprintf(stdout, "[%s] creating %s\\%s\n", THIS_PROGRAM, current_dir, VERSION_H);
	header = fopen(VERSION_H, "wb");
	if(access(VERSION_DB, 00) == -1)
	{
		fprintf(stdout, "[%s] creating %s\\%s\n", THIS_PROGRAM, current_dir, VERSION_DB);
		history = fopen(VERSION_DB, "wb");
		major = 0;
		minor = 0;
		patch = 0;
		build = 0;
		fprintf(history, "%d\n%d\n%d\n%d", major, minor, build, patch);
		fclose(history);
	} else
	{
		fprintf(stdout, "[%s] reading %s\\%s\n", THIS_PROGRAM, current_dir, VERSION_DB);
		history = fopen(VERSION_DB, "r");
		while(fgets(line, 8, history) != NULL)
		{
			switch(lineno)
			{
				case 1:
					major = atoi(const_cast<char const*>(line));
					if(strncmp(argv[1], "--inc-major", 12) == 0)
						major++;
					break;
				case 2:
					minor = atoi(const_cast<char const*>(line));
					if(strncmp(argv[1], "--inc-minor", 12) == 0)
						minor++;
					break;
				case 3:
					patch = atoi(const_cast<char const*>(line));
					if(strncmp(argv[1], "--inc-patch", 12) == 0)
						patch++;
					break;
				case 4:
					build = atoi(const_cast<char const*>(line));
					build++;
					break;
			}
			lineno++;
		}
		
		fclose(history);
		Sleep(100);
		history = fopen(VERSION_DB, "wb");
		fprintf(history, "%d\n%d\n%d\n%d", major, minor, patch, build);
		fclose(history);
	}
	
	snprintf(buff, 256, "%d.%d.%d.%d", major, minor, patch, build);
	GetLocalTime(&time);
	snprintf(time_buff, 256, "%02d/%02d/%02d - %02d:%02d:%02d:%02d", time.wDay, time.wMonth, time.wYear,
	time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	fprintf(header, header_body, major, minor, patch, build, buff, time_buff);
	fclose(header);
	fprintf(stdout, "[%s] Completed\n", THIS_PROGRAM);
	fprintf(stdout, "[%s] Generated version: %d.%d.%d.%d\n", THIS_PROGRAM, major, minor, patch, build);
	
	return 0;
}
#else
int linux_main(int argc, char **argv)
{
	time_t htime;
	char buff[256];
	char time_buff[256];
	char line[8];
	char current_dir[256];
	int lineno = 1;
	struct tm *mtime = NULL;
	char *time_string = (char*)"";
	
	if(argc != 2)
	{
		fprintf(stdout, "usage: %s <--inc-major|--inc-minor|--inc-patch>\n", THIS_PROGRAM);
		return 1;
	}

	if(getcwd(current_dir, 256) == NULL)
	{
		printf( "[%s] failed to get current dir.\n", THIS_PROGRAM);
		return 1;
	}
		
	fprintf(stdout, "[%s] Generating version header ...\n", THIS_PROGRAM);
	fprintf(stdout, "[%s] creating %s/%s\n", THIS_PROGRAM, current_dir, VERSION_H);
	header = fopen(VERSION_H, "wb");
	if(access(VERSION_DB, 00) == -1)
	{
		fprintf(stdout, "[%s] creating %s/%s\n", THIS_PROGRAM, current_dir, VERSION_DB);
		history = fopen(VERSION_DB, "wb");
		major = 0;
		minor = 0;
		patch = 0;
		build = 0;
		fprintf(history, "%d\n%d\n%d\n%d", major, minor, build, patch);
		fclose(history);
	} else
	{
		fprintf(stdout, "[%s] reading %s/%s\n", THIS_PROGRAM, current_dir, VERSION_DB);
		history = fopen(VERSION_DB, "r");
		while(fgets(line, 8, history) != NULL)
		{
			switch(lineno)
			{
				case 1:
					major = atoi(const_cast<char const*>(line));
					if(strncmp(argv[1], "--inc-major", 12) == 0)
						major++;
					break;
				case 2:
					minor = atoi(const_cast<char const*>(line));
					if(strncmp(argv[1], "--inc-minor", 12) == 0)
						minor++;
					break;
				case 3:
					patch = atoi(const_cast<char const*>(line));
					if(strncmp(argv[1], "--inc-patch", 12) == 0)
						patch++;
					break;
				case 4:
					build = atoi(const_cast<char const*>(line));
					build++;
					break;
			}
			lineno++;
		}
		
		fclose(history);
		usleep(1000);
		history = fopen(VERSION_DB, "wb");
		fprintf(history, "%d\n%d\n%d\n%d", major, minor, patch, build);
		fclose(history);
	}
	
	snprintf(buff, 256, "%d.%d.%d.%d", major, minor, patch, build);
	htime = time(NULL);
	mtime = localtime(&htime);
	if(mtime == NULL)
	{	
		printf( "[%s] Failed to get local time.\n", THIS_PROGRAM);
		return 1;
	}
	
	int len;
	
	time_string = asctime(mtime);
	len = strlen(time_string);
	if(time_string[len - 1] == '\n')
		time_string[len - 1] = '\0';
		
	snprintf(time_buff, 256, "%s", time_string);
	fprintf(header, header_body, major, minor, patch, build, buff, time_buff);
	fclose(header);		
	fprintf(stdout, "[%s] Completed\n", THIS_PROGRAM);
	fprintf(stdout, "[%s] Generated version: %d.%d.%d.%d\n", THIS_PROGRAM, major, minor, patch, build);
	
	return 0;
}
#endif
int main(int argc, char **argv)
{
#ifdef OS_WINDOWS
	return(winmain(argc, argv));
#else
	return(linux_main(argc, argv));
#endif
}
