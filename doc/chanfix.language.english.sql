-- Language Definition Table for Chanfix.
--
-- Language: English (1)
-- Author: Evilnet Development Team <http://evilnet.sourceforge.net>
--
-- $Id$

			
DELETE FROM translations;
	
COPY "translations" FROM stdin;
1	1	You may only add one flag per ADDFLAG command.	31337	0
1	2	No such user %s.	31337	0
1	3	You cannot add an owner flag.	31337	0
1	4	Only an owner can add the user management flag.	31337	0
1	5	You cannot add a flag to a user in a different group.	31337	0
1	6	You cannot add a block flag.	31337	0
1	7	You cannot add a serveradmin flag.	31337	0
1	8	User %s already has flag %c.	31337	0
1	9	Added flag %c to user %s.	31337	0
1	10	You cannot add a host to a user in a different group.	31337	0
1	11	User %s already has hostmask %s.	31337	0
1	12	Failed adding hostmask %s to user %s.	31337	0
1	13	Added hostmask %s to user %s.	31337	0
1	14	%s is an invalid channel name.	31337	0
1	15	Note recorded for channel %s.	31337	0
1	16	User %s already exists.	31337	0
1	17	Created user %s (%s).	31337	0
1	18	Created user %s.	31337	0
1	19	Error creating user %s. (Insertion failed)	31337	0
1	20	The channel %s already has the ALERT flag.	31337	0
1	21	ALERT flag added from channel %s	31337	0
1	22	Channel blocking is disabled.	31337	0
1	23	The channel %s is already blocked.	31337	0
1	24	The channel %s has been blocked.	31337	0
1	25	Sorry, manual chanfixes are currently disabled.	31337	0
1	26	Sorry, chanfix is currently disabled because not enough servers are linked.	31337	0
1	27	No such channel %s.	31337	0
1	28	%s cannot be chanfixed as it uses oplevels (+A/+U). If this channel has been taken over and needs to be returned to the original owners, append the OVERRIDE flag to force a manual fix.	31337	0
1	29	%s is a registered channel.	31337	0
1	30	There are no scores in the database for %s.	31337	0
1	31	The channel %s is already being manually fixed.	31337	0
1	32	The highscore in channel %s is %d which is lower than the minimum score required (%.2f * %d = %d).	31337	0
1	33	The channel %s is being automatically fixed. Append the OVERRIDE flag to force a manual fix.	31337	0
1	34	The channel %s is BLOCKED.	31337	0
1	35	Alert: The channel %s has notes. Use \002INFO %s\002 to read them. Append the OVERRIDE flag to force a manual fix.	31337	0
1	36	Manual chanfix acknowledged for %s	31337	0
1	37	I see %d opped out of %d total clients in %s.	31337	0
1	38	You may only remove one flag per DELFLAG command.	31337	0
1	39	You cannot delete an owner flag.	31337	0
1	40	Only an owner can delete the user management flag.	31337	0
1	41	You cannot delete a flag from a user in a different group.	31337	0
1	42	You cannot remove a block flag.	31337	0
1	43	You cannot remove a serveradmin flag.	31337	0
1	44	User %s does not have flag %c.	31337	0
1	45	Deleted flag %c of user %s.	31337	0
1	46	You cannot delete a host on a user in a different group.	31337	0
1	47	User %s doesn't have hostmask %s.	31337	0
1	48	Failed deleting hostmask %s from user %s.	31337	0
1	49	Deleted hostmask %s from user %s.	31337	0
1	50	There is no entry in the database for %s.	31337	0
1	51	The channel %s does not have any notes.	31337	0
1	52	An unknown error occured while checking the note id.	31337	0
1	53	There is no such note with that note_id.	31337	0
1	54	No such note %d for channel %s.	31337	0
1	55	Note %d for channel %s was not added by you. You can only delete notes that you added.	31337	0
1	56	Note %d for channel %s is not a manually added note. You can only delete notes that were manually added.	31337	0
1	57	Note %d for channel %s deleted.	31337	0
1	58	You cannot delete an owner.	31337	0
1	59	You cannot delete a user manager.	31337	0
1	60	You cannot delete a user in a different group.	31337	0
1	61	Deleted user %s.	31337	0
1	62	Error deleting user %s.	31337	0
1	63	Channel %s has never been manually fixed.	31337	0
1	64	An unknown error occurred while reading this channel's notes.	31337	0
1	65	Channel %s has been manually fixed on:	31337	0
1	66	%s	31337	0
1	67	End of list.	31337	0
1	68	No information on %s in the database.	31337	0
1	69	Information on %s:	31337	0
1	70	%s is BLOCKED.	31337	0
1	71	%s is being chanfixed.	31337	0
1	72	%s is being autofixed.	31337	0
1	73	Notes (%d):	31337	0
1	74	End of information.	31337	0
1	75	%s is ALERTED.	31337	0
1	76	[%d:%s] %s \002%s\002%s%s	31337	0
1	77	Could not find channel %s on the network.	31337	0
1	78	I am not in %s.	31337	0
1	79	You are already in %s!	31337	0
1	80	Top %d unique op accounts in channel %s:	31337	0
1	81	Found %d unique op accounts in channel %s:	31337	0
1	82	Rank Score Account -- Time first opped / Time last opped	31337	0
1	83	Opped clients on channel %s:	31337	0
1	84	I see 1 opped client in %s.	31337	0
1	85	I see %u opped clients in %s.	31337	0
1	86	Successfully reloaded help tables.	31337	0
1	87	Successfully rehashed configuration file.	31337	0
1	88	Reloading client...see you on the flip side	31337	0
1	89	Score for %s (%s) in channel %s: %u.	31337	0
1	90	~U %s no@such.nick 0	31337	0
1	91	No such nick %s.	31337	0
1	92	None.	31337	0
1	93	Top %u scores for channel %s in the database:	31337	0
1	94	Top %u scores for ops in %s in the database:	31337	0
1	95	Top %u scores for non-ops in %s in the database:	31337	0
1	96	Please use SET NUM_SERVERS <integer number>.	31337	0
1	97	NUM_SERVERS is now %d.	31337	0
1	98	Enabled autofix.	31337	0
1	99	Disabled autofix.	31337	0
1	100	Please use SET ENABLE_AUTOFIX <on/off>.	31337	0
1	101	Enabled manual chanfix.	31337	0
1	102	Disabled manual chanfix.	31337	0
1	103	Please use SET ENABLE_CHANFIX <on/off>.	31337	0
1	104	Enabled channel blocking.	31337	0
1	105	Disabled channel blocking.	31337	0
1	106	Please use SET ENABLE_CHANNEL_BLOCKING <on/off>.	31337	0
1	107	This setting does not exist.	31337	0
1	108	User %s is already in group %s.	31337	0
1	109	Set group %s for user %s.	31337	0
1	110	Shutting down the server as per your request.	31337	0
1	111	Uptime: \002%s\002	31337	0
1	112	Automatic fixing is: \002%s\002	31337	0
1	113	Manual fixing is: \002%s\002	31337	0
1	114	Channel blocking is: \002%s\002	31337	0
1	115	Required amount of servers linked is %u%% of %u, which is a minimum of %u servers.	31337	0
1	116	Splitmode enabled: only %i servers linked.	31337	0
1	117	Splitmode disabled. There are %i servers linked.	31337	0
1	118	You cannot suspend a user in a different group.	31337	0
1	119	User %s is already suspended.	31337	0
1	120	Suspended user %s indefinitely.	31337	0
1	121	The channel %s does not have the ALERT flag.	31337	0
1	122	ALERT flag removed from channel %s	31337	0
1	123	The channel %s is not blocked.	31337	0
1	124	Channel %s has been unblocked.	31337	0
1	125	You cannot unsuspend a user in a different group.	31337	0
1	126	User %s is not currently suspended.	31337	0
1	127	Unsuspended user %s.	31337	0
1	128	Language is set to %s.	31337	0
1	129	I will now send you notices.	31337	0
1	130	I will now send you privmsgs.	31337	0
1	131	Please use USET NOTICE <on/off>.	31337	0
1	132	This setting does not exist.	31337	0
1	134	You can only WHOGROUP on your group.	31337	0
1	135	Users with group %s [username (flags)]:	31337	0
1	136	Number of users: %d.	31337	0
1	137	List of all users:	31337	0
1	138	User: %s, Flags: %s, Group: %s	31337	0
1	139	End of user list.	31337	0
1	140	User: %s	31337	0
1	141	Hosts: %s	31337	0
1	142	Group: %s	31337	0
1	143	Flags: none.	31337	0
1	144	Flags: +%s	31337	0
1	145	Sorry, I do not accept commands during a burst.	31337	0
1	146	Unknown command.	31337	0
1	147	Your access to this service is suspended.	31337	0
1	148	Your current host does not match any registered hosts for your username.	31337	0
1	149	You need to authenticate to use this command.	31337	0
1	150	This command requires flag '%c'.	31337	0
1	151	This command requires one of these flags: \"%s\".	31337	0
1	152	SYNTAX: 	31337	0
\.
				
							