#pragma once

#include "GuiComponent.h"
#include <functional>
#include "resources/Font.h"
#include "components/NinePatchComponent.h"

class ButtonComponent : public GuiComponent
{
public:
	ButtonComponent(Window* window, const std::string& text = "", const std::string& helpText = "", const std::function<void()>& func = nullptr);

	void setPressedFunc(std::function<void()> f);

	bool input(InputConfig* config, Input input) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void setText(const std::string& text, const std::string& helpText);

	inline const std::string& getText() const { return mText; };
	inline const std::function<void()>& getPressedFunc() const { return mPressedFunc; };

	void onSizeChanged() override;
	void onFocusGained() override;
	void onFocusLost() override;

	void setColorShift(unsigned int color) { mModdedColor = color; mNewColor = true; updateImage(); }
	void removeColorShift() { mNewColor = false; updateImage(); }

	virtual std::vector<HelpPrompt> getHelpPrompts() override;
protected: 
	void OnEnabledChanged(bool, bool) override;

private:
	std::shared_ptr<Font> mFont;
	std::function<void()> mPressedFunc;

	bool mFocused;

	unsigned int mTextColorFocused;
	unsigned int mTextColorUnfocused;
	unsigned int mTextColorDisabled;

	
	bool mNewColor = false;
	unsigned int mModdedColor;
	unsigned int mBgColorDisabled;

	unsigned int getCurTextColor() const;
	void updateImage();

	std::string mText;
	std::string mHelpText;
	std::unique_ptr<TextCache> mTextCache;
	NinePatchComponent mBox;

};
