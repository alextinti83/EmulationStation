#include "EmulationStation.h"
#include "guis/GuiMenu.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "guis/GuiScreensaverOptions.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiDetectDevice.h"
#include "views/ViewController.h"

#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/MenuComponent.h"
#include "VolumeControl.h"
#include "Localization.h"
#include "guis/GuiContext.h"
#include "mediaplayer/IAudioPlayer.h"

GuiMenu::GuiMenu(gui::Context& context)
: GuiComponent(context), mMenu(context.GetWindow(), "MAIN MENU"), mVersion(context.GetWindow())
{
	// MAIN MENU

	// CONFIGURE INPUT >
	// SOUND SETTINGS >
	// UI SETTINGS >
	// SCRAPER >
	// QUIT >

	// [version]

	mMenu.SetScrollDelay(std::chrono::milliseconds(Settings::getInstance()->getInt("AutoScrollDelay")));

#if 0
	addEntry("CONFIGURE INPUT", 0x777777FF, true,
		[ this ]
	{
		Window* window = mWindow;
		window->pushGui(new GuiMsgBox(window, "ARE YOU SURE YOU WANT TO CONFIGURE INPUT?", "YES",
			[ window ]
		{
			window->pushGui(new GuiDetectDevice(window, false, nullptr));
		}, "NO", nullptr)
		);
	});
#else
	addEntry("CONTROLLERS SETTINGS", 0x777777FF, true, [ this ] { this->createConfigInput(); });
#endif

	addEntry("SOUND SETTINGS", 0x777777FF, true,
		[ this ]
	{
		auto s = new GuiSettings(mWindow, "SOUND SETTINGS");


		#ifdef _RPI_
			// volume control device
			auto vol_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "AUDIO DEVICE", false);
			std::vector<std::string> transitions;
			transitions.push_back("PCM");
			transitions.push_back("Speaker");
			transitions.push_back("Master");
			for(auto it = transitions.begin(); it != transitions.end(); it++)
				vol_dev->add(*it, *it, Settings::getInstance()->getString("AudioDevice") == *it);
			s->addWithLabel("AUDIO DEVICE", vol_dev);
			s->addSaveFunc([vol_dev] { 
				Settings::getInstance()->setString("AudioDevice", vol_dev->getSelected());
				VolumeControl::getInstance()->deinit();
				VolumeControl::getInstance()->init();
			});
		#endif

		// volume
		auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
		volume->setValue((float)VolumeControl::getInstance()->getVolume());
		s->addWithLabel("SYSTEM VOLUME", volume);
		s->addSaveFunc([volume] { VolumeControl::getInstance()->setVolume((int)round(volume->getValue())); });


		// disable sounds
		auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
		sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
		s->addWithLabel("ENABLE SOUNDS", sounds_enabled);
		s->addSaveFunc([ sounds_enabled ] { Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState()); });


		auto video_audio = std::make_shared<SwitchComponent>(mWindow);
		video_audio->setState(Settings::getInstance()->getBool("VideoAudio"));
		s->addWithLabel("ENABLE VIDEO AUDIO", video_audio);
		s->addSaveFunc([video_audio] { Settings::getInstance()->setBool("VideoAudio", video_audio->getState()); });

#ifdef _RPI_
		// OMX player Audio Device
		auto omx_audio_dev = std::make_shared< OptionListComponent<std::string> >(mWindow, "OMX PLAYER AUDIO DEVICE", false);
		std::vector<std::string> devices;
		devices.push_back("local");
		devices.push_back("hdmi");
		devices.push_back("both");
		// USB audio
		devices.push_back("alsa:hw:0,0");
		devices.push_back("alsa:hw:1,0");
		for (auto it = devices.begin(); it != devices.end(); it++)
			omx_audio_dev->add(*it, *it, Settings::getInstance()->getString("OMXAudioDev") == *it);
		s->addWithLabel("OMX PLAYER AUDIO DEVICE", omx_audio_dev);
		s->addSaveFunc([omx_audio_dev] {
			if (Settings::getInstance()->getString("OMXAudioDev") != omx_audio_dev->getSelected())
				Settings::getInstance()->setString("OMXAudioDev", omx_audio_dev->getSelected());
		});
#endif
		addBackgroundMusicEntries(s);
		mWindow->pushGui(s);
	});

	addEntry("UI SETTINGS", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "UI SETTINGS");
			addSystemsEntry(s);
			// theme set
			auto themeSets = ThemeData::getThemeSets();

			if (!themeSets.empty())
			{
				auto selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
				if (selectedSet == themeSets.end())
					selectedSet = themeSets.begin();

				auto theme_set = std::make_shared< OptionListComponent<std::string> >(mWindow, "THEME SET", false);
				for (auto it = themeSets.begin(); it != themeSets.end(); it++)
					theme_set->add(it->first, it->first, it == selectedSet);
				s->addWithLabel("THEME SET", theme_set);

				Window* window = mWindow;
				s->addSaveFunc([ window, theme_set ]
				{
					bool needReload = false;
					if (Settings::getInstance()->getString("ThemeSet") != theme_set->getSelected())
						needReload = true;

					Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

					if (needReload)
						ViewController::get()->reloadAll(); // TODO - replace this with some sort of signal-based implementation
				});
			}

			
			// screensaver time
			auto autoScrollDelay = std::make_shared<SliderComponent>(mWindow, 1.f, 1000.f, 5.f, "ms");
			autoScrollDelay->setValue(( float ) ( Settings::getInstance()->getInt("AutoScrollDelay")));
			autoScrollDelay->SetOnValueChangeCallback([ this, autoScrollDelay, s ](float oldValue, float newValue)
			{ 
				const int delay = static_cast< int >( newValue );
				Settings::getInstance()->setInt("AutoScrollDelay", delay);
				mMenu.SetScrollDelay(std::chrono::milliseconds(delay));
				s->SetScrollDelay(std::chrono::milliseconds(delay));
			});
			s->addWithLabel("AUTO SCROLL AFTER", autoScrollDelay);

			// screensaver time
			auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
			screensaver_time->setValue((float)(Settings::getInstance()->getInt("ScreenSaverTime") / (1000 * 60)));
			s->addWithLabel("SCREENSAVER AFTER", screensaver_time);
			s->addSaveFunc([screensaver_time] { Settings::getInstance()->setInt("ScreenSaverTime", (int)round(screensaver_time->getValue()) * (1000 * 60)); });

			// screensaver behavior
			auto screensaver_behavior = std::make_shared< OptionListComponent<std::string> >(mWindow, "SCREENSAVER BEHAVIOR", false);
			std::vector<std::string> screensavers;
			screensavers.push_back("dim");
			screensavers.push_back("black");
			screensavers.push_back("random video");
			for(auto it = screensavers.begin(); it != screensavers.end(); it++)
				screensaver_behavior->add(*it, *it, Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
			s->addWithLabel("SCREENSAVER BEHAVIOR", screensaver_behavior);
			s->addSaveFunc([this, screensaver_behavior] {
				if (Settings::getInstance()->getString("ScreenSaverBehavior") != "random video" && screensaver_behavior->getSelected() == "random video") {
					// if before it wasn't risky but now there's a risk of problems, show warning
					mWindow->pushGui(new GuiMsgBox(mWindow,
					"The \"Random Video\" screensaver shows videos from your gamelist.\n\nIf you do not have videos, or if in several consecutive attempts the games it selects don't have videos it will default to black.\n\nMore options in the \"UI Settings\" > \"Video Screensaver\" menu.",
						"OK", [] { return; }));
				}
				Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected());
			});

			ComponentListRow row;


			addLoopMenuEntries(s);

			addTemperatureEntry(s);
			

			// show filtered menu
			row.elements.clear();
			row.addElement(std::make_shared<TextComponent>(mWindow, "VIDEO SCREENSAVER SETTINGS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			row.addElement(makeArrow(mWindow), false);
			row.makeAcceptInputHandler(std::bind(&GuiMenu::openScreensaverOptions, this));
			s->addRow(row);


			// quick system select (left/right in game list view)
			auto quick_sys_select = std::make_shared<SwitchComponent>(mWindow);
			quick_sys_select->setState(Settings::getInstance()->getBool("QuickSystemSelect"));
			s->addWithLabel("QUICK SYSTEM SELECT", quick_sys_select);
			s->addSaveFunc([quick_sys_select] { Settings::getInstance()->setBool("QuickSystemSelect", quick_sys_select->getState()); });

			// transition style
			auto transition_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "TRANSITION STYLE", false);
			std::vector<std::string> transitions;
			transitions.push_back("fade");
			transitions.push_back("slide");
			transitions.push_back("simple slide");
			transitions.push_back("instant");
			for(auto it = transitions.begin(); it != transitions.end(); it++)
				transition_style->add(*it, *it, Settings::getInstance()->getString("TransitionStyle") == *it);
			s->addWithLabel("TRANSITION STYLE", transition_style);
			s->addSaveFunc([transition_style] { Settings::getInstance()->setString("TransitionStyle", transition_style->getSelected()); });



			// GameList view style
			auto gamelist_style = std::make_shared< OptionListComponent<std::string> >(mWindow, "GAMELIST VIEW STYLE", false);
			std::vector<std::string> styles;
			styles.push_back("automatic");
			styles.push_back("basic");
			styles.push_back("detailed");
			styles.push_back("video");
			for (auto it = styles.begin(); it != styles.end(); it++)
				gamelist_style->add(*it, *it, Settings::getInstance()->getString("GamelistViewStyle") == *it);
			s->addWithLabel("GAMELIST VIEW STYLE", gamelist_style);
			s->addSaveFunc([gamelist_style] {
				bool needReload = false;
				if (Settings::getInstance()->getString("GamelistViewStyle") != gamelist_style->getSelected())
					needReload = true;
				Settings::getInstance()->setString("GamelistViewStyle", gamelist_style->getSelected()); 
				if (needReload)
					ViewController::get()->reloadAll();
			});

			// show help
			auto show_help = std::make_shared<SwitchComponent>(mWindow);
			show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
			s->addWithLabel("ON-SCREEN HELP", show_help);
			s->addSaveFunc([show_help] { Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()); });

			mWindow->pushGui(s);
	});

	addEntry("OTHER SETTINGS", 0x777777FF, true,
		[this] {
			auto s = new GuiSettings(mWindow, "OTHER SETTINGS");

			// gamelists
			auto save_gamelists = std::make_shared<SwitchComponent>(mWindow);
			save_gamelists->setState(Settings::getInstance()->getBool("SaveGamelistsOnExit"));
			s->addWithLabel("SAVE METADATA ON EXIT", save_gamelists);
			s->addSaveFunc([save_gamelists] { Settings::getInstance()->setBool("SaveGamelistsOnExit", save_gamelists->getState()); });

			auto parse_gamelists = std::make_shared<SwitchComponent>(mWindow);
			parse_gamelists->setState(Settings::getInstance()->getBool("ParseGamelistOnly"));
			s->addWithLabel("PARSE GAMESLISTS ONLY", parse_gamelists);
			s->addSaveFunc([parse_gamelists] { Settings::getInstance()->setBool("ParseGamelistOnly", parse_gamelists->getState()); });

#ifdef _RPI_
			// Video Player - VideoOmxPlayer
			auto omx_player = std::make_shared<SwitchComponent>(mWindow);
			omx_player->setState(Settings::getInstance()->getBool("VideoOmxPlayer"));
			s->addWithLabel("USE OMX PLAYER (HW ACCELERATED)", omx_player);
			s->addSaveFunc([omx_player]
			{
				// need to reload all views to re-create the right video components
				bool needReload = false;
				if(Settings::getInstance()->getBool("VideoOmxPlayer") != omx_player->getState())
					needReload = true;

				Settings::getInstance()->setBool("VideoOmxPlayer", omx_player->getState());

				if(needReload)
					ViewController::get()->reloadAll();
			});
#endif

			// maximum vram
			auto max_vram = std::make_shared<SliderComponent>(mWindow, 0.f, 1000.f, 10.f, "Mb");
			max_vram->setValue((float)(Settings::getInstance()->getInt("MaxVRAM")));
			s->addWithLabel("VRAM LIMIT", max_vram);
			s->addSaveFunc([max_vram] { Settings::getInstance()->setInt("MaxVRAM", (int)round(max_vram->getValue())); });

			// framerate
			auto framerate = std::make_shared<SwitchComponent>(mWindow);
			framerate->setState(Settings::getInstance()->getBool("DrawFramerate"));
			s->addWithLabel("SHOW FRAMERATE", framerate);
			s->addSaveFunc([framerate] { Settings::getInstance()->setBool("DrawFramerate", framerate->getState()); });


			mWindow->pushGui(s);
	});

	auto openScrapeNow = [ this ] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
	addEntry("SCRAPER", 0x777777FF, true,
		[ this, openScrapeNow ]
	{
		auto s = new GuiSettings(mWindow, "SCRAPER");

		// scrape from
		auto scraper_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "SCRAPE FROM", false);
		std::vector<std::string> scrapers = getScraperList();
		for (auto it = scrapers.begin(); it != scrapers.end(); it++)
			scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));

		s->addWithLabel("SCRAPE FROM", scraper_list);
		s->addSaveFunc([ scraper_list ] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });

		// scrape ratings
		auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
		scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
		s->addWithLabel("SCRAPE RATINGS", scrape_ratings);
		s->addSaveFunc([ scrape_ratings ] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

		// scrape now
		ComponentListRow row;
		std::function<void()> openAndSave = openScrapeNow;
		openAndSave = [ s, openAndSave ] { s->save(); openAndSave(); };
		row.makeAcceptInputHandler(openAndSave);

		auto scrape_now = std::make_shared<TextComponent>(mWindow, "SCRAPE NOW", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		auto bracket = makeArrow(mWindow);
		row.addElement(scrape_now, true);
		row.addElement(bracket, false);
		s->addRow(row);

		mWindow->pushGui(s);
	});
	addEntry("QUIT", 0x777777FF, true, 
		[this] {
			auto s = new GuiSettings(mWindow, "QUIT");
			
			Window* window = mWindow;

			ComponentListRow row;
			row.makeAcceptInputHandler([ window ]
			{
				window->pushGui(new GuiMsgBox(window, "REALLY SHUTDOWN?", "YES",
					[]
				{
					if (quitES("/tmp/es-shutdown") != 0)
						LOG(LogWarning) << "Shutdown terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "SHUTDOWN SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);
			row.elements.clear();



			row.makeAcceptInputHandler([ window ]
			{
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES",
					[]
				{
					if (quitES("/tmp/es-sysrestart") != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART SYSTEM", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);
			row.elements.clear();


			row.makeAcceptInputHandler([window] {
				window->pushGui(new GuiMsgBox(window, "REALLY RESTART?", "YES",
				[] {
					if(quitES("/tmp/es-restart") != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			});
			row.addElement(std::make_shared<TextComponent>(window, "RESTART EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			s->addRow(row);
			
		
			if(Settings::getInstance()->getBool("ShowExit"))
			{
				row.elements.clear();
				row.makeAcceptInputHandler([window] {
					window->pushGui(new GuiMsgBox(window, "REALLY QUIT?", "YES", 
					[] { 
						SDL_Event ev;
						ev.type = SDL_QUIT;
						SDL_PushEvent(&ev);
					}, "NO", nullptr));
				});
				row.addElement(std::make_shared<TextComponent>(window, "QUIT EMULATIONSTATION", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
				s->addRow(row);
			}

			mWindow->pushGui(s);
	});

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0xC6C6C6FF);
	mVersion.setText("EMULATIONSTATION V" + std::string(PROGRAM_VERSION_STRING));
	mVersion.setAlignment(ALIGN_CENTER);

	addChild(&mMenu);
	addChild(&mVersion);

	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiMenu::openScreensaverOptions() {
	GuiScreensaverOptions* ggf = new GuiScreensaverOptions(mWindow, "VIDEO SCREENSAVER");
	mWindow->pushGui(ggf);
}

void GuiMenu::onSizeChanged()
{
	mVersion.setSize(mSize.x(), 0);
	mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	
	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}
	
	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

HelpStyle GuiMenu::getHelpStyle()
{
	HelpStyle style = HelpStyle();
	style.applyTheme(ViewController::get()->getState().getSystem()->getTheme(), "system");
	return style;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}



class StrInputConfig
{
public:
	StrInputConfig(std::string ideviceName, std::string ideviceGUIDString)
	{
		deviceName = ideviceName;
		deviceGUIDString = ideviceGUIDString;
	}

	std::string deviceName;
	std::string deviceGUIDString;
};


void GuiMenu::createConfigInput()
{

	GuiSettings *s = new GuiSettings(mWindow, _("CONTROLLERS SETTINGS").c_str());

	Window *window = mWindow;
	ComponentListRow row;



#if 0 //bluetooth pairing stuff disabled
	row.elements.clear();

	std::function<void(void *)> showControllerList = [ window, this, s ] (void *controllers)
	{
		std::function<void(void *)> deletePairGui = [ window ] (void *pairedPointer)
		{
			bool paired = *( ( bool * ) pairedPointer );
			window->pushGui(new GuiMsgBox(window, paired ? _("CONTROLLER PAIRED") : _("UNABLE TO PAIR CONTROLLER"), _("OK")));
		};
		if (controllers == NULL)
		{
			window->pushGui(new GuiMsgBox(window, _("AN ERROR OCCURED"), _("OK")));
		}
		else
		{
			std::vector<std::string> *resolvedControllers = ( ( std::vector<std::string> * ) controllers );
			if (resolvedControllers->size() == 0)
			{
				window->pushGui(new GuiMsgBox(window, _("NO CONTROLLERS FOUND"), _("OK")));
			}
			else
			{
				GuiSettings *pairGui = new GuiSettings(window, _("PAIR A BLUETOOTH CONTROLLER").c_str());
				for (std::vector<std::string>::iterator controllerString = ( ( std::vector<std::string> * ) controllers )->begin();
					controllerString != ( ( std::vector<std::string> * ) controllers )->end(); ++controllerString)
				{

					ComponentListRow controllerRow;
					std::function<void()> pairController = [ this, window, pairGui, controllerString, deletePairGui ]
					{
						window->pushGui(new GuiLoading(window, [ controllerString ]
						{
							bool paired = RecalboxSystem::getInstance()->pairBluetooth(*controllerString);

							return ( void * ) new bool(true);
						}, deletePairGui));

					};
					controllerRow.makeAcceptInputHandler(pairController);
					auto update = std::make_shared<TextComponent>(window, *controllerString,
						Font::get(FONT_SIZE_MEDIUM),
						0x777777FF);
					auto bracket = makeArrow(window);
					controllerRow.addElement(update, true);
					controllerRow.addElement(bracket, false);
					pairGui->addRow(controllerRow);
				}
				window->pushGui(pairGui);
			}
		}

	};

	row.makeAcceptInputHandler([ window, this, s, showControllerList ]
	{

		window->pushGui(new GuiLoading(window, []
		{
			auto s = RecalboxSystem::getInstance()->scanBluetooth();
			return ( void * ) s;
		}, showControllerList));
	});


	row.addElement(
		std::make_shared<TextComponent>(window, _("PAIR A BLUETOOTH CONTROLLER"), Font::get(FONT_SIZE_MEDIUM),
			0x777777FF),
		true);
	s->addRow(row);
	row.elements.clear();

	row.makeAcceptInputHandler([ window, this, s ]
	{
		RecalboxSystem::getInstance()->forgetBluetoothControllers();
		window->pushGui(new GuiMsgBox(window,
			_("CONTROLLERS LINKS HAVE BEEN DELETED."), _("OK")));
	});
	row.addElement(
		std::make_shared<TextComponent>(window, _("FORGET BLUETOOTH CONTROLLERS"), Font::get(FONT_SIZE_MEDIUM),
			0x777777FF),
		true);
	s->addRow(row);
	row.elements.clear();
#endif


	row.elements.clear();

	// Here we go; for each player
	std::list<int> alreadyTaken = std::list<int>();

	// clear the current loaded inputs
	clearLoadedInput();

	std::vector<std::shared_ptr<OptionListComponent<StrInputConfig *>>> options;
	char strbuf[ 256 ];

	for (int player = 0; player < MAX_PLAYERS; player++)
	{
		std::stringstream sstm;
		sstm << "INPUT P" << player + 1;
		std::string confName = sstm.str() + "NAME";
		std::string confGuid = sstm.str() + "GUID";
		snprintf(strbuf, 256, _("INPUT P%i").c_str(), player + 1);

		LOG(LogInfo) << player + 1 << " " << confName << " " << confGuid;
		auto inputOptionList = std::make_shared<OptionListComponent<StrInputConfig *> >(mWindow, strbuf, false);
		options.push_back(inputOptionList);

		// Checking if a setting has been saved, else setting to default
		std::string configuratedName = Settings::getInstance()->getString(confName);
		std::string configuratedGuid = Settings::getInstance()->getString(confGuid);
		bool found = false;
		// For each available and configured input
		for (auto it = 0; it < InputManager::getInstance()->getNumJoysticks(); it++)
		{
			InputConfig *config = InputManager::getInstance()->getInputConfigByDevice(it);
			if (config->isConfigured())
			{
				// create name
				std::stringstream dispNameSS;
				dispNameSS << "#" << config->getDeviceId() << " ";
				std::string deviceName = config->getDeviceName();
				if (deviceName.size() > 25)
				{
					dispNameSS << deviceName.substr(0, 16) << "..." <<
						deviceName.substr(deviceName.size() - 5, deviceName.size() - 1);
				}
				else
				{
					dispNameSS << deviceName;
				}

				std::string displayName = dispNameSS.str();


				bool foundFromConfig = configuratedName == config->getDeviceName() &&
					configuratedGuid == config->getDeviceGUIDString();
				int deviceID = config->getDeviceId();
				StrInputConfig* newInputConfig = new StrInputConfig(config->getDeviceName(), config->getDeviceGUIDString());
				mLoadedInput.push_back(newInputConfig);

				if (foundFromConfig
					&& std::find(alreadyTaken.begin(), alreadyTaken.end(), deviceID) == alreadyTaken.end()
					&& !found)
				{
					found = true;
					alreadyTaken.push_back(deviceID);
					LOG(LogWarning) << "adding entry for player" << player << " (selected): " <<
						config->getDeviceName() << "  " << config->getDeviceGUIDString();
					inputOptionList->add(displayName, newInputConfig, true);
				}
				else
				{
					LOG(LogWarning) << "adding entry for player" << player << " (not selected): " <<
						config->getDeviceName() << "  " << config->getDeviceGUIDString();
					inputOptionList->add(displayName, newInputConfig, false);
				}
			}
		}
		if (configuratedName.compare("") == 0 || !found)
		{
			LOG(LogWarning) << "adding default entry for player " << player << "(selected : true)";
			inputOptionList->add("default", NULL, true);
		}
		else
		{
			LOG(LogWarning) << "adding default entry for player" << player << "(selected : false)";
			inputOptionList->add("default", NULL, false);
		}

		// ADD default config

		// Populate controllers list
		s->addWithLabel(strbuf, inputOptionList);
	}
	s->addSaveFunc([ this, options, window ]
	{
		for (int player = 0; player < MAX_PLAYERS; player++)
		{
			std::stringstream sstm;
			sstm << "INPUT P" << player + 1;
			std::string confName = sstm.str() + "NAME";
			std::string confGuid = sstm.str() + "GUID";

			auto input_p1 = options.at(player);
			std::string name;
			std::string selectedName = input_p1->getSelectedName();

			if (selectedName.compare(strToUpper("default")) == 0)
			{
				name = "DEFAULT";
				Settings::getInstance()->setString(confName, name);
				Settings::getInstance()->setString(confGuid, "");
			}
			else
			{
				if (input_p1->getSelected() != NULL)
				{
					LOG(LogWarning) << "Found the selected controller ! : name in list  = " << selectedName;
					LOG(LogWarning) << "Found the selected controller ! : guid  = " << input_p1->getSelected()->deviceGUIDString;

					Settings::getInstance()->setString(confName, input_p1->getSelected()->deviceName);
					Settings::getInstance()->setString(confGuid, input_p1->getSelected()->deviceGUIDString);
				}
			}
		}

		Settings::getInstance()->saveFile();

	});

	row.elements.clear();
	row.makeAcceptInputHandler([ window, this, s ]
	{
		window->pushGui(new GuiMsgBox(window,
			_("YOU ARE GOING TO CONFIGURE A CONTROLLER. IF YOU HAVE ONLY ONE JOYSTICK, "
				"CONFIGURE THE DIRECTIONS KEYS AND SKIP JOYSTICK CONFIG BY HOLDING A BUTTON. "
				"IF YOU DO NOT HAVE A SPECIAL KEY FOR HOTKEY, CHOOSE THE SELECT BUTTON. SKIP "
				"ALL BUTTONS YOU DO NOT HAVE BY HOLDING A KEY. BUTTONS NAMES ARE BASED ON THE "
				"SNES CONTROLLER."), _("OK"),
			[ window, this, s ]
		{
			window->pushGui(new GuiDetectDevice(window, false, [ this, s ]
			{
				//	s->setSave(false);
				delete s;
				this->createConfigInput();
			}));
		}, "CANCEL", nullptr)
		);
	});


	row.addElement(
		std::make_shared<TextComponent>(window, _("CONFIGURE A CONTROLLER"), Font::get(FONT_SIZE_MEDIUM), 0x777777FF),
		true);
	s->addRow(row);
	row.elements.clear();


	window->pushGui(s);

}

void GuiMenu::clearLoadedInput()
{
	for (uint32_t i = 0; i < mLoadedInput.size(); i++)
	{
		delete mLoadedInput[ i ];
	}
	mLoadedInput.clear();
}

void GuiMenu::addTemperatureEntry(GuiSettings* s)
{
	Window* window = mWindow;
	auto createOptionList = [window] ()
	{ 
		auto temperature = std::make_shared< OptionListComponent<std::string> >(window, "SHOW TEMPERATURE", false);
		const std::vector<std::string> temperatureOptions({ "always", "> hi-temp only", "never" });
		for (const std::string& option : temperatureOptions)
		{
			temperature->add(option, option, Settings::getInstance()->getString("ShowTemperature") == option);
		}
		return temperature;
	};
	
	ComponentListRow row;
	row.makeAcceptInputHandler([ window, createOptionList ]
	{
		auto s = new GuiSettings(window, "TEMPERATURE");
		auto temperature = createOptionList();
		s->addWithLabel("SHOW TEMPERATURE", temperature);
		s->addSaveFunc([ temperature ]
		{
			Settings::getInstance()->setString("ShowTemperature", temperature->getSelected
			());
		});

		auto slider = std::make_shared<SliderComponent>(window, 0.f, 100.f, 1.f, " C");
		slider->setValue(static_cast<float>(Settings::getInstance()->getInt("HiTemperature")));
		s->addWithLabel("SET HI-TEMP", slider);
		s->addSaveFunc([ slider ] { 
			Settings::getInstance()->setInt("HiTemperature", std::lroundf(slider->getValue() ));
		});
		window->pushGui(s);
	});
	row.addElement(std::make_shared<TextComponent>(window, "TEMPERATURE", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	s->addRow(row);
	row.elements.clear();
}


void GuiMenu::addSystemsEntry(GuiSettings* s)
{
	Window* window = mWindow;

	ComponentListRow row;
	row.makeAcceptInputHandler([ window ]
	{
		auto s = new GuiSettings(window, "ENABLE SYSTEMS");
		std::vector<std::pair<std::shared_ptr<SwitchComponent>, SystemData*>> switches;
		for(SystemData* system : SystemData::GetAllSystems())
		{
			auto onOffSwitch = std::make_shared<SwitchComponent>(window);
			switches.push_back(std::make_pair(onOffSwitch, system));
			onOffSwitch->setState(system->IsEnabled());
			s->addWithLabel(system->getName(), onOffSwitch);
			s->addSaveFunc([ system, onOffSwitch ]
			{
				system->SetEnabled(onOffSwitch->getState());
			});
		}
		s->setCloseFunc([
			window, 
					 // this vector captured by copy is intended
			switches // I'm getting weird behaviors by capturing its reference
					 // it might be some stack memory corrupted
		]
		{
			bool needsRefresh = false;
			for (auto& onOff: switches)
			{
				if (onOff.first->getState() != onOff.second->IsEnabled())
				{
					needsRefresh = true;
				}
			}
			if (needsRefresh)
			{

				window->pushGui(new GuiMsgBox(window, "RESTART EMULATION STATION TO APPLY CHANGES?", "YES",
					[]
				{
					if (quitES("/tmp/es-restart") != 0)
						LOG(LogWarning) << "Restart terminated with non-zero result!";
				}, "NO", nullptr));
			}
		});
		window->pushGui(s);
	});
	row.addElement(std::make_shared<TextComponent>(window, "SYSTEMS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	s->addRow(row);
	row.elements.clear();
}

void GuiMenu::addLoopMenuEntries(GuiSettings* s)
{
	auto loop = std::make_shared<SwitchComponent>(mWindow);
	loop->setState(Settings::getInstance()->getBool("LoopMenuEntries"));
	s->addWithLabel("LOOP MENU ENTRIES", loop);
	s->addSaveFunc([ loop ] { Settings::getInstance()->setBool("LoopMenuEntries", loop->getState()); });
}

void GuiMenu::addBackgroundMusicEntries(GuiSettings* s)
{
	auto enabled = std::make_shared<SwitchComponent>(mWindow);
	enabled->setState(Settings::getInstance()->getBool("BackgroundMusicEnabled"));
	s->addWithLabel("BACKGROUND MUSIC", enabled);
	s->addSaveFunc([ enabled, this ] { 
		const bool state = enabled->getState();
		Settings::getInstance()->setBool("BackgroundMusicEnabled", state); 
		if (state)
		{
			m_context->GetAudioPlayer()->StartPlaylist();
		}
		else
		{
			m_context->GetAudioPlayer()->Stop();
		}
	});

	// volume
	auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
	unsigned v = m_context->GetAudioPlayer()->GetVolume();
	volume->setValue(static_cast<float>(v));
	s->addWithLabel("BACKGROUND MUSIC VOLUME", volume);
	s->addSaveFunc([ volume, this ] { m_context->GetAudioPlayer()->SetVolume(( int ) round(volume->getValue())); });
}