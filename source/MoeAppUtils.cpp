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


#include <cstdio>
#include <cmath>
#include <fs_attr.h>
#include <Node.h>
#include <Entry.h>
#include <DataIO.h>
#include <Message.h>
#include <String.h>
#include <TranslationUtils.h>
#include <Bitmap.h>
#include <View.h>
#include <List.h>
#include "MoeDefs.h"
#include "MoeUtils.h"
#include "MoeAnime.h"
#include "MoeConsole.h"
#include "MoeAppUtils.h"


void
MoeAppUtils::MagnifyBitmaps(float size,
			    BBitmap **bitmap,
			    BList *animes)
{
  int32 i, j;
  BBitmap *buf, *old_bitmap;
  
  old_bitmap = *bitmap;
  *bitmap =
    new BBitmap(BRect(0, 0,
		      ::floor(old_bitmap->Bounds().Width() * size),
		      ::floor(old_bitmap->Bounds().Height() * size)),
		B_RGB32);
  MoeUtils::TransparentLeftTop(old_bitmap);
  
  buf = new BBitmap(old_bitmap->Bounds(), B_RGB32, true);
  BView *canvas = new BView(buf->Bounds(), "", 0, B_WILL_DRAW);
  buf->Lock();
  buf->AddChild(canvas);
  
  // magnify animeations.
  BList newAnimes;
  
  for (i = 0; i < animes->CountItems(); i++)
    {
      MoeAnime *old_anime, *new_anime;
      BRect old_frame, new_frame;
      old_anime = reinterpret_cast<MoeAnime*>(animes->ItemAt(i));
      new_anime = new MoeAnime(old_anime->Name());
      old_frame = old_anime->Frame();
      new_frame.Set(::ceil(old_frame.left * size),
		    ::ceil(old_frame.top * size),
		    ::floor(old_frame.right * size),
		    ::floor(old_frame.bottom * size));
      new_anime->SetFrame(new_frame);
      
      for (j = 0; j < old_anime->CountSequences(); j++)
	{
	  const MoeAnime::sequence *seq = old_anime->SequenceAt(j);
	  new_anime->AddSequence(seq->cell, seq->wait);
	}
      
      for (j = 0; j < old_anime->CountCells(); j++)
	{
	  canvas->DrawBitmap(old_bitmap);
	  canvas->DrawBitmap(old_anime->CellAt(j), old_frame);
	  canvas->Sync();
	  MoeUtils::MapBitmap(*bitmap, buf);
	  BBitmap *cell = new BBitmap(new_frame.OffsetToCopy(0, 0),
				      B_RGB32);
	  MoeUtils::MapBitmap(cell, *bitmap, &new_frame);
	  new_anime->AddCell(cell);
	}
      
      delete old_anime;
      newAnimes.AddItem(new_anime);
    }
  
  MoeUtils::MapBitmap(*bitmap, old_bitmap);
  
  *animes = newAnimes;
  
  delete buf;
  delete old_bitmap;
}

			    

inline static entry_ref *
entry_at(BList list, int32 index)
{
  return reinterpret_cast<entry_ref*>(list.ItemAt(index));
}


inline static BBitmap *
bitmap_at(BList &list, int32 index)
{
  return reinterpret_cast<BBitmap*>(list.ItemAt(index));
}


