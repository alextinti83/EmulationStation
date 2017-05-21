#include "TextListComponent.h"

#ifdef WIN32
const float TextListComponent::k_favoriteImageScale = 0.7f;
#else
const float TextListComponent::k_favoriteImageScale = 1.0f;
#endif

TextListComponent::TextListComponent(Window* window) :
	BaseT(window),
	m_favoriteImage(window)
{

	m_favoriteImage.setImage(":/star_filled.svg");

	mMarqueeOffset = 0;

	mHorizontalMargin = 0;
	mAlignment = ALIGN_CENTER;

	mFont = Font::get(FONT_SIZE_MEDIUM);
	mUppercase = false;
	mLineSpacing = 1.5f;
	mSelectorColor = 0x000000FF;
	mSelectedColor = 0;
	mColors[ 0 ] = 0x0000FFFF;
	mColors[ 1 ] = 0x00FF00FF;
	mColors[ 2 ] = 0xFFFF00FF;

}


void TextListComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	std::shared_ptr<Font>& font = mFont;

	if ( size() == 0 )
		return;

	const float entrySize = round(font->getHeight(mLineSpacing));

	int startEntry = 0;

	//number of entries that can fit on the screen simultaniously
	int screenCount = ( int ) ( mSize.y() / entrySize + 0.5f );

	if ( size() >= screenCount )
	{
		startEntry = mCursor - screenCount / 2;
		if ( startEntry < 0 )
			startEntry = 0;
		if ( startEntry >= size() - screenCount )
			startEntry = size() - screenCount;
	}

	float y = 0;

	int listCutoff = startEntry + screenCount;
	if ( listCutoff > size() )
		listCutoff = size();
	const float fontHeight = font->getHeight();
	// draw selector bar
	if ( startEntry < listCutoff )
	{
		Renderer::setMatrix(trans);
		//BaseT::Entry& entry = mEntries.at(( unsigned int ) mCursor);
		//unsigned int color = mColors[ entry.data.colorId ];
		Renderer::drawRect(0.f, ( mCursor - startEntry )*entrySize + ( entrySize - fontHeight ) / 2, mSize.x(), fontHeight, mSelectorColor);
	}

	PushClipRect(trans);

	for ( int i = startEntry; i < listCutoff; i++ )
	{
		BaseT::Entry& entry = mEntries.at(( unsigned int ) i);

		unsigned int color;
		if ( mCursor == i && mSelectedColor )
			color = mSelectedColor;
		else
			color = mColors[ entry.data.colorId ];

		if ( !entry.data.textCache )
			entry.data.textCache = std::unique_ptr<TextCache>(font->buildTextCache(mUppercase ? strToUpper(entry.name) : entry.name, 0, 0, 0x000000FF));

		entry.data.textCache->setColor(color);

		Eigen::Vector3f offset(0, y, 0);

#define DRAW_FAVORITE 1
#if	DRAW_FAVORITE
		float horizMargin = mHorizontalMargin;
		float verticalCenterShift;
		const bool isFavorite = IsFavorite(i);
		if ( isFavorite )
		{
			const Eigen::Vector2f favImageSize = m_favoriteImage.getSize() * k_favoriteImageScale;
			const float favHeight = favImageSize.y();
			verticalCenterShift = ( fontHeight - favHeight ) * 0.5f;
			const float extraLeftMargin = favImageSize.x();
			PushClipRect(trans, extraLeftMargin);
			horizMargin += extraLeftMargin;
		}
#else
		const float horizMargin = mHorizontalMargin;
#endif

		switch ( mAlignment )
		{
		case ALIGN_LEFT:
			offset[ 0 ] = horizMargin;
			break;
		case ALIGN_CENTER:
			offset[ 0 ] = ( mSize.x() - entry.data.textCache->metrics.size.x() ) / 2;
			if ( offset[ 0 ] < 0 )
				offset[ 0 ] = 0;
			break;
		case ALIGN_RIGHT:
			offset[ 0 ] = ( mSize.x() - entry.data.textCache->metrics.size.x() );
			offset[ 0 ] -= horizMargin;
			if ( offset[ 0 ] < 0 )
				offset[ 0 ] = 0;
			break;
		}

		if ( mCursor == i )
			offset[ 0 ] += mMarqueeOffset;

		Eigen::Affine3f textTrans = trans;
		textTrans.translate(offset);
		Renderer::setMatrix(textTrans);

		font->renderTextCache(entry.data.textCache.get());

#if DRAW_FAVORITE
		if ( isFavorite )
		{
			m_favoriteImage.setColorShift(mColors[ entry.data.imageColorId]);
			Eigen::Affine3f favTrans = trans;
			Eigen::Vector3f favOffset(mHorizontalMargin, y + verticalCenterShift, 0);
			favTrans.translate(favOffset);
			{
				const float scale = k_favoriteImageScale;
				favTrans.scale(Eigen::Vector3f(scale, scale, scale));
			}
			Renderer::popClipRect(); //pop extra margin
			m_favoriteImage.render(favTrans);
		}
#endif
		y += entrySize;
	}

	Renderer::popClipRect();

	listRenderTitleOverlay(trans);

	GuiComponent::renderChildren(trans);
}


