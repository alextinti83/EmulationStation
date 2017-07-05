#pragma once
#include <string>

namespace mediaplayer
{
	class IBasicAudioPlayer
	{
	public:
		virtual ~IBasicAudioPlayer() { };
		virtual void Play(const std::string& path) = 0;
		virtual void Stop() = 0;
	};
}