void
MoeAppUtils::AddAnime(BList *entries, const char *name, int32 wait)
{
  int32 i, j, x, y, w, h, *sequences;
  BString animeName, attrName;
  BList bitmaps;
  BMessage archive;
  BNode node;
  BMallocIO buf;
  BBitmap *bitmap, *baseBitmap;
  BView *view;
  BRect frame(10000, 10000, -10000, -10000);
  MoeAnime *anime;

  sequences = new int32[entries->CountItems()];

  MoeConsole::Printf("begin anime processing.\n");

  for (i = 0; i < entries->CountItems(); i++)
    {
      entry_ref entry = *::entry_at(*entries, i);

      for (j = 0; j < i; j++)
	if (entry == *::entry_at(*entries, j))
	  break;

      if (i != j)
	{
	  sequences[i] = sequences[j];
	  continue;
	}

      MoeConsole::Printf("reading '%s'.\n", entry.name);
      
      bitmap = BTranslationUtils::GetBitmap(&entry);
      if (! bitmap)
	{
	  MoeConsole::Printf("read failed.\n");
	  goto SetAnimeErrorBreak;
	}

      MoeUtils::TransparentLeftTop(bitmap);
      sequences[i] = bitmaps.CountItems();
      bitmaps.AddItem(bitmap);
    }

  baseBitmap = ::bitmap_at(bitmaps, 0);

  if (bitmaps.CountItems() < 2)
    {
      MoeConsole::Printf("too few image files specified.\n");
      goto SetAnimeErrorBreak;
    }

  for (i = 1; i < bitmaps.CountItems(); i++)
    if (baseBitmap->Bounds() != bitmap_at(bitmaps, i)->Bounds())
      {
	MoeConsole::Printf("bitmap frame differs.\n");
	goto SetAnimeErrorBreak;
      }

  w = baseBitmap->Bounds().IntegerWidth() + 1;
  h = baseBitmap->Bounds().IntegerHeight() + 1;

  for (i = 1; i < bitmaps.CountItems(); i++)
    {
      bitmap = bitmap_at(bitmaps, i);
      for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	  if (*pixel_at(baseBitmap, x, y) != *pixel_at(bitmap, x, y))
	    frame = frame | BRect(x, y, x, y);
    }
  
  if (! frame.IsValid())
    {
      MoeConsole::Printf("no anime pattern found.\n");
      goto SetAnimeErrorBreak;
    }

  MoeConsole::Printf("found anime pattern frame (%d, %d)-(%d, %d).\n",
		     int(frame.left), int(frame.top),
		     int(frame.right), int(frame.bottom));

#if 0
  // minimize animation pixels.
  for (i = 1; i < bitmaps.CountItems(); i++)
    {
      bitmap = bitmap_at(bitmaps, i);
      for (x = int32(frame.left); x <= int32(frame.right); x++)
	for (y = int32(frame.top); y <= int32(frame.bottom); y++)
	  {
	    rgb_color *pixel0, *pixel1;
	    pixel0 = ::pixel_at(baseBitmap, x, y);
	    pixel1 = ::pixel_at(bitmap, x, y);
	    if (*pixel0 == *pixel1)
	      *pixel1 = B_TRANSPARENT_32_BIT;
	    else if (*pixel0 != B_TRANSPARENT_32_BIT &&
		     *pixel1 == B_TRANSPARENT_32_BIT)
	      pixel1->red = 255;
	  }
    }
#endif

  animeName = name;
  animeName.ToUpper();
  anime = new MoeAnime(animeName.String());

  bitmap = new BBitmap(frame.OffsetToCopy(0, 0), B_RGB32, true);
  bitmap->Lock();
  view = new BView(bitmap->Bounds(), "", 0, B_WILL_DRAW);
  bitmap->AddChild(view);
  
  anime->SetFrame(frame);

  for (i = 1; i < entries->CountItems(); i++)
    anime->AddSequence(sequences[i] - 1, wait);

  for (i = 1; i < bitmaps.CountItems(); i++)
    {
      view->DrawBitmap(bitmap_at(bitmaps, i), frame, bitmap->Bounds());
      view->Sync();
      anime->AddCell(new BBitmap(bitmap));
    }

  anime->Archive(&archive);
  
  delete anime;
  delete bitmap;

  archive.Flatten(&buf);

  MoeConsole::Printf("writing '%s'.\n", entry_at(*entries, 0)->name);

  attrName << MOE_ATTR_ANIME << name;
  attrName.ToUpper();

  if (node.SetTo(entry_at(*entries, 0)) != B_OK ||
      node.Lock() != B_OK)
    {
      MoeConsole::Printf("write failed.\n");
      goto SetAnimeErrorBreak;
    }

  node.RemoveAttr(attrName.String());

  if (node.WriteAttr(attrName.String(), B_MESSAGE_TYPE, 0, 
		     buf.Buffer(), buf.BufferLength())
      != int32(buf.BufferLength()))
    {
      MoeConsole::Printf("write failed.\n");
      goto SetAnimeErrorBreak;
    }

 SetAnimeErrorBreak:

  for (i = 0; i < bitmaps.CountItems(); i++)
    delete ::bitmap_at(bitmaps, i);

  delete[] sequences;

  MoeConsole::Printf("\n");
}


