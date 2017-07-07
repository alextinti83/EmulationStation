#pragma once
#include "VlcHelper.h"
#include <vlc/vlc.h>
#include "platform.h"

void vlc::helper::InitVLC(libvlc_instance_t** vlcInstance)
{
	const std::string subtitles = getVideoTitlePath();
	const char** args;
	const char* newargs[] = { "--quiet", "--sub-file", subtitles.c_str() };
	const char* singleargs[] = { "--quiet" };
	int argslen = 0;

	if (!subtitles.empty())
	{
		argslen = sizeof(newargs) / sizeof(newargs[ 0 ]);
		args = newargs;
	}
	else
	{
		argslen = sizeof(singleargs) / sizeof(singleargs[ 0 ]);
		args = singleargs;
	}
	*vlcInstance = libvlc_new(argslen, args);
}