#include "AudioPlayer.h"
#include <stdio.h>
#include <stdlib.h>

#include <vlc/vlc.h>
#include <map>

namespace detail
{
	class audioplayer
	{
		class mp_handle
		{
		public:
			mp_handle(const std::string& path, libvlc_instance_t* vlcInstance)
			{
				libvlc_media_t *m = libvlc_media_new_path(vlcInstance, path.c_str());
				m_mediaplayer = libvlc_media_player_new_from_media(m);
				libvlc_media_release(m);
			}
			~mp_handle()
			{
				libvlc_media_player_release(m_mediaplayer);
			}
			libvlc_media_player_t& get_mediaplayer() { return *m_mediaplayer; }
			const libvlc_media_player_t& get_mediaplayer() const { return *m_mediaplayer; }

		private:
			const std::string m_path;
			libvlc_media_player_t *m_mediaplayer;
		};

	public:
		audioplayer();
		~audioplayer();

		void play(const std::string& path);
		void stop(const std::string& path);

		libvlc_media_player_t& init_handle(const std::string& path);
		libvlc_media_player_t* get_handle(const std::string& path);

	private:
		void stop(libvlc_media_player_t& mp);
		void play(libvlc_media_player_t& mp);

	private:
		libvlc_instance_t *m_vlcInstance;
		std::map<std::string, std::unique_ptr<mp_handle>> m_handles;
	};

	audioplayer::audioplayer()
	{
		m_vlcInstance = libvlc_new(0, NULL);
	}

	audioplayer::~audioplayer()
	{
		libvlc_release(m_vlcInstance);
	}

	libvlc_media_player_t& audioplayer::init_handle(const std::string& path)
	{
		auto mp = m_handles.find(path);
		if (mp == m_handles.cend())
		{
			auto mp_h = std::unique_ptr<mp_handle>(new mp_handle(path, m_vlcInstance));
			auto result = m_handles.emplace(path, std::move(mp_h));
			return result.first->second->get_mediaplayer();
		}
		else
		{
			return mp->second->get_mediaplayer();
		}
	}

	libvlc_media_player_t* audioplayer::get_handle(const std::string& path)
	{
		auto mp = m_handles.find(path);
		if (mp != m_handles.cend())
		{
			return &(mp->second->get_mediaplayer());
		}
		return nullptr;
	}

	void audioplayer::play(const std::string& path)
	{
		libvlc_media_player_t& mp = init_handle(path);
		play(mp);
	}

	void audioplayer::stop(const std::string& path)
	{
		libvlc_media_player_t* mp = get_handle(path);
		if (mp) { stop(*mp); }
	}

	void audioplayer::play(libvlc_media_player_t& mp)
	{
		libvlc_media_player_play(&mp);
	}

	void audioplayer::stop(libvlc_media_player_t& mp)
	{
		libvlc_media_player_stop(&mp);
	}

}

namespace mediaplayer
{
	namespace vlc
	{

		AudioPlayer::AudioPlayer()
			: m_impl(new detail::audioplayer())
		{
		}

		AudioPlayer::~AudioPlayer() { }

		void AudioPlayer::Play(const std::string& path)
		{
			m_impl->play(path);
		}

		void AudioPlayer::Stop(const std::string& path)
		{
			m_impl->stop(path);
		}

	}
}