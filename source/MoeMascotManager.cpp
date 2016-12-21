// -*- c++ -*-
//
//  'Moe' window sitter for BeOS.
//  Copyright (C) 2001
//  Okada Jun (yun@be-in.org)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Moe is window sitter running on BeOS Intel and PPC,
//  originally designed and developed by Okada Jun in 2001.
//  http://www.be-in.org/~yun/
//


#include <cmath>
#include <cstring>
#include <algorithm>
#include <fs_attr.h>
#include <TypeConstants.h>
#include <Node.h>
#include <Message.h>
#include <List.h>
#include <View.h>
#include <Bitmap.h>
#include <TranslationUtils.h>
#include "MoeDefs.h"
#include "MoeUtils.h"
#include "MoeSurface.h"
#include "MoeAnime.h"
#include "MoeMascot.h"
#include "MoeAppUtils.h"
#include "MoeMascotManager.h"


MoeMascotManager::MoeMascotManager(int32 cellSize, int32 maxWindowNum)
  : mCellSize(cellSize)
  , mMaxWindowNum(maxWindowNum)
{
}


MoeMascotManager::~MoeMascotManager(void)
{
}


int32
MoeMascotManager::CountMascots(void)
{
  return mMascots.CountItems();
}


MoeMascot *
MoeMascotManager::MascotAt(int32 index)
{
  return reinterpret_cast<MoeMascot*>(mMascots.ItemAt(index));
}


void
MoeMascotManager::CellReduction(void)
{
  int32 i;
  
  mCellSize *= 2;

  for (i = 0; i < this->CountMascots(); i++)
    {
      MoeMascot *mascot = this->MascotAt(i);

      if (! mascot->IsHidden())
	mascot->Hide();

      mascot->SetCellSize(mCellSize);
      mascot->MoveToPrefered();
    }
}


int32
MoeMascotManager::SumSlices(void)
{
  int32 i, sum;

  for (i = sum = 0; i < this->CountMascots(); i++)
    sum += this->MascotAt(i)->CountSlices();

  return sum;
}


MoeMascot *
MoeMascotManager::Open(const entry_ref &entry)
{
  int32 i;
  BNode node;
  BList animes;
  attr_info info;

  for (i = 0; i < this->CountMascots(); i++)
    if (entry == this->MascotAt(i)->mEntry)
      return NULL;

  BBitmap *bitmap = BTranslationUtils::GetBitmap(&entry);
  if (! bitmap)
    return NULL;

  if (node.SetTo(&entry) == B_OK &&
      node.Lock() == B_OK)
    {
      // read anime patterns.
      char attrName[B_ATTR_NAME_LENGTH];
      node.RewindAttrs();

      while (node.GetNextAttrName(attrName) == B_OK)
	{
	  if (::strncmp(MOE_ATTR_ANIME,
			attrName,
			::strlen(MOE_ATTR_ANIME)) != 0)
	    continue;
	  
	  if (node.GetAttrInfo(attrName, &info) == B_OK &&
	      info.type == B_MESSAGE_TYPE)
	    {
	      char *buf = new char[info.size];
	      node.ReadAttr(attrName, B_MESSAGE_TYPE,
			    0, buf, info.size);
	      BMessage archive;
	      archive.Unflatten(buf);
	      MoeAnime *anime = new MoeAnime(&archive);
	      animes.AddItem(anime);
	      delete[] buf;
	    }
	}

      // read size.
      float size;
      if (node.GetAttrInfo(MOE_ATTR_SIZE, &info) == B_OK &&
	  info.type == B_FLOAT_TYPE &&
	  info.size == sizeof(float) &&
	  node.ReadAttr(MOE_ATTR_SIZE, B_FLOAT_TYPE, 0,
			&size, sizeof(float)) == sizeof(float) &&
	  size != 1.0)
	MoeAppUtils::MagnifyBitmaps(::sqrt(size), &bitmap, &animes);

      node.Unlock();
    }
  
  MoeSurface *surface = new MoeSurface(bitmap, mCellSize);
  for (i = 0; i < animes.CountItems(); i++)
    surface->IncludeExtraRect
      (reinterpret_cast<MoeAnime*>(animes.ItemAt(i))->Frame());


  while (this->SumSlices() + surface->CountRects() > 
	 std::max(mMaxWindowNum, this->CountMascots() + 1))
    {
      this->CellReduction();
      surface->SetCellSize(mCellSize);
    }

  MoeMascot *mascot = new MoeMascot(entry, surface);
  mMascots.AddItem(mascot);

  if (! animes.IsEmpty())
    {
      mascot->Lock();
      for (i = 0; i < animes.CountItems(); i++)
	mascot->AddAnime(reinterpret_cast<MoeAnime*>(animes.ItemAt(i)));
      mascot->Unlock();
      mascot->PostMessage(MOE_WINK_DONE);
    }

  return mascot;
}


void
MoeMascotManager::Close(MoeMascot *mascot)
{
  if (! mMascots.RemoveItem(mascot))
    return;

  mascot->Lock();
  mascot->Quit();
}


void
MoeMascotManager::Reopen(MoeMascot *mascot)
{
  if (! mMascots.HasItem(mascot))
    return;

  entry_ref entry = mascot->mEntry;
  this->Close(mascot);
  this->Open(entry);
}
