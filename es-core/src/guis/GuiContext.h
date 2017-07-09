#pragma once

struct libvlc_instance_t;
class Window;

namespace mediaplayer
{
class IAudioPlayer;
}
namespace gui
{
class Context
{
public:
	Context(
		Window* window, 
		libvlc_instance_t* vlcInstance, 
		mediaplayer::IAudioPlayer* audioPlayer
	)
		: m_window(window)
		, m_vlcInstance(vlcInstance)
		, m_audioPlayer(audioPlayer)
	{ }
	libvlc_instance_t* GetVlcInstance() { return m_vlcInstance; }
	mediaplayer::IAudioPlayer* GetAudioPlayer() { return m_audioPlayer; }
	Window* GetWindow() { return m_window; }

private:
	libvlc_instance_t* m_vlcInstance;
	mediaplayer::IAudioPlayer* m_audioPlayer;
	Window* m_window;
};
}
