#pragma once

struct libvlc_instance_t;
class Window;

namespace mediaplayer
{
class MusicPlayer;
}
namespace gui
{
class Context
{
public:
	Context(
		Window* window, 
		libvlc_instance_t* vlcInstance, 
		mediaplayer::MusicPlayer* musicPlayer
	)
		: m_window(window)
		, m_vlcInstance(vlcInstance)
		, m_musicPlayer(musicPlayer) 
	{ }
	libvlc_instance_t* GetVlcInstance() { return m_vlcInstance; }
	mediaplayer::MusicPlayer* GetMusicPlayer() { return m_musicPlayer; }
	Window* GetWindow() { return m_window; }

private:
	libvlc_instance_t* m_vlcInstance;
	mediaplayer::MusicPlayer* m_musicPlayer;
	Window* m_window;
};
}
