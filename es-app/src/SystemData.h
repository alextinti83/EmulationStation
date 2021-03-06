#pragma once

#include <vector>
#include <string>
#include "FileData.h"
#include "Window.h"
#include "MetaData.h"
#include "PlatformId.h"
#include "ThemeData.h"
#include "FileFilterIndex.h"

class GameCollections;

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& fullName, const std::string& startPath, const std::vector<std::string>& extensions,
		const std::string& command, const std::vector<PlatformIds::PlatformId>& platformIds, const std::string& themeFolder, const bool enabled = true);
	~SystemData();

	inline FileData* getRootFolder() const { return mRootFolder; };
	inline const std::string& getName() const { return mName; }
	inline const std::string& getFullName() const { return mFullName; }
	inline const std::string& getStartPath() const { return mStartPath; }
	inline const std::vector<std::string>& getExtensions() const { return mSearchExtensions; }
	inline const std::string& getThemeFolder() const { return mThemeFolder; }

	inline const std::vector<PlatformIds::PlatformId>& getPlatformIds() const { return mPlatformIds; }
	inline bool hasPlatformId(PlatformIds::PlatformId id) { return std::find(mPlatformIds.begin(), mPlatformIds.end(), id) != mPlatformIds.end(); }

	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }

	std::string getGamelistPath(bool forWrite) const;
	bool hasGamelist() const;
	std::string getThemePath() const;

	unsigned int getGameCount() const;
	unsigned int getDisplayedGameCount() const;

	void launchGame(Window* window, FileData* game);

	static void deleteSystems();
	static bool loadConfig(); //Load the system config file at getConfigPath(). Returns true if no errors were encountered. An example will be written if the file doesn't exist.
	static bool IsSystemEnabled(const std::string& path);
	static bool SaveConfig();

	static void writeExampleConfig(const std::string& path);
	static std::string getConfigPath(bool forWrite); // if forWrite, will only return ~/.emulationstation/es_systems.cfg, never /etc/emulationstation/es_systems.cfg

	boost::filesystem::path getRetroArchConfigImportFolder() const;
	boost::filesystem::path getRetroArchSystemConfigFilepath() const;
	

	inline std::vector<SystemData*>::const_iterator getIterator() const { return std::find(sSystemVector.begin(), sSystemVector.end(), this); };
	inline std::vector<SystemData*>::const_reverse_iterator getRevIterator() const { return std::find(sSystemVector.rbegin(), sSystemVector.rend(), this); };

	inline SystemData* getNext() const
	{
		auto it = getIterator();
		it++;
		if(it == sSystemVector.end()) it = sSystemVector.begin();
		return *it;
	}
	SystemData* getNextEnabled() const;
	SystemData* getPrevEnabled() const;

	template<typename IteratorType>
	SystemData* findNextIfEnabled(IteratorType begin, IteratorType end) const
	{
		auto start = std::find(begin, end, this);
		auto isEnabled = [ start ] (SystemData*s) { return *start != s && s->IsEnabled(); };
		auto it = std::find_if(start, end, isEnabled);
		if (it == end)
		{
			it = std::find_if(begin, start, isEnabled);
			if (it == end)
			{
				return *( getIterator() );
			}
		}
		return *it;
	}


	inline SystemData* getPrev() const
	{
		auto it = getRevIterator();
		it++;
		if(it == sSystemVector.rend()) it = sSystemVector.rbegin();
		return *it;
	}

	// Load or re-load theme.
	void loadTheme();

	FileFilterIndex* getIndex() { return mFilterIndex; };

	void SetEnabled(const bool enabled);
	bool IsEnabled() const;

	static SystemData* GetSystemByName(const std::string& name);
	static std::vector<SystemData*> GetSystems();
	static std::vector<SystemData*> GetAllSystems(); //disable ones too

	const GameCollections* GetGameCollections() const;
		  GameCollections* GetGameCollections();

private:

	std::string mName;
	std::string mFullName;
	std::string mStartPath;
	std::vector<std::string> mSearchExtensions;
	std::string mLaunchCommand;
	std::vector<PlatformIds::PlatformId> mPlatformIds;
	std::string mThemeFolder;
	std::shared_ptr<ThemeData> mTheme;

	void populateFolder(FileData* folder);

	FileFilterIndex* mFilterIndex;

	FileData* mRootFolder;

	bool m_enabled;

	static std::vector<SystemData*> sSystemVector;
	std::unique_ptr<GameCollections> m_gameCollections;
};
