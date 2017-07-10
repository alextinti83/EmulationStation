#include "AudioPlayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#include <random>

#include "detail/vlc_audioplayer.h"
#include "detail/vlc_playlist.h"
#include <chrono>


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

		void AudioPlayer::Pause()
		{
			m_impl->pause();
		}

		void AudioPlayer::Resume()
		{
			if (m_impl->get_state() == libvlc_Paused)
			{
				m_impl->toggle_pause();
			}
		}

		void AudioPlayer::Stop()
		{
			m_impl->stop();
		}

		void AudioPlayer::Next()
		{
			m_impl->get_playlist().next();
		}

		void AudioPlayer::Prev()
		{
			m_impl->get_playlist().prev();
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

		void AudioPlayer::StartPlaylist()
		{
			m_impl->get_playlist().play();
		}

		void AudioPlayer::AddToPlaylist(const std::string path)
		{
			m_impl->add_media_to_playlist(path);
		}

		void AudioPlayer::AddToPlaylist(const std::vector<std::string>& paths)
		{
			for (const std::string& path : paths)
			{
				AddToPlaylist(path);
			}
		}

		void AudioPlayer::AddToPlaylist(std::vector<std::string>& paths, ShuffleE shuffle)
		{
			if (shuffle == ShuffleE::k_yes)
			{
				Shuffle(paths);
			}
			AddToPlaylist(paths);
		}

		void AudioPlayer::ClearPlaylist()
		{
			m_impl->get_playlist().clear_items();
		}

		void AudioPlayer::SetPlaybacktMode(PlaybackModeE mode)
		{
			switch (mode)
			{
			case mediaplayer::PlaybackModeE::k_default:
				m_impl->get_playlist().set_playback_mode(playback_mode_t::k_default);
				break;
			case mediaplayer::PlaybackModeE::k_loop:
				m_impl->get_playlist().set_playback_mode(playback_mode_t::k_loop);
				break;
			case mediaplayer::PlaybackModeE::k_repeat:
				m_impl->get_playlist().set_playback_mode(playback_mode_t::k_repeat);
				break;
			default:
				break;
			}
		}

		void AudioPlayer::Shuffle(std::vector<std::string>& list)
		{
			auto seed = std::chrono::system_clock::now().time_since_epoch().count();
			shuffle(list.begin(), list.end(), std::default_random_engine(seed));
		}

		bool AudioPlayer::IsPlaying() const
		{
			return m_impl->get_state() == libvlc_Playing;
		}

		std::size_t AudioPlayer::PlaylistSize() const
		{
			return m_impl->get_playlist().get_item_count();
		}

	}
}