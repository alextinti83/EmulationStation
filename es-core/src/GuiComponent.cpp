#include "GuiComponent.h"
#include "Window.h"
#include "Log.h"
#include "Renderer.h"
#include "animations/AnimationController.h"
#include "ThemeData.h"

GuiComponent::GuiComponent(Window* window) : mWindow(window), mParent(NULL), mOpacity(255),
	mPosition(Eigen::Vector3f::Zero()), mSize(Eigen::Vector2f::Zero()), mTransform(Eigen::Affine3f::Identity()),
	mIsProcessing(false), mEnabled(true), mVisible(true), mIsPersistent(false), m_context(nullptr)
{
	for(unsigned char i = 0; i < MAX_ANIMATIONS; i++)
		mAnimationMap[i] = NULL;
}

GuiComponent::GuiComponent(gui::Context& context)
	: mWindow(context.GetWindow()), mParent(NULL), mOpacity(255),
	mPosition(Eigen::Vector3f::Zero()), mSize(Eigen::Vector2f::Zero()), mTransform(Eigen::Affine3f::Identity()),
	mIsProcessing(false), mEnabled(true), mVisible(true), mIsPersistent(false), m_context(&context)
{
	for (unsigned char i = 0; i < MAX_ANIMATIONS; i++)
		mAnimationMap[ i ] = NULL;
}

GuiComponent::~GuiComponent()
{
	mWindow->removeGui(this);

	cancelAllAnimations();

	if(mParent)
		mParent->removeChild(this);

	for(unsigned int i = 0; i < getChildCount(); i++)
		getChild(i)->setParent(NULL);
}

bool GuiComponent::input(InputConfig* config, Input input)
{
	if (input.value != 0 && !mBackButton.empty())
	{
		if (config->isMappedTo(mBackButton, input))
		{
			delete this;
			return true;
		}
	}
	for ( GuiComponent* child : mChildren )
	{
		if ( child->input(config, input) )
		{
			return true;
		}
	}
	return false;
}

void GuiComponent::updateSelf(int deltaTime)
{
	for(unsigned char i = 0; i < MAX_ANIMATIONS; i++)
		advanceAnimation(i, deltaTime);
}

void GuiComponent::updateChildren(int deltaTime)
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		getChild(i)->update(deltaTime);
	}
}

void GuiComponent::update(int deltaTime)
{
	if (mVisible)
	{
		updateSelf(deltaTime);
		updateChildren(deltaTime);
	}
}

void GuiComponent::render(const Eigen::Affine3f& parentTrans)
{
	if (mVisible)
	{
		Eigen::Affine3f trans = parentTrans * getTransform();
		renderChildren(trans);
	}
}

void GuiComponent::renderChildren(const Eigen::Affine3f& transform) const
{
	for(unsigned int i = 0; i < getChildCount(); i++)
	{
		getChild(i)->render(transform);
	}
}

Eigen::Vector3f GuiComponent::getPosition() const
{
	return mPosition;
}

void GuiComponent::setPosition(const Eigen::Vector3f& offset)
{
	mPosition = offset;
	onPositionChanged();
}

void GuiComponent::setPosition(float x, float y, float z)
{
	mPosition << x, y, z;
	onPositionChanged();
}

Eigen::Vector2f GuiComponent::getSize() const
{
	return mSize;
}

void GuiComponent::setSize(const Eigen::Vector2f& size)
{
    mSize = size;
    onSizeChanged();
}

void GuiComponent::setSize(float w, float h)
{
	mSize << w, h;
    onSizeChanged();
}

float GuiComponent::getZIndex() const
{
	return mZIndex;
}

void GuiComponent::setZIndex(float z)
{
	mZIndex = z;
}

float GuiComponent::getDefaultZIndex() const
{
	return mDefaultZIndex;
}

void GuiComponent::setDefaultZIndex(float z)
{
	mDefaultZIndex = z;
}

