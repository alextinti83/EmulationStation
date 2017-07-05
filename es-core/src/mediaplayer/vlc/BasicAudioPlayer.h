#pragma once
#include <memory>
#include <string>
#include "../IBasicAudioPlayer.h"



namespace mediaplayer
{
	namespace vlc
	{
		namespace detail
		{
			class audioplayer;
		}

		class BasicAudioPlayer : public IBasicAudioPlayer
		{
		public:
			BasicAudioPlayer();
			~BasicAudioPlayer() override;

			void Play(const std::string& path) override;
			void Stop() override;
		private:
			std::unique_ptr<detail::audioplayer> m_impl;
		};
	}
}
