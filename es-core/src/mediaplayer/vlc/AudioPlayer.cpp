#include "AudioPlayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <map>
#include <vector>
#include <functional>
#include "detail/vlc_audioplayer.h"


namespace mediaplayer
{
	namespace vlc
	{

		AudioPlayer::AudioPlayer()
			: m_impl(new detail::audioplayer())
		{
			// nothing to do
		}
		AudioPlayer::AudioPlayer(libvlc_instance_t& vlcInstance)
			: m_impl(new detail::audioplayer(&vlcInstance))
		{
			// nothing to do
		}

		AudioPlayer::~AudioPlayer() { }

		void AudioPlayer::Play(const std::string& path)
		{
			m_path = path;
			m_impl->play(path);
		}

		void AudioPlayer::Stop()
		{
			m_impl->stop();
		}

		void AudioPlayer::SetOnEventCallback(const OnEventCallback& c)
		{
			m_impl->set_on_event_callback([ this, c ] (event_t e)
			{
				mediaplayer::event_t event(mediaplayer::event_t::k_undefined);
				switch (e)
				{
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerMediaChanged:
					event = mediaplayer::event_t::k_mediaChanged;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerOpening:
					event = mediaplayer::event_t::k_opening;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerPlaying:
					event = mediaplayer::event_t::k_playing;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerPaused:
					event = mediaplayer::event_t::k_paused;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerStopped:
					event = mediaplayer::event_t::k_stopped;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerForward:
					event = mediaplayer::event_t::k_forward;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerBackward:
					event = mediaplayer::event_t::k_backward;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerEndReached:
					event = mediaplayer::event_t::k_endReached;
					break;
				case mediaplayer::vlc::event_t::libvlc_MediaPlayerEncounteredError:
					event = mediaplayer::event_t::k_encounteredError;
					break;
				default:
					break;
				}

				c(event, m_path);

			});
		}
	}
}