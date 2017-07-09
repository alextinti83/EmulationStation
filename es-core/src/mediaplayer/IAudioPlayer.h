#pragma once
#include <string>
#include <functional>
#include <vector>

namespace mediaplayer
{
	enum class event_t
	{
		k_undefined,
		k_mediaChanged,
		k_opening,				
		k_playing,				
		k_paused,			
		k_stopped,				
		k_forward,			
		k_backward,			
		k_endReached,	
		k_encounteredError
	};

	enum class ShuffleE
	{
		k_yes, k_no
	};
	enum class PlaybackModeE
	{
		k_default, k_loop, k_repeat
	};

	class IAudioPlayer
	{
	public:
		using OnEventCallback = std::function<void(event_t, const std::string& path)>;
		virtual ~IAudioPlayer() { };
		virtual void Play(const std::string& path) = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual void Stop() = 0;
		virtual void SetOnEventCallback(const OnEventCallback& c) = 0;
		virtual void StartPlaylist() = 0;
		virtual void AddToPlaylist(const std::string path) = 0;
		virtual void AddToPlaylist(const std::vector<std::string>& paths) = 0;
		virtual void AddToPlaylist(std::vector<std::string>& paths, ShuffleE shuffle) = 0;
		virtual void SetPlaybacktMode(PlaybackModeE mode) = 0;
		virtual void ClearPlaylist() = 0;

		virtual bool IsPlaying() const = 0;
	};
}
