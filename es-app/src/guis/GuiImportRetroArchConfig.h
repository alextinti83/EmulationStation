#pragma  once
#include "GuiPagedListView.h"

class GuiImportRetroArchConfig : public GuiPagedListView
{
public:
	using CallbackT = std::function<void(boost::filesystem::path)>;
	GuiImportRetroArchConfig(Window* window, const std::string& title, const boost::filesystem::path& configFolder, CallbackT);
	virtual ~GuiImportRetroArchConfig();
	void SetOnButtonPressedCallback(const std::string& button, CallbackT callback);

};