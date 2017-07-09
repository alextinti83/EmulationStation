#include "vlc_playlist.h"


namespace
{
	struct lock_playlist
	{
		lock_playlist(libvlc_media_list_t& ml) : m_ml(ml)
		{
			libvlc_media_list_lock(&m_ml);
		}
		~lock_playlist()
		{
			libvlc_media_list_unlock(&m_ml);
		}
	private:
		libvlc_media_list_t& m_ml;
	};
}

namespace mediaplayer
{
	namespace vlc
	{
		namespace detail
		{
			playlist::playlist(libvlc_instance_t* vlc_instance, libvlc_media_player_t *mp)
				: m_media_list_player(nullptr), m_media_list(nullptr), m_mode(playback_mode_t::k_default)
			{
				if (vlc_instance && mp)
				{
					init(*vlc_instance, *mp);
				}
			}


			playlist::~playlist()
			{
				deinit();
			}

			bool playlist::init(libvlc_instance_t& vlc_instance, libvlc_media_player_t& mp)
			{
				m_media_list = libvlc_media_list_new(&vlc_instance);

				if (m_media_list)
				{
					m_media_list_player = libvlc_media_list_player_new(&vlc_instance);
				}

				if (m_media_list_player)
				{
					libvlc_media_list_player_set_media_list(m_media_list_player, m_media_list);
					libvlc_media_list_player_set_media_player(m_media_list_player, &mp);
					set_playback_mode(m_mode);
					return true;
				}
				return false;
			}

			void playlist::deinit()
			{
				if (m_media_list_player)
				{
					libvlc_media_list_player_release(m_media_list_player);
					m_media_list_player = nullptr;
				}

				if (m_media_list)
				{
					libvlc_media_list_release(m_media_list);
					m_media_list = nullptr;
				}
			}
			playback_mode_t playlist::get_playback_mode() const
			{
				return m_mode;
			}

			void playlist::set_playback_mode(playback_mode_t m)
			{
				m_mode = m;
				libvlc_playback_mode_t vlc_mode = static_cast< libvlc_playback_mode_t >( m );
				libvlc_media_list_player_set_playback_mode(m_media_list_player, vlc_mode);
			}

			int playlist::add_media(libvlc_media_t& media)
			{
				int idx = -1;
				if (is_valid())
				{
					lock_playlist lock(*m_media_list);
					if (libvlc_media_list_add_media(m_media_list, &media) == 0)
					{
						idx = libvlc_media_list_index_of_item(m_media_list, &media);
					}
				}
				return idx;
			}

			void playlist::play()
			{
				if (is_valid())
				{
					libvlc_media_list_player_play(m_media_list_player);
				}
			}

			bool playlist::play(unsigned idx)
			{
				if (is_valid())
				{
					return libvlc_media_list_player_play_item_at_index(m_media_list_player, idx) == 0;
				}
				return false;
			}

			void playlist::next()
			{
				if (is_valid())
				{
					libvlc_media_list_player_next(m_media_list_player);
				}
			}

			void playlist::prev()
			{
				if (is_valid())
				{
					libvlc_media_list_player_previous(m_media_list_player);
				}
			}

			bool playlist::delete_item(unsigned idx)
			{
				bool result = false;
				if (is_valid())
				{
					lock_playlist lock(*m_media_list);
					result = libvlc_media_list_remove_index(m_media_list, idx) == 0;
				}

				return result;
			}

			void playlist::clear_items()
			{
				if (is_valid())
				{
					lock_playlist lock(*m_media_list);
					for (int i = libvlc_media_list_count(m_media_list); i > 0; --i)
					{
						libvlc_media_list_remove_index(m_media_list, i - 1);
					}
				}
			}
		}
	}
}