void
MoeAppUtils::DeleteAnime(BList *entries, const char *name)
{
  int32 i;
  BNode node;
  BString attrName;

  attrName << MOE_ATTR_ANIME << name;
  attrName.ToUpper();

  for (i = 0; i < entries->CountItems(); i++)
    if (node.SetTo(::entry_at(*entries, i)) == B_OK &&
	node.Lock() == B_OK)
      {
	if (node.RemoveAttr(attrName.String()) == B_OK)
	  MoeConsole::Printf("delete '%s' from '%s'.\n",
			     name, 
			     ::entry_at(*entries, i)->name);
	node.Unlock();
      }

  MoeConsole::Printf("\n");
}


void
MoeAppUtils::Info(BList *entries)
{
  int32 i;
  attr_info info;
  BNode node;
  float rate, size;
  bool locked;
  static const char *lockedString[2] = {"locked", "not locked"};

  for (i = 0; i < entries->CountItems(); i++)
    {
      entry_ref *entry = ::entry_at(*entries, i);

      if (node.SetTo(entry) != B_OK ||
	  node.Lock() != B_OK)
	continue;

      MoeConsole::Printf("information of '%s'\n",
			 ::entry_at(*entries, i)->name);

      size = 1.0;
      if (node.GetAttrInfo(MOE_ATTR_SIZE, &info) == B_OK &&
	  info.type == B_FLOAT_TYPE &&
	  info.size == sizeof(float))
	node.ReadAttr(MOE_ATTR_SIZE, B_FLOAT_TYPE, 0, &size, sizeof(float));

      MoeConsole::Printf("\tsize:\t%f\n", size);

      rate = 0.5;
      if (node.GetAttrInfo(MOE_ATTR_X_RATE, &info) == B_OK &&
	  info.type == B_FLOAT_TYPE &&
	  info.size == sizeof(float))
	node.ReadAttr(MOE_ATTR_X_RATE, B_FLOAT_TYPE, 0,	&rate, sizeof(float));
      MoeConsole::Printf("\tx_rate:\t%f\n", rate);

      rate = 0.5;
      if (node.GetAttrInfo(MOE_ATTR_Y_RATE, &info) == B_OK &&
	  info.type == B_FLOAT_TYPE &&
	  info.size == sizeof(float))
	node.ReadAttr(MOE_ATTR_Y_RATE, B_FLOAT_TYPE, 0, &rate, sizeof(float));
      MoeConsole::Printf("\ty_rate:\t%f\n", rate);

      locked = false;
      if (node.GetAttrInfo(MOE_ATTR_X_LOCKED, &info) == B_OK &&
	  info.type == B_BOOL_TYPE &&
	  info.size == sizeof(bool))
	node.ReadAttr(MOE_ATTR_X_LOCKED, B_BOOL_TYPE, 0,
		      &locked, sizeof(bool));
      MoeConsole::Printf("\tx_lock:\t%s\n", lockedString[locked]);

      locked = false;
      if (node.GetAttrInfo(MOE_ATTR_Y_LOCKED, &info) == B_OK &&
	  info.type == B_BOOL_TYPE &&
	  info.size == sizeof(bool))
	node.ReadAttr(MOE_ATTR_Y_LOCKED, B_BOOL_TYPE, 0,
		      &locked, sizeof(bool));
      MoeConsole::Printf("\ty_lock:\t%s\n", lockedString[locked]);

      node.Unlock();

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
	      BMessage archive;
	      char *buf = new char[info.size];
	      node.ReadAttr(attrName, B_MESSAGE_TYPE,
			    0, buf, info.size);
	      archive.Unflatten(buf);
	      MoeAnime *anime = new MoeAnime(&archive);
	      MoeConsole::Printf("\tanime:\t%s, frame=(%d,%d,%d,%d), "
				 "sequences=%d, cells=%d\n",
				 anime->Name(), 
				 int(anime->Frame().left), 
				 int(anime->Frame().top), 
				 int(anime->Frame().right), 
				 int(anime->Frame().bottom), 
				 int(anime->CountSequences()),
				 int(anime->CountCells()));

	      delete[] buf;
	      delete anime;
	    }
	}

      MoeConsole::Printf("\n");
    }
}