//Children stuff.
void GuiComponent::addChild(GuiComponent* cmp)
{
	mChildren.push_back(cmp);

	if(cmp->getParent())
		cmp->getParent()->removeChild(cmp);

	cmp->setParent(this);
}

void GuiComponent::removeChild(GuiComponent* cmp)
{
	if(!cmp->getParent())
		return;

	if(cmp->getParent() != this)
	{
		LOG(LogError) << "Tried to remove child from incorrect parent!";
	}

	cmp->setParent(NULL);

	for(auto i = mChildren.begin(); i != mChildren.end(); i++)
	{
		if(*i == cmp)
		{
			mChildren.erase(i);
			return;
		}
	}
}

void GuiComponent::clearChildren()
{
	mChildren.clear();
}

void GuiComponent::sortChildren()
{
	std::stable_sort(mChildren.begin(), mChildren.end(),  [](GuiComponent* a, GuiComponent* b) {
		return b->getZIndex() > a->getZIndex();
	});
}

unsigned int GuiComponent::getChildCount() const
{
	return mChildren.size();
}

GuiComponent* GuiComponent::getChild(unsigned int i) const
{
	return mChildren.at(i);
}

void GuiComponent::setParent(GuiComponent* parent)
{
	mParent = parent;
}

GuiComponent* GuiComponent::getParent() const
{
	return mParent;
}

unsigned char GuiComponent::getOpacity() const
{
	return mOpacity;
}

void GuiComponent::setOpacity(unsigned char opacity)
{
	mOpacity = opacity;
	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		(*it)->setOpacity(opacity);
	}
}

const Eigen::Affine3f& GuiComponent::getTransform()
{
	mTransform.setIdentity();
	mTransform.translate(mPosition);
	return mTransform;
}

void GuiComponent::setValue(const std::string& value)
{
}

std::string GuiComponent::getValue() const
{
	return "";
}

void GuiComponent::textInput(const char* text)
{
	for(auto iter = mChildren.begin(); iter != mChildren.end(); iter++)
	{
		(*iter)->textInput(text);
	}
}

void GuiComponent::setAnimation(Animation* anim, int delay, std::function<void()> finishedCallback, bool reverse, unsigned char slot)
{
	assert(slot < MAX_ANIMATIONS);

	AnimationController* oldAnim = mAnimationMap[slot];
	mAnimationMap[slot] = new AnimationController(anim, delay, finishedCallback, reverse);

	if(oldAnim)
		delete oldAnim;
}

bool GuiComponent::stopAnimation(unsigned char slot)
{
	assert(slot < MAX_ANIMATIONS);
	if(mAnimationMap[slot])
	{
		delete mAnimationMap[slot];
		mAnimationMap[slot] = NULL;
		return true;
	}else{
		return false;
	}
}

bool GuiComponent::cancelAnimation(unsigned char slot)
{
	assert(slot < MAX_ANIMATIONS);
	if(mAnimationMap[slot])
	{
		mAnimationMap[slot]->removeFinishedCallback();
		delete mAnimationMap[slot];
		mAnimationMap[slot] = NULL;
		return true;
	}else{
		return false;
	}
}

bool GuiComponent::finishAnimation(unsigned char slot)
{
	assert(slot < MAX_ANIMATIONS);
	if(mAnimationMap[slot])
	{
		// skip to animation's end
		const bool done = mAnimationMap[slot]->update(mAnimationMap[slot]->getAnimation()->getDuration() - mAnimationMap[slot]->getTime());
		assert(done);

		delete mAnimationMap[slot]; // will also call finishedCallback
		mAnimationMap[slot] = NULL;
		return true;
	}else{
		return false;
	}
}

bool GuiComponent::advanceAnimation(unsigned char slot, unsigned int time)
{
	assert(slot < MAX_ANIMATIONS);
	AnimationController* anim = mAnimationMap[slot];
	if(anim)
	{
		bool done = anim->update(time);
		if(done)
		{
			mAnimationMap[slot] = NULL;
			delete anim;
		}
		return true;
	}else{
		return false;
	}
}

