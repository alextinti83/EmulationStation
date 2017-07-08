#pragma once
#include "IBasicAudioPlayer.h"

#include <memory>
#include <string>
#include <vector>

namespace mediaplayer
{
	class MusicPlayer
	{
	public:
		MusicPlayer(std::unique_ptr<IBasicAudioPlayer> player);
		~MusicPlayer();
		void Play(const std::string& path);
		void StartPlaylist();
		void Stop();
		void AddToPlaylist(const std::string path);
		void AddToPlaylist(const std::vector<std::string> paths);
		void ClearPlaylist();
		void ShufflePlaylist();

	private:
		void PlayNextMedia();
		void PlayCurrentMedia();
		void OnEvent(event_t e, const std::string& path);
		bool IsCurrentIndexValid() const;
		void IncreaseCurrentIndex();
		std::string GetCurrentMediaPath() const;
		
		using PlaylistT = std::vector<std::string>;

		std::unique_ptr<IBasicAudioPlayer> m_player;
		PlaylistT m_playlist;
		std::size_t m_currentIndex;
	};
}
