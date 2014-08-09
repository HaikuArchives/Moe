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
#include <Invoker.h>
#include <Looper.h>
#include <Messenger.h>
#include <MessageRunner.h>
#include <View.h>
#include <Bitmap.h>
#include <DataIO.h>
#include "MoeUtils.h"
#include "MoeCompressionUtils.h"
#include "MoeWindow.h"
#include "MoeAnime.h"


#define MOE_NEXT_SEQUENCE 'Mnsq'
#define MOE_INVALIDATE 'Mivd'


MoeAnime::MoeAnime(const char *name)
  : BHandler(name)
  , mTrigger(NULL)
  , mIsPlaying(false)
  , mCount(-1)
  , mReply(NULL)
{
}


MoeAnime::MoeAnime(BMessage *msg)
  : BHandler(msg)
  , mTrigger(NULL)
  , mIsPlaying(false)
  , mCount(-1)
  , mReply(NULL)
{
  this->Unarchive(msg);
}


MoeAnime::~MoeAnime(void)
{
  int32 i;

  for (i = 0; i < this->CountSequences(); i++)
    delete this->SequenceAt(i);

  for (i = 0; i < this->CountCells(); i++)
    delete this->CellAt(i);

  delete mTrigger;
  delete mReply;
}


status_t
MoeAnime::Archive(BMessage *archive, bool deep) const
{
  int32 i;
  BHandler::Archive(archive, deep);

  archive->AddRect("frame", mFrame);

  for (i = 0; i < this->CountSequences(); i++)
    {
      const sequence *seq = this->SequenceAt(i);
      archive->AddInt32("sequence", seq->cell);
      archive->AddInt32("wait", seq->wait);
    }

  for (i = 0; i < this->CountCells(); i++)
    {
      BMessage bitmapArchive;
      BMallocIO buf1, buf2;

      this->CellAt(i)->Archive(&bitmapArchive);
      bitmapArchive.Flatten(&buf1);
      buf1.Seek(0, SEEK_SET);
      MoeCompressionUtils::Compress(&buf1, &buf2, Z_BEST_COMPRESSION);
      archive->AddData("cell",
		       B_RAW_TYPE,
		       buf2.Buffer(),
		       buf2.BufferLength(),
		       false);
    }
}


void
MoeAnime::Unarchive(BMessage *msg)
{
  type_code type;
  int32 i, count, cell, wait;
  ssize_t bufLength;
  const void *buf;

  msg->FindRect("frame", &mFrame);

  msg->GetInfo("cell", &type, &count);

  for (i = 0; i < count; i++)
    {
      
      msg->FindData("cell", B_RAW_TYPE, i, &buf, &bufLength);
      BMemoryIO in(buf, bufLength);
      BMallocIO buf;
      BMessage bitmapArchive;

      MoeCompressionUtils::Decompress(&in, &buf);
      buf.Seek(0, SEEK_SET);
      bitmapArchive.Unflatten(&buf);
      this->AddCell(new BBitmap(&bitmapArchive));
    }

  msg->GetInfo("sequence", &type, &count);

  for (i = 0; i < count; i++)
    {
      msg->FindInt32("sequence", i, &cell);
      msg->FindInt32("wait", i, &wait);
      this->AddSequence(cell, wait);
    }
}


int32
MoeAnime::CountSequences(void) const
{
  return mSequences.CountItems();
}


const MoeAnime::sequence *
MoeAnime::SequenceAt(int32 index) const
{
  return reinterpret_cast<sequence*>(mSequences.ItemAt(index));
}


void
MoeAnime::AddSequence(int32 cell, int32 wait)
{
  sequence *seq = new sequence();
  seq->cell = cell;
  seq->wait = wait;
  mSequences.AddItem(seq);
}


int32
MoeAnime::CountCells(void) const
{
  return mCells.CountItems();
}


const BBitmap *
MoeAnime::CellAt(int32 index) const
{
  return reinterpret_cast<BBitmap*>(mCells.ItemAt(index));
}


void
MoeAnime::AddCell(BBitmap *bitmap)
{
  mCells.AddItem(bitmap);
}


BRect
MoeAnime::Frame(void)
{
  return mFrame;
}


void
MoeAnime::SetFrame(BRect frame)
{
  mFrame = frame;
}


MoeWindow *
MoeAnime::Window(void) const
{
  return static_cast<MoeWindow*>(this->Looper());
}


bool
MoeAnime::IsPlaying(void)
{
  return mIsPlaying;
}


void
MoeAnime::Reset(void)
{
  if (mReply)
    mReply->Invoke();
  delete mReply;
  mReply = NULL;
  delete mTrigger;
  mTrigger = NULL;
  mIsPlaying = false;
}


void
MoeAnime::Redraw(void)
{
  this->Window()->ComposeBitmap(false);
  BMessage msg(MOE_INVALIDATE);
  msg.AddRect("frame", mFrame);
  this->Window()->PostMessage(&msg);
}


void
MoeAnime::Play(BInvoker *reply)
{
  this->Reset();
  mIsPlaying = true;
  mCount = -1;
  mReply = reply;
  this->Next();
}


void
MoeAnime::Stop(void)
{
  this->Reset();
  this->Redraw();
}


void
MoeAnime::Next(void)
{
  if (++mCount >= this->CountSequences())
    {
      this->Stop();
      return;
    }

  delete mTrigger;
  BMessage msg(MOE_NEXT_SEQUENCE);
  mTrigger = new BMessageRunner(BMessenger(this),
				&msg,
				this->SequenceAt(mCount)->wait * 10000,
				1);

  this->Redraw();
}


void
MoeAnime::Draw(BView *view)
{
  if (! mIsPlaying)
    return;

  const BBitmap *bitmap = this->CellAt(this->SequenceAt(mCount)->cell);
  if (! bitmap)
    return;

  view->DrawBitmap(bitmap, mFrame);
}


void
MoeAnime::MessageReceived(BMessage *msg)
{
  switch (msg->what)
    {
    default:
      BHandler::MessageReceived(msg);
      break;

    case MOE_NEXT_SEQUENCE:
      this->Next();
      break;
    }
}

