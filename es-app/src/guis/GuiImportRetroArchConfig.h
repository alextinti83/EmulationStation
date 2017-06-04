#pragma  once
#include "guis/GuiOptionWindow.h"

class GuiImportRetroArchConfig : public GuiOptionWindow
{
public:
	using CallbackT = std::function<void(boost::filesystem::path)>;
	GuiImportRetroArchConfig(Window* window, const std::string& title, const boost::filesystem::path& configFolder, CallbackT);
	virtual ~GuiImportRetroArchConfig();

private:
	bool OnRowSelected(InputConfig* config, Input input,  boost::filesystem::path);
	boost::filesystem::path m_configFolder;
	CallbackT m_onConfigSelected;
};