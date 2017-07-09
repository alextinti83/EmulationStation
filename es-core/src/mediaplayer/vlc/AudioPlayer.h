#pragma once
#include <memory>
#include <string>
#include "../IAudioPlayer.h"


struct libvlc_instance_t;

namespace mediaplayer
{
	namespace vlc
	{
		namespace detail
		{
			class audioplayer;
		}

		class AudioPlayer : public IAudioPlayer
		{
		public:
			AudioPlayer();
			AudioPlayer(libvlc_instance_t& vlcInstance);
			~AudioPlayer() override;

			void Play(const std::string& path) override;
			void Stop() override;
			void SetOnEventCallback(const OnEventCallback& c) override;
		private:
			std::unique_ptr<detail::audioplayer> m_impl;
			std::string m_path;
		};
	}
}
