/*
Server-side voting system via the chat.
*/
/*
SONIC ROBO BLAST 2 KART

Copyright 2019 James R.

This program is free software distributed under the
terms of the GNU General Public License, version 2.
See the 'LICENSE' file for more details.
*/

#ifndef SRB2KART_D_VOTE_H
#define SRB2KART_D_VOTE_H

#include "command.h"

enum
{
	/*VOTE_NONE = 0,*/
	VOTE_KICK = 1,
};

struct D_ChatVote
{
	int     type;/* one the above enum */
	int     time;/* tics left until vote ends */
	int   target;/* player number of the victim */

	/* number of votes in favor or against */

	int      yes;
	int       no;

	int   needed;/* votes needed; cached because lol cvars */
	int    votes[MAXPLAYERS];/* you can't vote twice! */
	int duration;/* total time of vote */
	int     from;/* caller of this vote */
};

extern consvar_t cv_chatvote_time;
extern consvar_t cv_chatvote_minimum;
extern consvar_t cv_chatvote_percentage;

extern struct D_ChatVote d_chatvote;

/* vote time in seconds */
int  D_VoteTime   (void);

void D_StartVote  (int type, int target, int from);
void D_StopVote   (const char *reason,   int from);

void D_Vote       (int direction,        int from);

void D_VoteTicker (void);

#endif/*SRB2KART_D_VOTE_H*/
