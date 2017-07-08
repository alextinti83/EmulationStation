#include "MusicPlayer.h"
#include <stdio.h>
#include <stdlib.h>

#include <vlc/vlc.h>
#include <map>
#include <vector>
#include <functional>
#include <iostream>
#include <algorithm>
#include <random>
#include <iterator>


namespace mediaplayer
{

	MusicPlayer::MusicPlayer(std::unique_ptr<IBasicAudioPlayer> player)
		: m_player(std::move(player))
		, m_currentIndex(0u)
	{
		m_player->SetOnEventCallback(std::bind(&MusicPlayer::OnEvent, this, std::placeholders::_1, std::placeholders::_2));
	}

	MusicPlayer::~MusicPlayer() { }

	void MusicPlayer::Play(const std::string& path)
	{
		m_player->Play(path);
	}

	void MusicPlayer::StartPlaylist()
	{
		PlayCurrentMedia();
	}

	void MusicPlayer::IncreaseCurrentIndex()
	{
		const std::size_t nextIndex = m_currentIndex + 1;
		m_currentIndex = nextIndex % m_playlist.size();
	}
	std::string MusicPlayer::GetCurrentMediaPath() const
	{
		return IsCurrentIndexValid() ? m_playlist[ m_currentIndex ] : "";
	}

	void MusicPlayer::PlayCurrentMedia()
	{
		const std::string& currentMediaPath = GetCurrentMediaPath();
		if (!currentMediaPath.empty())
		{
			Play(currentMediaPath);
		}
	}

	void MusicPlayer::PlayNextMedia()
	{
		if (m_playlist.size() > 0)
		{
			IncreaseCurrentIndex();
			PlayCurrentMedia();
		}
	}

	void MusicPlayer::Stop()
	{
		m_player->Stop();
	}

	void MusicPlayer::AddToPlaylist(const std::string path)
	{
		m_playlist.emplace_back(path);
	}

	void MusicPlayer::AddToPlaylist(const std::vector<std::string> paths)
	{
		for (const std::string path : paths)
		{
			AddToPlaylist(path);
		}
	}

	void MusicPlayer::ClearPlaylist()
	{
		m_playlist.clear();
	}

	void MusicPlayer::ShufflePlaylist()
	{
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(m_playlist.begin(), m_playlist.end(), g);
	}

	void MusicPlayer::OnEvent(event_t e, const std::string& path)
	{
		switch (e)
		{
		case mediaplayer::event_t::k_endReached:
			PlayNextMedia();
			break;
		case mediaplayer::event_t::k_encounteredError:
			if (IsCurrentIndexValid())
			{
				m_playlist.erase(m_playlist.begin() + m_currentIndex);
			}
			PlayCurrentMedia();
			break;
		case mediaplayer::event_t::k_mediaChanged:
		case mediaplayer::event_t::k_opening:
		case mediaplayer::event_t::k_playing:
		case mediaplayer::event_t::k_paused:
		case mediaplayer::event_t::k_stopped:
		case mediaplayer::event_t::k_forward:
		case mediaplayer::event_t::k_backward:
			break;
		}
	}

	bool MusicPlayer::IsCurrentIndexValid() const
	{
		return m_currentIndex < m_playlist.size();
	}

}