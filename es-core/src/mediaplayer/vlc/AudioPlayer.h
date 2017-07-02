#pragma once
#include <memory>
#include <string>

namespace detail
{
	class audioplayer;
}

namespace mediaplayer
{
	namespace vlc
	{
		class AudioPlayer
		{

		public:
			AudioPlayer();
			~AudioPlayer();
			void Play(const std::string& path);
			void Stop(const std::string& path);
		private:
			std::unique_ptr<detail::audioplayer> m_impl;

		};
	}
}
