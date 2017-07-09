#include "vlc_audioplayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

namespace mediaplayer
{
	namespace vlc
	{
		namespace detail
		{
			audioplayer::audioplayer(libvlc_instance_t* vlcInstance)
				: m_vlc_instance(nullptr), m_mediaplayer(nullptr), m_playlist()
			{
				m_vlc_instance = vlcInstance ? vlcInstance : libvlc_new(0, NULL);
				if (m_vlc_instance)
				{
					m_mediaplayer = libvlc_media_player_new(m_vlc_instance);
					if (m_mediaplayer)
					{
						m_playlist.reset(new playlist(m_vlc_instance, m_mediaplayer));
					}
				}
			}

			audioplayer::~audioplayer()
			{
				m_playlist.reset();
				attach_events(false);
				libvlc_release(m_vlc_instance);
				libvlc_media_player_release(m_mediaplayer);
			}

			void audioplayer::set_on_event_callback(on_event_callback_t callback)
			{
				m_on_event_callback = callback;
			}

			mediaplayer::vlc::detail::playlist& audioplayer::get_playlist()
			{
				return *m_playlist.get();
			}

			const mediaplayer::vlc::detail::playlist& audioplayer::get_playlist() const
			{
				return *m_playlist.get();
			}

			void audioplayer::error(const std::string& errorMsg)
			{
				std::cerr << __FUNCTION__ << " " << errorMsg << std::endl;
				if (m_on_event_callback)
				{
					m_on_event_callback(event_t::libvlc_MediaPlayerEncounteredError);
				}
			}

			void audioplayer::play(const std::string& path)
			{
				libvlc_media_t *m = libvlc_media_new_path(m_vlc_instance, path.c_str());
				if (m)
				{
					if (m_mediaplayer)
					{
						set_media(*m);
						play();
					}

					if (m_on_event_callback)
					{
						attach_events(true);
					}
					libvlc_media_release(m);
				}
				else
				{
					error("Could not play " + path);
				}
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
					if (libvlc_media_player_play(m_mediaplayer) == -1)
					{
						error("Could not play ");
					}
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

			void audioplayer::toggle_pause()
			{
				if (m_mediaplayer)
				{
					libvlc_media_player_pause(m_mediaplayer);
				}
			}

			void audioplayer::add_media_to_playlist(const std::string& path)
			{
				libvlc_media_t *m = libvlc_media_new_path(m_vlc_instance, path.c_str());
				if (m)
				{
					get_playlist().add_media(*m);
					libvlc_media_release(m);
				}
				else
				{
					error("Could not add media " + path);
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
					auto dbe = static_cast< event_t >( e->type );
					m_on_event_callback(dbe);
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
	}
}