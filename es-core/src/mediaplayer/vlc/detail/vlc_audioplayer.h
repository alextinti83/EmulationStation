#pragma once
#include <vector>
#include <functional>
#include <vlc/vlc.h>
#include "vlc_playlist.h"
#include <memory>

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
				void add_media_to_playlist(const std::string& path);
				unsigned get_volume();
				void set_volume(unsigned volume);
				void play();
				void stop();
				void pause();
				void toggle_pause();

				libvlc_state_t get_state();
				libvlc_media_t* get_current_media();

				void set_on_event_callback(on_event_callback_t callback);

				const playlist& get_playlist() const;
				playlist& get_playlist();

			private:
				void error(const std::string& errorMsg);
				static void event_proxy(const libvlc_event_t*, void*);
				void on_event(const libvlc_event_t* e);
				void attach_events(bool attach);

			private:
				libvlc_instance_t *m_vlc_instance;
				libvlc_media_player_t *m_mediaplayer;
				on_event_callback_t m_on_event_callback;
				std::unique_ptr<playlist> m_playlist;

			};
		}
	}
}