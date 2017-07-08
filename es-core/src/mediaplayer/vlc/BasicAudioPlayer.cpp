#include "BasicAudioPlayer.h"
#include <stdio.h>
#include <stdlib.h>

#include <vlc/vlc.h>
#include <map>
#include <vector>
#include <functional>
#include <iostream>


namespace mediaplayer
{
	namespace vlc
	{
		enum class event_t
		{
			k_mediaPlayerMediaChanged = libvlc_MediaPlayerMediaChanged,
			k_mediaPlayerOpening = libvlc_MediaPlayerOpening,
			k_mediaPlayerPlaying = libvlc_MediaPlayerPlaying,
			k_mediaPlayerPaused = libvlc_MediaPlayerPaused,
			k_mediaPlayerStopped = libvlc_MediaPlayerStopped,
			k_mediaPlayerForward = libvlc_MediaPlayerForward,
			k_mediaPlayerBackward = libvlc_MediaPlayerBackward,
			k_mediaPlayerEndReached = libvlc_MediaPlayerEndReached,
			k_mediaPlayerEncounteredError = libvlc_MediaPlayerEncounteredError,
		};

		namespace detail
		{
			static const std::vector<event_t> event_list = {
			 event_t::k_mediaPlayerMediaChanged,
			 event_t::k_mediaPlayerOpening,
			 event_t::k_mediaPlayerPlaying,
			 event_t::k_mediaPlayerPaused,
			 event_t::k_mediaPlayerStopped,
			 event_t::k_mediaPlayerForward,
			 event_t::k_mediaPlayerBackward,
			 event_t::k_mediaPlayerEndReached,
			 event_t::k_mediaPlayerEncounteredError,
			};

			class audioplayer
			{
			public:
				using on_event_callback_t = std::function<void(event_t)>;
				audioplayer(libvlc_instance_t* vlcInstance = nullptr);
				~audioplayer();

				libvlc_media_t* play(const std::string& path);
				void set_media(libvlc_media_t& media);
				void play();
				void stop();
				void pause();
				void togglePause();

				libvlc_state_t get_state();
				libvlc_media_t* get_current_media();

				void set_on_event_callback(on_event_callback_t callback);

			private:
				static void event_proxy(const libvlc_event_t*, void*);
				void on_event(const libvlc_event_t* e);
				void attach_events(bool attach);

			private:
				libvlc_instance_t *m_vlcInstance;
				libvlc_media_player_t *m_mediaplayer;
				on_event_callback_t m_on_event_callback;
			};

			audioplayer::audioplayer(libvlc_instance_t* vlcInstance)
				: m_vlcInstance(nullptr), m_mediaplayer(nullptr)
			{
     				m_vlcInstance = vlcInstance ? vlcInstance : libvlc_new(0, NULL);
			}

			audioplayer::~audioplayer()
			{
				attach_events(false);
				libvlc_release(m_vlcInstance);
			}

			void audioplayer::set_on_event_callback(on_event_callback_t callback)
			{
				m_on_event_callback = callback;
				
			}

			libvlc_media_t* audioplayer::play(const std::string& path)
			{
				libvlc_media_t *m = libvlc_media_new_path(m_vlcInstance, path.c_str());
				if (m)
				{
					if (m_mediaplayer == nullptr)
					{
						m_mediaplayer = libvlc_media_player_new_from_media(m);
						libvlc_media_release(m);
					}
					else
					{
						set_media(*m);
					}
					libvlc_media_player_play(m_mediaplayer);
					if (m_on_event_callback)
					{
						attach_events(true);
					}
					return m;
				}
				else
				{
					std::cerr << __FUNCTION__ << ": Could not play " << path << std::endl;
					if (m_on_event_callback)
					{
						m_on_event_callback(event_t::k_mediaPlayerEncounteredError);
					}
				}
				return nullptr;
			}

			libvlc_state_t audioplayer::get_state()
			{
				if (m_mediaplayer == nullptr)
				{
					return libvlc_NothingSpecial;
				}

				return libvlc_media_player_get_state(m_mediaplayer);
			}

			void audioplayer::play()
			{
				if (m_mediaplayer)
				{
					libvlc_media_player_play(m_mediaplayer);
				}
			}

			void audioplayer::stop()
			{
				if (m_mediaplayer)
				{
					libvlc_media_player_stop(m_mediaplayer);
				}
			}

			libvlc_media_t* audioplayer::get_current_media()
			{
				if (m_mediaplayer)
				{
					return libvlc_media_player_get_media(m_mediaplayer);
				}
				return nullptr;
			}

			void audioplayer::set_media(libvlc_media_t& media)
			{
				if (m_mediaplayer)
				{
					libvlc_media_player_set_media(m_mediaplayer, &media);
				}
			}

			void audioplayer::pause()
			{
				if (m_mediaplayer)
				{
					libvlc_media_player_set_pause(m_mediaplayer, true);
				}
			}

			void audioplayer::togglePause()
			{
				if (m_mediaplayer)
				{
					libvlc_media_player_pause(m_mediaplayer);
				}
			}

			void audioplayer::event_proxy(const libvlc_event_t* e, void* param)
			{
				if (param)
				{
					static_cast< audioplayer* >( param )->on_event(e);
				}
			}

			void audioplayer::on_event(const libvlc_event_t* e)
			{
				if (m_on_event_callback && e)
				{
					m_on_event_callback(static_cast< event_t >( e->type ));
				}
			}

			void audioplayer::attach_events(bool attach)
			{
				libvlc_event_manager_t* em = libvlc_media_player_event_manager(m_mediaplayer);
				if (em)
				{
					for (event_t e : event_list)
					{
						if (attach)
						{
							libvlc_event_attach(em, static_cast< libvlc_event_e >( e ), event_proxy, this);
						}
						else
						{
							libvlc_event_detach(em, static_cast< libvlc_event_e >( e ), event_proxy, this);
						}
					}
				}
			}

		}


		BasicAudioPlayer::BasicAudioPlayer()
			: m_impl(new detail::audioplayer())
		{
			// nothing to do
		}
		BasicAudioPlayer::BasicAudioPlayer(libvlc_instance_t& vlcInstance)
			: m_impl(new detail::audioplayer(&vlcInstance))
		{
			// nothing to do
		}

		BasicAudioPlayer::~BasicAudioPlayer() { }

		void BasicAudioPlayer::Play(const std::string& path)
		{
			m_path = path;
			m_impl->play(path);
		}

		void BasicAudioPlayer::Stop()
		{
			m_impl->stop();
		}

		void BasicAudioPlayer::SetOnEventCallback(const OnEventCallback& c)
		{
			m_impl->set_on_event_callback([this, c] (event_t e)
			{
				mediaplayer::event_t event(mediaplayer::event_t::k_undefined);
				switch (e)
				{
				case mediaplayer::vlc::event_t::k_mediaPlayerMediaChanged: 
					event = mediaplayer::event_t::k_mediaChanged;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerOpening:
					event = mediaplayer::event_t::k_opening;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerPlaying:
					event = mediaplayer::event_t::k_playing;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerPaused:
					event = mediaplayer::event_t::k_paused;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerStopped:
					event = mediaplayer::event_t::k_stopped;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerForward:
					event = mediaplayer::event_t::k_forward;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerBackward:
					event = mediaplayer::event_t::k_backward;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerEndReached:
					event = mediaplayer::event_t::k_endReached;
					break;
				case mediaplayer::vlc::event_t::k_mediaPlayerEncounteredError:
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