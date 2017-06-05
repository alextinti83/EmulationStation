#include "components/MenuComponent.h"
#include "components/ButtonComponent.h"

#define BUTTON_GRID_VERT_PADDING 32
#define BUTTON_GRID_HORIZ_PADDING 10

#define TITLE_HEIGHT (mTitle->getFont()->getLetterHeight() + TITLE_VERT_PADDING)

using namespace Eigen;

MenuComponent::MenuComponent(Window* window, const char* title, const std::shared_ptr<Font>& titleFont) : GuiComponent(window), 
	mBackground(window), mGrid(window, Vector2i(1, 3))
{	
	addChild(&mBackground);
	addChild(&mGrid);

	mBackground.setImagePath(":/frame.png");

	// set up title
	mTitle = std::make_shared<TextComponent>(mWindow);
	mTitle->setAlignment(ALIGN_CENTER);
	mTitle->setColor(0x555555FF);
	setTitle(title, titleFont);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false);

	// set up list which will never change (externally, anyway)
	mList = std::make_shared<ComponentList>(mWindow);
	mGrid.setEntry(mList, Vector2i(0, 1), true);

	mSize[ 0 ] = Renderer::getScreenWidth() * 0.5f;
	updateGrid();
	updateSize();

	mGrid.resetCursor();
}

void MenuComponent::setTitle(const char* title, const std::shared_ptr<Font>& font)
{
	mTitle->setText(strToUpper(title));
	mTitle->setFont(font);
}

float MenuComponent::getButtonGridHeight() const
{
	return (mButtonGrid ? mButtonGrid->getSize().y() : Font::get(FONT_SIZE_MEDIUM)->getHeight() + BUTTON_GRID_VERT_PADDING);
}

void MenuComponent::updateSize()
{
	const float maxHeight = Renderer::getScreenHeight() * 0.7f;
	float height = TITLE_HEIGHT + mList->getTotalRowHeight() + getButtonGridHeight() + 2;
	if(height > maxHeight)
	{
		height = TITLE_HEIGHT + getButtonGridHeight();
		int i = 0;
		while(i < mList->size())
		{
			float rowHeight = mList->getRowHeight(i);
			if(height + rowHeight < maxHeight)
				height += rowHeight;
			else
				break;
			i++;
		}
	}

	setSize(getSize().x(), height);
}

void MenuComponent::onSizeChanged()
{
	mBackground.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-32, -32));

	// update grid row/col sizes
	mGrid.setRowHeightPerc(0, TITLE_HEIGHT / mSize.y());
	mGrid.setRowHeightPerc(2, getButtonGridHeight() / mSize.y());
	
	mGrid.setSize(mSize);
}

std::shared_ptr<ButtonComponent> MenuComponent::addButton(const std::string& name, const std::string& helpText, const std::function<void()>& callback)
{
	std::shared_ptr<ButtonComponent> button = std::make_shared<ButtonComponent>(mWindow, strToUpper(name), helpText, callback);
	mButtons.push_back(button);
	updateGrid();
	updateSize();
	return button;
}

void MenuComponent::updateGrid()
{
	if(mButtonGrid)
		mGrid.removeEntry(mButtonGrid);

	mButtonGrid.reset();

	if(mButtons.size())
	{
		mButtonGrid = makeButtonGrid(mWindow, mButtons);
		mGrid.setEntry(mButtonGrid, Vector2i(0, 2), true, false);
	}
}

std::vector<HelpPrompt> MenuComponent::getHelpPrompts()
{
	return mGrid.getHelpPrompts();
}

std::shared_ptr<ComponentGrid> makeButtonGrid(Window* window, const std::vector< std::shared_ptr<ButtonComponent> >& buttons)
{
	std::shared_ptr<ComponentGrid> buttonGrid = std::make_shared<ComponentGrid>(window, Vector2i(buttons.size(), 2));

	float buttonGridWidth = (float)BUTTON_GRID_HORIZ_PADDING * buttons.size(); // initialize to padding
	for(int i = 0; i < (int)buttons.size(); i++)
	{
		buttonGrid->setEntry(buttons.at(i), Vector2i(i, 0), true, false);
		buttonGridWidth += buttons.at(i)->getSize().x();
	}
	for(unsigned int i = 0; i < buttons.size(); i++)
	{
		buttonGrid->setColWidthPerc(i, (buttons.at(i)->getSize().x() + BUTTON_GRID_HORIZ_PADDING) / buttonGridWidth);
	}
	
	buttonGrid->setSize(buttonGridWidth, buttons.at(0)->getSize().y() + BUTTON_GRID_VERT_PADDING + 2);
	buttonGrid->setRowHeightPerc(1, 2 / buttonGrid->getSize().y()); // spacer row to deal with dropshadow to make buttons look centered

	return buttonGrid;
}

std::shared_ptr<ImageComponent> makeArrow(Window* window)
{
	auto bracket = std::make_shared<ImageComponent>(window);
	bracket->setImage(":/arrow.svg");
	bracket->setResize(0, round(Font::get(FONT_SIZE_MEDIUM)->getLetterHeight()));
	return bracket;
}
