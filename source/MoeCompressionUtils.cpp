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


#include <DataIO.h>
#include "MoeCompressionUtils.h"


const static unsigned int BUF_SIZE = 1024;


status_t
MoeCompressionUtils::Compress(BDataIO *in, BDataIO *out, int level)
{
  z_stream z;
  Bytef inBuf[BUF_SIZE], outBuf[BUF_SIZE];
  int status, flush, count;

  z.zalloc = Z_NULL;
  z.zfree = Z_NULL;
  z.opaque = Z_NULL;

  if (::deflateInit(&z, level) != Z_OK)
    return B_ERROR;

  z.avail_in = 0;
  z.avail_out = BUF_SIZE;
  z.next_out = outBuf;

  flush = Z_NO_FLUSH;

  for (;;)
    {
      if (z.avail_in == 0)
	{
	  z.next_in = inBuf;
	  z.avail_in = in->Read(inBuf, BUF_SIZE);
	  if (z.avail_in < BUF_SIZE)
	    flush = Z_FINISH;
	}

      status = ::deflate(&z, flush);

      if (status == Z_STREAM_END)
	break;

      if (status != Z_OK)
	return B_ERROR;

      if (z.avail_out == 0)
	{
	  if (out->Write(outBuf, BUF_SIZE) != size_t(BUF_SIZE))
	    return B_ERROR;

	  z.next_out = outBuf;
	  z.avail_out = BUF_SIZE;
	}
    }

  if ((count = (BUF_SIZE - z.avail_out)) != 0)
    if (out->Write(outBuf, count) != count)
      return B_ERROR;

  if (::deflateEnd(&z) != Z_OK)
    return B_ERROR;

  return B_OK;
}


status_t
MoeCompressionUtils::Decompress(BDataIO *in, BDataIO *out)
{
  z_stream z;
  Bytef inBuf[BUF_SIZE], outBuf[BUF_SIZE];
  int status, count;

  z.zalloc = Z_NULL;
  z.zfree = Z_NULL;
  z.opaque = Z_NULL;

  if (::inflateInit(&z) != Z_OK)
    return B_ERROR;

  z.avail_in = 0;
  z.avail_out = BUF_SIZE;
  z.next_out = outBuf;

  for (;;)
    {
      if (z.avail_in == 0)
	{
	  z.next_in = inBuf;
	  z.avail_in = in->Read(inBuf, BUF_SIZE);
	}

      status = ::inflate(&z, Z_NO_FLUSH);

      if (status == Z_STREAM_END)
	break;

      if (status != Z_OK)
	return B_ERROR;

      if (z.avail_out == 0)
	{
	  if (out->Write(outBuf, BUF_SIZE) != size_t(BUF_SIZE))
	    return B_ERROR;

	  z.next_out = outBuf;
	  z.avail_out = BUF_SIZE;
	}
    }

  if ((count = (BUF_SIZE - z.avail_out)) != 0)
    if (out->Write(outBuf, count) != count)
      return B_ERROR;

  if (::inflateEnd(&z) != Z_OK)
    return B_ERROR;

  return B_OK;
}