bool TextListComponent::input(InputConfig* config, Input input)
{
	if ( size() > 0 )
	{
		if ( input.value != 0 )
		{
			if ( config->isMappedTo("down", input) )
			{
				listInput(1);
				return true;
			}

			if ( config->isMappedTo("up", input) )
			{
				listInput(-1);
				return true;
			}
			if ( config->isMappedTo("pagedown", input) )
			{
				listInput(10);
				return true;
			}

			if ( config->isMappedTo("pageup", input) )
			{
				listInput(-10);
				return true;
			}
		}
		else
		{
			if ( config->isMappedTo("down", input) || config->isMappedTo("up", input) ||
				config->isMappedTo("pagedown", input) || config->isMappedTo("pageup", input) )
			{
				stopScrolling();
			}
		}
	}

	return GuiComponent::input(config, input);
}


void TextListComponent::update(int deltaTime)
{
	listUpdate(deltaTime);
	if ( !isScrolling() && size() > 0 )
	{
		static const int k_maxWaitTime = 1000; //ms
		static const int k_marqueeDeltaShift = 1;

		const Entry& selectedEntry = mEntries.at(( unsigned int ) mCursor);

		const bool hasFavorites = true;
		float extraLeftMargin = 0;
		if ( hasFavorites )
		{
			extraLeftMargin = m_favoriteImage.getSize().x() * k_favoriteImageScale;
		}

		const std::string& text = selectedEntry.name;
		const Eigen::Vector2f textSize = mFont->sizeText(text);

		const float textBoxSize = mSize.x() - mHorizontalMargin * 2 - extraLeftMargin;
		const float exceedingTextSize = textSize.x() - textBoxSize;
		const float extraRightMargin = 5;
		if ( exceedingTextSize > 0 )
		{
			if ( mMarqueeWaitTime > k_maxWaitTime )
			{
				if ( mMarqueeGoBack )
				{
					if ( mMarqueeOffset < 0 )
					{
						mMarqueeOffset += k_marqueeDeltaShift;
					}
					else
					{
						mMarqueeWaitTime = 0;
						mMarqueeGoBack = false;
					}
				}
				else
				{
					if ( exceedingTextSize + extraRightMargin + mMarqueeOffset > 0 )
					{
						mMarqueeOffset -= k_marqueeDeltaShift;
					}
					else
					{
						mMarqueeWaitTime = 0;
						mMarqueeGoBack = true;
					}
				}
			}
			else
			{
				mMarqueeWaitTime += deltaTime;
			}
		}
	}

	GuiComponent::update(deltaTime);
}

//list management stuff
void TextListComponent::add(
	const std::string& name, 
	FileData* obj,
	unsigned int color,
	unsigned int imageColor)
{
	assert(color < COLOR_ID_COUNT);

	BaseT::Entry entry;
	entry.name = name;
	entry.object = obj;
	entry.data.colorId = color;
	entry.data.imageColorId = imageColor;

	BaseT::add(entry);
}

void TextListComponent::onCursorChanged(const CursorState& state)
{
	mMarqueeOffset = 0;
	mMarqueeWaitTime = 0;
	if ( mCursorChangedCallback )
		mCursorChangedCallback(state);
}

void TextListComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "textlist");
	if ( !elem )
		return;

	using namespace ThemeFlags;
	if ( properties & COLOR )
	{
		if ( elem->has("selectorColor") )
			setSelectorColor(elem->get<unsigned int>("selectorColor"));
		if ( elem->has("selectedColor") )
			setSelectedColor(elem->get<unsigned int>("selectedColor"));
		if ( elem->has("primaryColor") )
			setColor(0, elem->get<unsigned int>("primaryColor"));
		if ( elem->has("secondaryColor") )
			setColor(1, elem->get<unsigned int>("secondaryColor"));
	}

	setFont(Font::getFromTheme(elem, properties, mFont));

	if ( properties & SOUND && elem->has("scrollSound") )
		setSound(Sound::get(elem->get<std::string>("scrollSound")));

	if ( properties & ALIGNMENT )
	{
		if ( elem->has("alignment") )
		{
			const std::string& str = elem->get<std::string>("alignment");
			if ( str == "left" )
				setAlignment(ALIGN_LEFT);
			else if ( str == "center" )
				setAlignment(ALIGN_CENTER);
			else if ( str == "right" )
				setAlignment(ALIGN_RIGHT);
			else
				LOG(LogError) << "Unknown TextListComponent alignment \"" << str << "\"!";
		}
		if ( elem->has("horizontalMargin") )
		{
			mHorizontalMargin = elem->get<float>("horizontalMargin") * ( this->mParent ? this->mParent->getSize().x() : ( float ) Renderer::getScreenWidth() );
		}
	}

	if ( properties & FORCE_UPPERCASE && elem->has("forceUppercase") )
		setUppercase(elem->get<bool>("forceUppercase"));

	if ( properties & LINE_SPACING && elem->has("lineSpacing") )
		setLineSpacing(elem->get<float>("lineSpacing"));
}
