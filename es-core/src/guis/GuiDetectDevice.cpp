#include "guis/GuiDetectDevice.h"
#include "Window.h"
#include "Renderer.h"
#include "resources/Font.h"
#include "guis/GuiInputConfig.h"
#include "components/TextComponent.h"
#include <iostream>
#include <string>
#include <sstream>
#include "Util.h"
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include "GuiPagedListView.h"
#include "Log.h"

#define HOLD_TIME 1000
#define PRESS_TIME 300

using namespace Eigen;

namespace fs = boost::filesystem;

GuiDetectDevice::GuiDetectDevice(Window* window, bool firstRun, const std::function<void()>& doneCallback) : GuiComponent(window), mFirstRun(firstRun), 
	mBackground(window, ":/frame.png"), mGrid(window, Vector2i(1, 7)), m_pressCount(0u)
{
	mHoldingConfig = NULL;
	mHoldTime = 0;
	mPressTime = 0;
	mDoneCallback = doneCallback;

	addChild(&mBackground);
	addChild(&mGrid);
	
	// title
	mTitle = std::make_shared<TextComponent>(mWindow, firstRun ? "WELCOME" : "CONFIGURE INPUT", 
		Font::get(FONT_SIZE_LARGE), 0x555555FF, ALIGN_CENTER);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true, Vector2i(1, 1), GridFlags::BORDER_BOTTOM);

	// device info
	mDeviceInfo = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x999999FF, ALIGN_CENTER);
	mGrid.setEntry(mDeviceInfo, Vector2i(0, 1), false, true);
	UpdateDeviceInfo();
	
	// message
	mMsg1 = std::make_shared<TextComponent>(mWindow, "HOLD A BUTTON ON YOUR DEVICE TO CONFIGURE IT.", Font::get(FONT_SIZE_SMALL), 0x777777FF, ALIGN_CENTER);
	mGrid.setEntry(mMsg1, Vector2i(0, 2), false, true);


	mMsg2 = std::make_shared<TextComponent>(mWindow, "TO QUIT THIS SCREEN PRESS ESC", Font::get(FONT_SIZE_SMALL), 0x777777FF, ALIGN_CENTER);
	mGrid.setEntry(mMsg2, Vector2i(0, 3), false, true);

	mMsg2 = std::make_shared<TextComponent>(mWindow, "OR QUICKLY PRESS ANY BUTTON 3 TIMES IN A ROW", Font::get(FONT_SIZE_SMALL), 0x777777FF, ALIGN_CENTER);
	mGrid.setEntry(mMsg2, Vector2i(0, 4), false, true);


	mMsg2 = std::make_shared<TextComponent>(mWindow,  "PRESS F4 TO QUIT EMULATION STATION AT ANY TIME", Font::get(FONT_SIZE_SMALL), 0x777777FF, ALIGN_CENTER);
	mGrid.setEntry(mMsg2, Vector2i(0, 5), false, true);

	// currently held device
	mDeviceHeld = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_MEDIUM), 0xFFFFFFFF, ALIGN_CENTER);
	mGrid.setEntry(mDeviceHeld, Vector2i(0, 6), false, true);

	setSize(Renderer::getScreenWidth() * 0.6f, Renderer::getScreenHeight() * 0.6f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

void GuiDetectDevice::UpdateDeviceInfo()
{
	std::stringstream deviceInfo;
	const uint32_t numDevices = InputManager::getInstance()->getNumJoysticks();

	if (numDevices > 0)
	{
		deviceInfo << numDevices << " GAMEPAD" << ( numDevices > 1 ? "S" : "" ) << " DETECTED";
	}
	else
	{
		deviceInfo << "NO GAMEPADS DETECTED";
	}

	mDeviceInfo->setText(deviceInfo.str());
}

void GuiDetectDevice::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	// grid
	mGrid.setSize(mSize);
	mGrid.setRowHeightPerc(0, mTitle->getFont()->getHeight() / mSize.y());
	//mGrid.setRowHeightPerc(1, mDeviceInfo->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(2, mMsg1->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(3, mMsg2->getFont()->getHeight() / mSize.y());
	//mGrid.setRowHeightPerc(4, mDeviceHeld->getFont()->getHeight() / mSize.y());
}

bool GuiDetectDevice::input(InputConfig* config, Input input)
{
	if(!mFirstRun && input.device == DEVICE_KEYBOARD && input.type == TYPE_KEY && input.value && input.id == SDLK_ESCAPE)
	{
		// cancel configuring
		delete this;
		return true;
	}


	if(input.type == TYPE_BUTTON || input.type == TYPE_KEY)
	{
		
		if(input.value && mHoldingConfig == NULL)
		{
			// started holding
			mHoldingConfig = config;
			mHoldTime = HOLD_TIME;
			mDeviceHeld->setText(strToUpper(config->getDeviceName()));

			m_pressCount++;
			mPressTime = PRESS_TIME;
		}else if(!input.value && mHoldingConfig == config)
		{
			// cancel
			mHoldingConfig = NULL;
			mDeviceHeld->setText("");
		}

		if (m_pressCount == 3)
		{
			delete this;
			return true;
		}
	}

	return true;
}

void GuiDetectDevice::update(int deltaTime)
{
	mPressTime -= deltaTime;
	if (mPressTime <=0 )
	{
		m_pressCount = 0;
	}

	UpdateDeviceInfo();
	if(mHoldingConfig)
	{
		// If ES starts and if a known device is connected after startup skip controller configuration
		if(mFirstRun && fs::exists(InputManager::getConfigPath()) && InputManager::getInstance()->getNumConfiguredDevices() > 0)
		{
			if(mDoneCallback)
				mDoneCallback();
			delete this; // delete GUI element
		}
		else
		{
			mHoldTime -= deltaTime;
			const float t = (float)mHoldTime / HOLD_TIME;
			unsigned int c = (unsigned char)(t * 255);
			mDeviceHeld->setColor((c << 24) | (c << 16) | (c << 8) | 0xFF);
			if(mHoldTime <= 0)
			{
				mDeviceHeld->setText("");

					// picked one!
				mWindow->pushGui(new GuiInputConfig(mWindow, mHoldingConfig, true, mDoneCallback));
				delete this;
				mHoldingConfig = nullptr;
			}
		}
	}
}
