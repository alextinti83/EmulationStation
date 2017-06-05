#pragma  once
#include "components/ButtonComponent.h"
#include "GuiPagedListView.h"

class GuiImportRetroArchConfig : public GuiPagedListView
{
public:
	using CallbackT = std::function<void(boost::filesystem::path)>;
	GuiImportRetroArchConfig(Window* window, const std::string& title, const boost::filesystem::path& configFolder, CallbackT);
	virtual ~GuiImportRetroArchConfig();

private:
	CallbackT m_onConfigSelected;
};