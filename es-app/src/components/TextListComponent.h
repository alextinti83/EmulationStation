#pragma once

#include "components/IList.h"
#include "Renderer.h"
#include "resources/Font.h"
#include "InputManager.h"
#include "Sound.h"
#include "Log.h"
#include "ThemeData.h"
#include "Util.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "FileData.h"

struct TextListData
{
	unsigned int colorId;
	unsigned int imageColorId;
	std::shared_ptr<TextCache> textCache;
};

//A graphical list. Supports multiple colors for rows and scrolling.
class TextListComponent : public IList<TextListData, FileData*>
{
protected:
	using BaseT = IList<TextListData, FileData*>;
	using BaseT::mEntries;
	using BaseT::listUpdate;
	using BaseT::listInput;
	using BaseT::listRenderTitleOverlay;
	using BaseT::getTransform;
	using BaseT::mSize;
	using BaseT::mCursor;
	using BaseT::Entry;

public:
	using BaseT::size;
	using BaseT::isScrolling;
	using BaseT::stopScrolling;

	TextListComponent(Window* window);
	
	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

	void add(const std::string& name, FileData* obj, unsigned int colorId, unsigned int imageColor);
	
	enum Alignment
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	inline void setAlignment(Alignment align) { mAlignment = align; }

	inline void setCursorChangedCallback(const std::function<void(CursorState state)>& func) { mCursorChangedCallback = func; }

	inline void setFont(const std::shared_ptr<Font>& font)
	{
		mFont = font;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setUppercase(bool uppercase) 
	{
		mUppercase = true;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setSelectorHeight(float selectorScale) { mSelectorHeight = selectorScale; }
	inline void setSelectorOffsetY(float selectorOffsetY) { mSelectorOffsetY = selectorOffsetY; }
	inline void setSelectorColor(unsigned int color) { mSelectorColor = color; }
	inline void setSelectedColor(unsigned int color) { mSelectedColor = color; }
	inline void setScrollSound(const std::shared_ptr<Sound>& sound) { mScrollSound = sound; }
	inline void setColor(unsigned int id, unsigned int color) { mColors[id] = color; }
	inline void setSound(const std::shared_ptr<Sound>& sound) { mScrollSound = sound; }
	inline void setLineSpacing(float lineSpacing) { mLineSpacing = lineSpacing; }

protected:
	virtual void onScroll(int amt) { if(mScrollSound) mScrollSound->play(); }
	virtual void onCursorChanged(const CursorState& state);

	struct OffsetData
	{
		float horizMargin;
		float gcHorizPos;
		float extraLeftMargin;
		float verticalCenterShift;
	};
	OffsetData ComputeOffsetData(Eigen::Affine3f trans, int i, bool isInAnyGC);
	Eigen::Vector3f ComputeOffset(float y, int i, Eigen::Vector2f textSize, OffsetData& offsetData);
	void RenderGCImage(uint32_t imageColorId, Eigen::Affine3f gcTrans, float gcHorizPos, float verticalShift);
	void RenderText(Eigen::Affine3f trans, Eigen::Vector3f offset, TextCache* textCache);
	void RenderSelectorImage(Eigen::Affine3f trans, int startEntry, int listCutoff);
	void RenderBar(const uint32_t visibleCount);
	uint32_t GetColor(int i, uint32_t defaultColorId) const;
	void BuildTextCache(BaseT::Entry& entry, uint32_t color);
	float GetRowHeight() const;
	uint32_t GetFullyVisibleRowCount() const;
	std::pair<int, int> ComputeListEdgeIndexes() const;

	bool IsInGameCollection(unsigned int entryIndex)
	{
		const Entry& selectedEntry = mEntries.at(entryIndex);
		const FileData& fileData = *selectedEntry.object;
		const bool isCurrent = fileData.isInActiveGameCollection() || fileData.isHighlighted();
		return isCurrent;
	}

	void PushClipRect(const Eigen::Affine3f& trans, float extraLeftMargin = 0.0f)
	{
		// clip to inside margins
		Eigen::Vector3f dim(mSize.x(), mSize.y(), 0);
		dim = trans * dim - trans.translation();
		Renderer::pushClipRect(Eigen::Vector2i(( int ) ( trans.translation().x() + mHorizontalMargin + extraLeftMargin ), ( int ) trans.translation().y()),
			Eigen::Vector2i(( int ) ( dim.x() - mHorizontalMargin * 2 ), ( int ) dim.y()));
	}

private:

	int mMarqueeOffset;
	int mMarqueeWaitTime = 0;
	bool mMarqueeGoBack = false;

	Alignment mAlignment;
	float mHorizontalMargin;
	float mGCImageHorizontalMargin;//game collection image margin

	std::function<void(CursorState state)> mCursorChangedCallback;

	std::shared_ptr<Font> mFont;
	bool mUppercase;
	float mLineSpacing;
	float mSelectorHeight;
	float mSelectorOffsetY;
	unsigned int mSelectorColor;
	unsigned int mSelectedColor;
	std::shared_ptr<Sound> mScrollSound;
	static const unsigned int COLOR_ID_COUNT = 3;
	unsigned int mColors[COLOR_ID_COUNT];

	ImageComponent mSelectorImage;
	ImageComponent m_gameCollectionImage;
	float mGameCollectionImageScale;

	
	struct ScrollBar
	{
		float posX;
		float posY;
		float targetPosY;
		float sizeY;
		float sizeX;
	} mBar;

};