void GuiComponent::stopAllAnimations()
{
	for(unsigned char i = 0; i < MAX_ANIMATIONS; i++)
		stopAnimation(i);
}

void GuiComponent::cancelAllAnimations()
{
	for(unsigned char i = 0; i < MAX_ANIMATIONS; i++)
		cancelAnimation(i);
}

bool GuiComponent::isAnimationPlaying(unsigned char slot) const
{
	return mAnimationMap[slot] != NULL;
}

bool GuiComponent::isAnimationReversed(unsigned char slot) const
{
	assert(mAnimationMap[slot] != NULL);
	return mAnimationMap[slot]->isReversed();
}

int GuiComponent::getAnimationTime(unsigned char slot) const
{
	assert(mAnimationMap[slot] != NULL);
	return mAnimationMap[slot]->getTime();
}

void GuiComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	Eigen::Vector2f scale = getParent() ? getParent()->getSize() : Eigen::Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "");
	if(!elem)
		return;

	using namespace ThemeFlags;
	if(properties & POSITION && elem->has("pos"))
	{
		Eigen::Vector2f denormalized = elem->get<Eigen::Vector2f>("pos").cwiseProduct(scale);
		setPosition(Eigen::Vector3f(denormalized.x(), denormalized.y(), 0));
	}

	if(properties & ThemeFlags::SIZE && elem->has("size"))
		setSize(elem->get<Eigen::Vector2f>("size").cwiseProduct(scale));

	if(properties & ThemeFlags::Z_INDEX && elem->has("zIndex"))
		setZIndex(elem->get<float>("zIndex"));
	else
		setZIndex(getDefaultZIndex());
}

void GuiComponent::updateHelpPrompts()
{
	if(getParent())
	{
		getParent()->updateHelpPrompts();
		return;
	}

	std::vector<HelpPrompt> prompts = getHelpPrompts();
	if (!mBackButton.empty())
	{
		prompts.push_back(HelpPrompt(mBackButton, "BACK"));
	}

	if(mWindow->peekGui() == this)
		mWindow->setHelpPrompts(prompts, getHelpStyle());
}

HelpStyle GuiComponent::getHelpStyle()
{
	return HelpStyle();
}

bool GuiComponent::isProcessing() const
{
	return mIsProcessing;
}

void GuiComponent::setEnabled(bool enabled)
{
	if (mEnabled != enabled)
	{
		const bool oldValue = mEnabled;
		mEnabled = enabled;
		OnEnabledChanged(oldValue, mEnabled);
	}
}

void GuiComponent::setVisible(bool visible)
{
	if (mVisible != visible)
	{
		const bool oldValue = mVisible;
		mVisible = visible;
		OnVisibleChanged(oldValue, mVisible);
	}
}

void GuiComponent::setIsPersistent(bool i_isPersistent)
{
	mIsPersistent = i_isPersistent;
}

void GuiComponent::onShow()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
		getChild(i)->onShow();
}

void GuiComponent::onHide()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
		getChild(i)->onHide();
}

void GuiComponent::onScreenSaverActivate()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
		getChild(i)->onScreenSaverActivate();
}

void GuiComponent::onScreenSaverDeactivate()
{
	for(unsigned int i = 0; i < getChildCount(); i++)
		getChild(i)->onScreenSaverDeactivate();
}

void GuiComponent::topWindow(bool isTop)
{
	for(unsigned int i = 0; i < getChildCount(); i++)
		getChild(i)->topWindow(isTop);
}

void GuiComponent::SetBackButton(const std::string& buttonName)
{
	mBackButton = buttonName;
}

bool GuiComponent::UpdateFocus(FocusPosition position, bool enableFocus)
{
	bool focusFound = false;
	for (GuiComponent* child : mChildren)
	{
		if (child->UpdateFocus(position, enableFocus && !focusFound))
		{
			focusFound = true;
		}
	}
	return focusFound;
}

