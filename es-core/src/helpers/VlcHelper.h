#pragma once
#include <string>

struct libvlc_instance_t;

namespace vlc
{
	namespace helper
	{
		void InitVLC(libvlc_instance_t** vlcInstance);

	}
}
