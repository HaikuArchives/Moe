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
#include <cstdlib>
#include <getopt.h>
#include <String.h>
#include <Application.h>
#include <AboutWindow.h>
#include <Roster.h>
#include <AppFileInfo.h>
#include <File.h>
#include <Entry.h>
#include <Path.h>
#include <Alert.h>
#include <Catalog.h>
#include "MoeDefs.h"
#include "MoeUtils.h"
#include "MoeConsole.h"
#include "MoeMascotManager.h"
#include "MoeProperty.h"
#include "MoeAppUtils.h"
#include "MoeActiveWindowWatcher.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Command line strings"

static const char *
sHelp =
B_TRANSLATE("Usage: Moe [options] [files]\n"
"Options: [default in brackets after descriptions]\n"
"Configuration:\n"
"  --help                print this message\n"
"Animation:\n"
"  --add-anime=ANIME     add anime named ANIME to primary image file\n"
"  --delete-anime=ANIME  delete anime named ANIME from mascot files\n"
"  --wait=WAIT           set anime wait to WAIT\n"
"                        [10]\n"
"Files:\n"
"  --info                show mascot file information\n"
"\n")
;


class MoeApplication : public BApplication
{
public:
  MoeApplication(void);
  virtual ~MoeApplication(void);

protected:  
  virtual void RefsReceived(BMessage *msg);
  virtual void ArgvReceived(int32 argc, char **argv);
  virtual void ReadyToRun(void);
  virtual void AboutRequested(void);
  virtual bool QuitRequested(void);
  virtual void MessageReceived(BMessage *msg);

private:
  MoeMascotManager mMascotManager;
};



MoeApplication::MoeApplication(void)
  : BApplication(MOE_APP_SIGNATURE)
{
}


MoeApplication::~MoeApplication(void)
{
}


void
MoeApplication::RefsReceived(BMessage *msg)
{
  entry_ref ref;
  int i;

  for (i = 0; msg->FindRef("refs", i, &ref) == B_OK; i++)
    mMascotManager.Open(ref);
}


void
MoeApplication::ArgvReceived(int32 argc, char **argv)
{
  const char *cwd;
  BPath path;
  BList entries;
  entry_ref entry;
  int i;

  int wait, c;
  enum {OPEN, ADD, DELETE, INFO} mode;
  BString animeName;
  static struct option
    long_options[] = {
      {"add-anime", 1, 0, 'a'},
      {"delete-anime", 1, 0, 'd'},
      {"info", 0, 0, 'i'},
      {"help", 0, 0, 'h'},
      {"wait", 1, 0, 'w'},
      {0, 0, 0, 0},
    };

  ::optind = 0;
  wait = 10;
  mode = OPEN;

  while (1)
    {
      c = ::getopt_long(argc, argv, "a:d:w:ih", long_options, NULL);

      if (c == -1)
	break;

      switch (c) 
	{
	case 'a':
	  if (mode != OPEN)
	    {
	      MoeConsole::Printf(B_TRANSLATE("overlapping mode specific options.\n\n"));
	      return;
	    }
	  mode = ADD;
	  animeName = ::optarg;
  	  break;

	case 'd':
	  if (mode != OPEN)
	    {
	      MoeConsole::Printf(B_TRANSLATE("overlapping mode specific options.\n\n"));
	      return;
	    }
	  mode = DELETE;
	  animeName = ::optarg;
  	  break;
	  
	case 'i':
	  if (mode != OPEN)
	    {
	      MoeConsole::Printf(B_TRANSLATE("overlapping mode specific options.\n\n"));
	      return;
	    }
	  mode = INFO;
  	  break;
	  
	case 'w':
	  wait = ::atoi(::optarg);
	  break;

	case 'h':
	  MoeConsole::Printf(sHelp);
	  break;

	default:
	  MoeConsole::Printf(B_TRANSLATE("unknown option.\n\n"));
	  return;
	}
    }
  
  if (this->CurrentMessage()->FindString("cwd", &cwd) != B_OK)
    return;

  for (i = optind; i < argc; i++)
    {
      if (argv[i][0] == '/')
	{
	  path.SetTo(argv[i]);
	}
      else
	{
	  path.SetTo(cwd);
	  path.Append(argv[i]);
	}

      if (path.InitCheck() != B_NO_ERROR)
	continue;

      ::get_ref_for_path(path.Path(), &entry);
      entries.AddItem(new entry_ref(entry));
    }

  switch (mode)
    {
    default:
      for (i = 0; i < entries.CountItems(); i++)
	mMascotManager.Open(*reinterpret_cast<entry_ref*>(entries.ItemAt(i)));
      break;

    case ADD:
      MoeAppUtils::AddAnime(&entries, animeName.String(), wait);
      break;

    case DELETE:
      MoeAppUtils::DeleteAnime(&entries, animeName.String());
      break;

    case INFO:
      MoeAppUtils::Info(&entries);
      break;
    }

  for (i = 0; i < entries.CountItems(); i++)
    delete reinterpret_cast<entry_ref*>(entries.ItemAt(i));
}



void
MoeApplication::ReadyToRun(void)
{
  this->PostMessage(MOE_EXAMINE_QUIT_REQUESTED);

  MoeActiveWindowWatcher::Watcher()->Run();
}

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application About dialog box"

void
MoeApplication::AboutRequested(void)
{
  BAboutWindow* about = new BAboutWindow(B_TRANSLATE_SYSTEM_NAME("Moe"),
  	MOE_APP_SIGNATURE);
  const char* authors [] = { /* From documentation files */
  	B_TRANSLATE("Okada Jun (programming)"),
  	B_TRANSLATE("Yu-Ki (illustration)"),
  	"Cafeina",
  	NULL
  };
  const char* extraCopyrights [] = {
  	"2021 Cafeina",
  	NULL
  };
  const char* thanks [] = { /* From documentation files */
  	"Toyoshima",
  	"Yu-Ki",
  	NULL
  };
  
  about->AddCopyright(2001, "Okada Jun", extraCopyrights);
  about->AddAuthors(authors);
  about->AddSpecialThanks(thanks);
  about->AddExtraInfo(B_TRANSLATE("Project Be Moe.")); /* From resource file */
  about->AddDescription(B_TRANSLATE("Moe is a program to place a cute mascot on the active window.\n")); /* From resource file */
  about->Show();
}


bool
MoeApplication::QuitRequested(void)
{
  MoeActiveWindowWatcher::Watcher()->Stop();
  
  while (mMascotManager.CountMascots() > 0)
    mMascotManager.Close(mMascotManager.MascotAt(0));

  return BApplication::QuitRequested();
}


void
MoeApplication::MessageReceived(BMessage *msg)
{
  switch (msg->what)
    {
    default:
      BApplication::MessageReceived(msg);
      break;

    case MOE_MASCOT_REOPEN_REQUESTED:
      {
	MoeMascot *mascot;

	msg->FindPointer("data", reinterpret_cast<void**>(&mascot));
	mMascotManager.Reopen(mascot);
	
	break;
      }

    case MOE_MASCOT_QUIT_REQUESTED:
      {
	MoeMascot *mascot;

	msg->FindPointer("data", reinterpret_cast<void**>(&mascot));
	mMascotManager.Close(mascot);
      }
      // PASS THROUGH
      
    case MOE_EXAMINE_QUIT_REQUESTED:
      {
	if (mMascotManager.CountMascots() == 0)
	  this->PostMessage(B_QUIT_REQUESTED);
	break;
      }
    }
}


int
main(int argc, char **argv)
{
  if (argc == 1)
    printf(sHelp);

  MoeApplication().Run();
}
