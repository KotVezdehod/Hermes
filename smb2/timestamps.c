/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
/*
   Copyright (C) 2016 by Ronnie Sahlberg <ronniesahlberg@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "compat.h"

#include "portable-endian.h"

#include "include/smb2/smb2.h"
#include "include/smb2/libsmb2.h"
#include "include/libsmb2-private.h"

uint64_t
timeval_to_win(struct smb2_timeval *tv)
{
        return ((uint64_t)tv->tv_sec * 10000000) +
                116444736000000000 + tv->tv_usec * 10;
}

void
win_to_timeval(uint64_t smb2_time, struct smb2_timeval *tv)
{
        tv->tv_usec = (smb2_time / 10) % 1000000;
        tv->tv_sec  = (smb2_time - 116444736000000000) / 10000000;
}
