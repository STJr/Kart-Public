/*
SONIC ROBO BLAST 2 KART

Copyright 2019 James R.

This program is free software distributed under the
terms of the GNU General Public License, version 2.
See the 'LICENSE' file for more details.
*/

#include "doomdef.h"
#include "doomstat.h"
#include "d_vote.h"
#include "command.h"
#include "g_game.h"

#define CVAR( name, default, ...) \
	consvar_t cv_chatvote_ ## name =\
{\
	"chatvote_" #name, default,\
	CV_SAVE, __VA_ARGS__\
}

CVAR (time,       "20", CV_Unsigned);
CVAR (minimum,     "0", CV_PlayerCount);
CVAR (percentage,  "0", CV_Percent);

CVAR (nag,        "VOTE NOW FUCKER");

#undef  CVAR

struct D_ChatVote d_chatvote;

static void
Saytoissuer (int n, const char *s)
{
	if (n == consoleplayer)
		CONS_Printf("%s\n", s);
	else
		D_Sayto(n, s);
}

static void
Addvote (int c, int n)
{
	if (c < 0)
		d_chatvote.no  += n;
	else
		d_chatvote.yes += n;
}

void
D_ClearVote (void)
{
	d_chatvote.type = 0;
	d_chatvote.time = 0;
}

void
Callvote (void)
{
	switch (d_chatvote.type)
	{
		case VOTE_KICK:
			D_Say(va(
						"Vote passed (%d to %d). Bye bye!",
						d_chatvote.yes,
						d_chatvote.no));
			/* xd console hacks */
			COM_ImmedExecute(va(
						"kick %d",
						d_chatvote.target));
			break;
	}
}

void
Endvote (void)
{
	int yes;
	int  no;

	yes = d_chatvote.yes;
	no  = d_chatvote.no;

	/* First we need more votes in favor. Easy. */
	if (yes > no)
	{
		/* check minimum votes needed */
		if (yes >= d_chatvote.needed)
		{
			Callvote();
		}
		else
		{
			D_Say(va(
						"Not enough votes were made in order to suffice the "
						"minimum (%d / %d).",
						yes,
						d_chatvote.needed));
		}
	}
	else if (yes < no)
	{
		D_Say(va(
					"Vote failed (%d to %d).",
					no,
					yes));
	}
	else
	{
		if (no)/* did anyone vote??? */
		{
			D_Say(va(
						"Votes tied (%d to %d).",
						no,
						yes));
		}
		else
		{
			D_Say("No one voted! What is wrong with you?");
		}
	}

	D_ClearVote();
}

int
D_VoteTime (void)
{
	return (
			d_chatvote.time / TICRATE +
			d_chatvote.time % TICRATE / (TICRATE/2)
	);
}

void
D_StartVote (int type, int target, int from)
{
	if (d_chatvote.type)
	{
		if (from == consoleplayer)
		{
			CONS_Printf(
					"There is already a vote running.\n"
					"Cancel it with cancelvote first, or just wait %d seconds.\n",
					D_VoteTime());
		}
		else
		{
			D_Sayto(from,
					"There is already a vote running.");
		}
	}
	else
	{
		d_chatvote.type     = type;
		d_chatvote.time     = cv_chatvote_time.value * TICRATE;

		d_chatvote.target   = target;
		d_chatvote.from     = from;

		d_chatvote.yes      = 0;
		d_chatvote.no       = 0;

		d_chatvote.minimum  = cv_chatvote_minimum.value;
		d_chatvote.percent  = cv_chatvote_percentage.value;

		D_RecalcVote();

		memset(d_chatvote.votes, 0, sizeof d_chatvote.votes);

		d_chatvote.duration = d_chatvote.time;

		D_Say(va(
				"A vote to kick %s has started. You have %d seconds to vote. "
				"Vote now!",
				player_names[target],
				D_VoteTime()));
		if (cv_chatvote_nag.string[0])
			D_CSay(cv_chatvote_nag.string);
	}
}

void
D_StopVote (const char *reason, int from)
{
	if (d_chatvote.type)
	{
		D_ClearVote();
		if (reason)
		{
			D_Say(va(
						"The vote has been cancelled. %s",
						reason));
		}
		else
		{
			D_Say(
					"The vote has been cancelled.");
		}
	}
	else
	{
		Saytoissuer(from,
				"There is no vote running!");
	}
}

void
D_Vote (int n, int from)
{
	int * vote;

	int d;

	if (d_chatvote.type)
	{
		/*
		Don't let the caller vote. The check on d_chatvote.from confirms that the
		caller didn't leave. It also means the server can always vote. But it's
		THE server, let 'em.

		...Don't let the target vote either
		*/
		if (d_chatvote.from && from == d_chatvote.from)
			D_Sayto(from, "You cannot vote if you called the vote!");
		else if (from == d_chatvote.target)
			D_Sayto(from, "This vote is against you; you can't vote!");
		else if (! d_chatvote.from || from != d_chatvote.from)
		{
			d = (*( vote = &d_chatvote.votes[from] ));

			if (~(d))
			{
				Addvote(d, -abs(d));/* subtract our previous vote */
				Addvote(n,  abs(n));/* add our new vote */

				(*vote) = n;/* cache for later */

				if (n != d)
				{
					if (n < 0)
						D_Sayto(from, "You voted no.");
					else
						D_Sayto(from, "You voted yes.");
				}
			}
			else
			{
				D_Sayto(from,
						"You cannot vote yet. "
						"Please wait until another vote starts.");
			}
		}
	}
}

void
D_VoteTicker (void)
{
	int     time;
	int duration;

	if (d_chatvote.time)
	{
		if (! --d_chatvote.time)
		{
			Endvote();
		}
		else
		{
			/*
			Reminding you to VOTE at half and fourth time,
			then countdown at or below five seconds.
			*/

			time     = d_chatvote.time;
			duration = d_chatvote.duration;

			if (time % TICRATE == 0)/* ON the second */
			{
				if (time <= 5*TICRATE)
				{
					D_Say(va(
								"Vote ends in %d seconds...",
								D_VoteTime()));
				}
				else
				{
					if (duration % time == 0)/* don't count integer flooring */
					{
						switch (( duration / time ))
						{
							case 2:
							case 4:
								D_Say(va(
											"You have %d seconds left to vote.",
											D_VoteTime()));
						}
					}
				}
			}
		}
	}
}

void
D_RecalcVote (void)
{
	if (d_chatvote.type)
	{
		d_chatvote.needed   = d_chatvote.minimum;
		if (! d_chatvote.needed &&
				d_chatvote.percent)
		{
			/* subtract the caller and target from minimum */
			d_chatvote.needed =
				D_NumPlayers() * d_chatvote.percent / 100 - 2;
		}
	}
}
