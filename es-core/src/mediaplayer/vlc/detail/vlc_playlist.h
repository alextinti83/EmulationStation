#pragma once
#include <vlc/vlc.h>
#include <string>


namespace mediaplayer
{
	namespace vlc
	{
		enum class playback_mode_t
		{
			k_default = libvlc_playback_mode_default,
			k_loop = libvlc_playback_mode_loop,
			k_repeat = libvlc_playback_mode_repeat
		};

		namespace detail
		{
			class playlist
			{
			public:
				playlist(libvlc_instance_t* vlc_instance, libvlc_media_player_t *mp);
				~playlist();
				void play();
				bool play(unsigned idx);
				void next();
				void prev();
				bool delete_item(unsigned idx);
				void clear_items();
				std::size_t get_item_count() const;
				int add_media(libvlc_media_t& media);
				bool is_valid() const;
				void set_playback_mode(playback_mode_t m);
				playback_mode_t get_playback_mode() const;

			private:
				void deinit();
				bool init(libvlc_instance_t& vlc_instance, libvlc_media_player_t& mp);

			private:
				libvlc_media_list_player_t* m_media_list_player;
				libvlc_media_list_t* m_media_list;
				playback_mode_t m_mode;
			};
		}
	}
}