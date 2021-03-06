#include "FileData.h"
#include "SystemData.h"
#include "GameCollections.h"

namespace fs = boost::filesystem;

std::string removeParenthesis(const std::string& str)
{
	// remove anything in parenthesis or brackets
	// should be roughly equivalent to the regex replace "\((.*)\)|\[(.*)\]" with ""
	// I would love to just use regex, but it's not worth pulling in another boost lib for one function that is used once

	std::string ret = str;
	size_t start, end;

	static const int NUM_TO_REPLACE = 2;
	static const char toReplace[NUM_TO_REPLACE*2] = { '(', ')', '[', ']' };

	bool done = false;
	while(!done)
	{
		done = true;
		for(int i = 0; i < NUM_TO_REPLACE; i++)
		{
			end = ret.find_first_of(toReplace[i*2+1]);
			start = ret.find_last_of(toReplace[i*2], end);

			if(start != std::string::npos && end != std::string::npos)
			{
				ret.erase(start, end - start + 1);
				done = false;
			}
		}
	}

	// also strip whitespace
	end = ret.find_last_not_of(' ');
	if(end != std::string::npos)
		end++;

	ret = ret.substr(0, end);

	return ret;
}


FileData::FileData(FileType type, const fs::path& path, SystemData* system)
	: mType(type)
	, mPath(path)
	, mSystem(system)
	, mParent(NULL)
	, metadata(type == GAME ? GAME_METADATA : FOLDER_METADATA)
{
	// metadata needs at least a name field (since that's what getName() will return)
	if ( metadata.get("name").empty() )
	{
		metadata.set("name", getDisplayName());
	}

	importLegacyFavoriteTag();
}



FileData::~FileData()
{
	if ( mParent )
	{
		mParent->removeChild(this);
	}
	mChildren.clear();
}

std::unique_ptr<FileData> FileData::Clone() const
{
	std::unique_ptr<FileData> clone = std::unique_ptr<FileData>(new FileData(mType, mPath, mSystem));
	clone->mRelativePath = mRelativePath;
	clone->mParent = nullptr;

	for (auto& child : mChildren)
	{
		clone->addChild(std::move(child->Clone()));
	}
	// Let's forget about filtered ones for now..
	return std::move(clone);
}

void FileData::addChild(std::unique_ptr<FileData> child)
{
	std::unique_ptr<FileData> childClone = child->Clone();
	addChild(childClone.get());
	childClone.release();
}

std::string FileData::getDisplayName() const
{
	std::string stem = mPath.stem().generic_string();
	if(mSystem && mSystem->hasPlatformId(PlatformIds::ARCADE) || mSystem->hasPlatformId(PlatformIds::NEOGEO))
		stem = PlatformIds::getCleanMameName(stem.c_str());

	return stem;
}

std::string FileData::getCleanName() const
{
	return removeParenthesis(this->getDisplayName());
}

const std::string& FileData::getThumbnailPath() const
{
	if(!metadata.get("thumbnail").empty())
		return metadata.get("thumbnail");
	else
		return metadata.get("image");
}

GameCollection::Tag FileData::GetActiveGameCollectionTag() const
{
	if (mSystem)
	{
		const GameCollection* gc = mSystem->GetGameCollections()->GetActiveGameCollection();
		return gc->GetTag();
	}
	return GameCollection::Tag::None;
}

void FileData::AddToActiveGameCollection(bool isFavorite)
{
	if ( mSystem )
	{
		GameCollections* gc = mSystem->GetGameCollections();
		if (isFavorite)
		{
			if (gc) { gc->AddToActiveGameCollection(*this); }
		}
		else
		{
			if (gc) { gc->RemoveFromActiveGameCollection(*this); }
		}
	}
}

void FileData::SetMetadata(const MetaDataList& i_metadata)
{
	std::string defaultName = metadata.get("name");
	metadata = i_metadata;
	if ( metadata.get("name").empty() )
	{
		metadata.set("name", defaultName);
	}
	metadata.resetChangedFlag();

	importLegacyFavoriteTag();
}

const std::vector<FileData*>& FileData::getChildrenListToDisplay()
{
	
	FileFilterIndex* idx = mSystem->getIndex();
	if (idx->isFiltered()) {
		mFilteredChildren.clear();
		for(auto it = mChildren.begin(); it != mChildren.end(); it++)
		{
			if (idx->showFile((*it))) {
				mFilteredChildren.push_back(*it);
			}
		}

		return mFilteredChildren;
	}
	else 
	{
		return mChildren;
	}
}

const std::string& FileData::getVideoPath() const
{
	return metadata.get("video");
}

const std::string& FileData::getMarqueePath() const
{
	return metadata.get("marquee");
}

bool FileData::isInActiveGameCollection() const
{
	const GameCollections* gc = mSystem->GetGameCollections();
	if (gc) { return gc->IsInActivetGameCollection(*this); }
	return false;
}

bool FileData::isHighlighted() const
{
	const GameCollections* gc = mSystem->GetGameCollections();
	if (gc) { return gc->HasTag(*this, GameCollection::Tag::Highlight); }
	return false;
}

bool FileData::isHidden() const
{
	const GameCollections* gc = mSystem->GetGameCollections();
	if (gc) { return gc->HasTag(*this, GameCollection::Tag::Hide); }
	return false;
}

std::vector<FileData*> FileData::getFilesRecursive(unsigned int typeMask, bool displayedOnly) const
{
	std::vector<FileData*> out;
	FileFilterIndex* idx = mSystem->getIndex();

	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if((*it)->getType() & typeMask)
		{
			if (!displayedOnly || !idx->isFiltered() || idx->showFile(*it))
				out.push_back(*it);
		}
		
		if((*it)->getChildren().size() > 0)
		{
			std::vector<FileData*> subchildren = (*it)->getFilesRecursive(typeMask, displayedOnly);
			out.insert(out.end(), subchildren.cbegin(), subchildren.cend());
		}
	}

	return out;
}

void FileData::addChild(FileData* file)
{
	assert(mType == FOLDER);
	assert(file->getParent() == NULL);

	const std::string key = file->getPath().filename().string();
	if (mChildrenByFilename.find(key) == mChildrenByFilename.end())
	{
		mChildrenByFilename[key] = file;
		mChildren.push_back(file);
		file->mParent = this;
	}
}

void FileData::removeChild(FileData* file)
{
	assert(mType == FOLDER);
	assert(file->getParent() == this);
	mChildrenByFilename.erase(file->getPath().filename().string());
	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if(*it == file)
		{
			mChildren.erase(it);
			return;
		}
	}

	// File somehow wasn't in our children.
	assert(false);

}

void FileData::sort(ComparisonFunction& comparator, bool ascending)
{
	std::sort(mChildren.begin(), mChildren.end(), comparator);

	for(auto it = mChildren.begin(); it != mChildren.end(); it++)
	{
		if((*it)->getChildren().size() > 0)
			(*it)->sort(comparator, ascending);
	}

	if(!ascending)
		std::reverse(mChildren.begin(), mChildren.end());
}

void FileData::sort(const SortType& type)
{
	sort(*type.comparisonFunction, type.ascending);
}

void FileData::importLegacyFavoriteTag()
{
	if ( metadata.get("favorite").compare("true") == 0 )
	{
		AddToActiveGameCollection(true);
		metadata.erase("favorite");
	}
}
