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
		enum class event_t {
			/* Append new event types at the end of a category.
			* Do not remove, insert or re-order any entry.
			* Keep this in sync with lib/event.c:libvlc_event_type_name(). */
			libvlc_MediaMetaChanged = 0,
			libvlc_MediaSubItemAdded,
			libvlc_MediaDurationChanged,
			libvlc_MediaParsedChanged,
			libvlc_MediaFreed,
			libvlc_MediaStateChanged,
			libvlc_MediaSubItemTreeAdded,

			libvlc_MediaPlayerMediaChanged = 0x100,
			libvlc_MediaPlayerNothingSpecial,
			libvlc_MediaPlayerOpening,
			libvlc_MediaPlayerBuffering,
			libvlc_MediaPlayerPlaying,
			libvlc_MediaPlayerPaused,
			libvlc_MediaPlayerStopped,
			libvlc_MediaPlayerForward,
			libvlc_MediaPlayerBackward,
			libvlc_MediaPlayerEndReached,
			libvlc_MediaPlayerEncounteredError,
			libvlc_MediaPlayerTimeChanged,
			libvlc_MediaPlayerPositionChanged,
			libvlc_MediaPlayerSeekableChanged,
			libvlc_MediaPlayerPausableChanged,
			libvlc_MediaPlayerTitleChanged,
			libvlc_MediaPlayerSnapshotTaken,
			libvlc_MediaPlayerLengthChanged,
			libvlc_MediaPlayerVout,
			libvlc_MediaPlayerScrambledChanged,
			libvlc_MediaPlayerCorked = libvlc_MediaPlayerScrambledChanged + 3 + 1,
			libvlc_MediaPlayerUncorked,
			libvlc_MediaPlayerMuted,
			libvlc_MediaPlayerUnmuted,
			libvlc_MediaPlayerAudioVolume,

			libvlc_MediaListItemAdded = 0x200,
			libvlc_MediaListWillAddItem,
			libvlc_MediaListItemDeleted,
			libvlc_MediaListWillDeleteItem,

			libvlc_MediaListViewItemAdded = 0x300,
			libvlc_MediaListViewWillAddItem,
			libvlc_MediaListViewItemDeleted,
			libvlc_MediaListViewWillDeleteItem,

			libvlc_MediaListPlayerPlayed = 0x400,
			libvlc_MediaListPlayerNextItemSet,
			libvlc_MediaListPlayerStopped,

			libvlc_MediaDiscovererStarted = 0x500,
			libvlc_MediaDiscovererEnded,

			libvlc_VlmMediaAdded = 0x600,
			libvlc_VlmMediaRemoved,
			libvlc_VlmMediaChanged,
			libvlc_VlmMediaInstanceStarted,
			libvlc_VlmMediaInstanceStopped,
			libvlc_VlmMediaInstanceStatusInit,
			libvlc_VlmMediaInstanceStatusOpening,
			libvlc_VlmMediaInstanceStatusPlaying,
			libvlc_VlmMediaInstanceStatusPause,
			libvlc_VlmMediaInstanceStatusEnd,
			libvlc_VlmMediaInstanceStatusError
		};



		namespace detail
		{
			static const std::vector<event_t> event_list = {
			 event_t::libvlc_MediaPlayerMediaChanged,
			 event_t::libvlc_MediaPlayerOpening,
			 event_t::libvlc_MediaPlayerPlaying,
			 event_t::libvlc_MediaPlayerPaused,
			 event_t::libvlc_MediaPlayerStopped,
			 event_t::libvlc_MediaPlayerForward,
			 event_t::libvlc_MediaPlayerBackward,
			 event_t::libvlc_MediaPlayerEndReached,
			 event_t::libvlc_MediaPlayerEncounteredError,
			};


			class audioplayer
			{
			public:
				using on_event_callback_t = std::function<void(event_t)>;
				audioplayer(libvlc_instance_t* vlcInstance = nullptr);
				~audioplayer();

				void play(const std::string& path);
				void set_media(libvlc_media_t& media);
				void play();
				void stop();
				void pause();
				void togglePause();

				libvlc_state_t get_state();
				libvlc_media_t* get_current_media();

				void set_on_event_callback(on_event_callback_t callback);

			private:
				void error(const std::string& errorMsg);
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
				libvlc_media_player_release(m_mediaplayer);

			}

			void audioplayer::set_on_event_callback(on_event_callback_t callback)
			{
				m_on_event_callback = callback;

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
				libvlc_media_t *m = libvlc_media_new_path(m_vlcInstance, path.c_str());
				if (m)
				{
					if (m_mediaplayer == nullptr)
					{
						m_mediaplayer = libvlc_media_player_new_from_media(m);
					}
					else
					{
						set_media(*m);
					}

					play();

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