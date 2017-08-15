#include "platform.h"
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <SDL.h>
#include <iostream>
#include <fcntl.h>

#ifdef WIN32
#include <codecvt>
#endif

std::string getHomePath()
{
	std::string homePath;

	// this should give you something like "/home/YOUR_USERNAME" on Linux and "C:\Users\YOUR_USERNAME\" on Windows
	const char * envHome = getenv("HOME");
	if(envHome != nullptr)
	{
		homePath = envHome;
	}

#ifdef WIN32
	// but does not seem to work for Windows XP or Vista, so try something else
	if (homePath.empty()) {
		const char * envDir = getenv("HOMEDRIVE");
		const char * envPath = getenv("HOMEPATH");
		if (envDir != nullptr && envPath != nullptr) {
			homePath = envDir;
			homePath += envPath;

			for(unsigned int i = 0; i < homePath.length(); i++)
				if(homePath[i] == '\\')
					homePath[i] = '/';
		}
	}
#endif

	// convert path to generic directory seperators
	boost::filesystem::path genericPath(homePath);
	return genericPath.generic_string();
}

std::string getVideoTitlePath()
{
	std::string titleFolder = getVideoTitleFolder();
	return titleFolder + "last_title.srt";
}

std::string getVideoTitleFolder()
{
	std::string home = getHomePath();
	return home + "/.emulationstation/tmp/";
}

int runShutdownCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -s -t 0");
#else // osx / linux
	return system("sudo shutdown -h now");
#endif
}

int runRestartCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -r -t 0");
#else // osx / linux
	return system("sudo shutdown -r now");
#endif
}

int runSystemCommand(const std::string& cmd_utf8)
{
#ifdef WIN32
	// on Windows we use _wsystem to support non-ASCII paths
	// which requires converting from utf8 to a wstring
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	std::wstring wchar_str = converter.from_bytes(cmd_utf8);
	return _wsystem(wchar_str.c_str());
#else
	return system(cmd_utf8.c_str());
#endif
}

int quitES(const std::string& filename)
{
	touch(filename);
	SDL_Event* quit = new SDL_Event();
	quit->type = SDL_QUIT;
	SDL_PushEvent(quit);
	return 0;
}

void touch(const std::string& filename)
{
#ifdef WIN32
	FILE* fp = fopen(filename.c_str(), "ab+");
	if (fp != NULL)
		fclose(fp);
#else
	int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
	if (fd >= 0)
		close(fd);
#endif
}



#ifndef WIN32
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
pid_t proc_find(const char* name)
{
	DIR* dir;
	struct dirent* ent;
	char* endptr;
	char buf[ 512 ];

	if (!( dir = opendir("/proc") ))
	{
		perror("can't open /proc");
		return -1;
	}

	while (( ent = readdir(dir) ) != NULL)
	{
		/* if endptr is not a null character, the directory is not
		* entirely numeric, so ignore it */
		long lpid = strtol(ent->d_name, &endptr, 10);
		if (*endptr != '\0')
		{
			continue;
		}

		/* try to open the cmdline file */
		snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
		FILE* fp = fopen(buf, "r");

		if (fp)
		{
			if (fgets(buf, sizeof(buf), fp) != NULL)
			{
				/* check the first token in the file, the program name */
				char* first = strtok(buf, " ");
				//LOG(LogInfo) << "proc name: " << first;
				if (!strcmp(first, name))
				{
					fclose(fp);
					closedir(dir);
					return ( pid_t ) lpid;
				}
			}
			fclose(fp);
		}
	}
	closedir(dir);
	return -1;
}
void WaitForVideoSplashScreen()
{
	bool waitForVideoSplashScreen = true;
	const std::chrono::milliseconds timespan(500);
	while (waitForVideoSplashScreen)
	{
		waitForVideoSplashScreen = proc_find("/usr/bin/omxplayer.bin") != -1;
		if (waitForVideoSplashScreen)
		{
			std::this_thread::sleep_for(timespan);
		}
	};
}
#else
void WaitForVideoSplashScreen()
{
	// nothing to do
}
#endif