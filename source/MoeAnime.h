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


#ifndef MOE_ANIME_H
#define MOE_ANIME_H


#include <Handler.h>
#include <List.h>
#include <Rect.h>


class BInvoker;
class BBitmap;
class BMessageRunner;
class MoeWindow;


class MoeAnime : public BHandler
{
public:
  struct sequence
  {
    int32 cell;
    int32 wait;
  };

  MoeAnime(const char *name);
  MoeAnime(BMessage *msg);
  virtual ~MoeAnime(void);
  virtual status_t Archive(BMessage *archive, bool deep = true) const;

  void Play(BInvoker *reply = NULL);
  void Stop(void);
  bool IsPlaying(void);

  MoeWindow *Window(void) const;

  void Draw(BView *view);

  int32 CountSequences(void) const;
  const sequence *SequenceAt(int32 index) const;
  void AddSequence(int32 cell, int32 wait);

  int32 CountCells(void) const;
  const BBitmap *CellAt(int32 index) const;
  void AddCell(BBitmap *bitmap);

  BRect Frame(void);
  void SetFrame(BRect frame);

protected:
  virtual void MessageReceived(BMessage *msg);

private:
  void Unarchive(BMessage *msg);

  void Reset(void);
  void Redraw(void);
  void Next(void);

  BList mSequences, mCells;
  BRect mFrame;
  BMessageRunner *mTrigger;
  bool mIsPlaying;
  int32 mCount;
  BInvoker *mReply;
};


#endif
