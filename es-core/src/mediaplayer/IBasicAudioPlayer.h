#pragma once
#include <string>
#include <functional>

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

	class IBasicAudioPlayer
	{
	public:
		using OnEventCallback = std::function<void(event_t, const std::string& path)>;
		virtual ~IBasicAudioPlayer() { };
		virtual void Play(const std::string& path) = 0;
		virtual void Stop() = 0;
		virtual void SetOnEventCallback(const OnEventCallback& c) = 0;
	};
}
