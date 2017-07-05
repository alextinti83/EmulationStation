#pragma once
#include <memory>
#include <string>
#include "IBasicAudioPlayer.h"



namespace mediaplayer
{
	class MusicPlayer
	{
	public:
		MusicPlayer(std::unique_ptr<IBasicAudioPlayer> player);
		~MusicPlayer();
		void Play(const std::string& path);
		void Stop();
	private:
		std::unique_ptr<IBasicAudioPlayer> m_player;
	};
}
