#pragma  once
#include "guis/GuiPagedListView.h"

class GuiImportRetroArchConfig : public GuiPagedListView
{
public:
	using CallbackT = std::function<void(boost::filesystem::path)>;
	GuiImportRetroArchConfig(Window* window, const std::string& title, const boost::filesystem::path& configFolder, CallbackT);
	virtual ~GuiImportRetroArchConfig();
	void SetOnButtonPressedCallback(const std::string& button, CallbackT callback);
private:
	// I got some bug/crash calling this from outside to refresh the screen.
	void ReloadConfigs(); //just called once for now during the first loading for now
	const boost::filesystem::path& m_configFolder;
	Window* m_window;
};