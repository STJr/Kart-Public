// SONIC ROBO BLAST 2 KART
//-----------------------------------------------------------------------------
// Copyright (C) 2020 by James R.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  stun.h
/// \brief RFC 5389 client implementation to fetch external IP address.

#ifndef KART_STUN_H
#define KART_STUN_H

typedef void (*stun_callback_t)(UINT32 address);

void    STUN_bind (stun_callback_t);
boolean STUN_got_response (const char * const buffer, const size_t size);

#endif/*KART_STUN_H*/
