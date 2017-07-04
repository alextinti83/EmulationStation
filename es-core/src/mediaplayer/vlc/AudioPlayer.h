#pragma once
#include <memory>
#include <string>



namespace mediaplayer
{
	namespace vlc
	{
		namespace detail
		{
			class audioplayer;
		}

		class AudioPlayer
		{

		public:
			AudioPlayer();
			~AudioPlayer();
			void Play(const std::string& path);
			void Stop();
		private:
			std::unique_ptr<detail::audioplayer> m_impl;

		};
	}
}
