#include "MusicPlayer.h"
#include <stdio.h>
#include <stdlib.h>

#include <vlc/vlc.h>
#include <map>
#include <vector>
#include <functional>
#include <iostream>


namespace mediaplayer
{

	MusicPlayer::MusicPlayer(std::unique_ptr<IBasicAudioPlayer> player)
		: m_player(std::move(player))
	{
	}

	MusicPlayer::~MusicPlayer() { }

	void MusicPlayer::Play(const std::string& path)
	{
		m_player->Play(path);
	}

	void MusicPlayer::Stop()
	{
		m_player->Stop();
	}
}