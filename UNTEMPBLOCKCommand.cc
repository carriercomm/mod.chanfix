/**
 * UNTEMPBLOCKCommand.cc
 *
 * 09/26/2006 - Neil Spierling <sirvulcan@gmail.com>
 * Initial Version
 *
 * Removes the temp block on a channel
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
 * $Id: UNBLOCKCommand.cc 1553 2006-04-18 04:03:59Z sirvulcan $
 */

#include "gnuworld_config.h"

#include "chanfix.h"
#include "responses.h"
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlUser.h"

RCSTAG("$Id: UNBLOCKCommand.cc 1553 2006-04-18 04:03:59Z sirvulcan $");

namespace gnuworld
{
namespace cf
{

void UNTEMPBLOCKCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

/* Check if channel blocking has been disabled in the config. */
if (!bot->doChanBlocking()) {
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::channel_blocking_disabled,
                              std::string("Channel blocking is disabled.")).c_str());
  return;
}
	
sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) {
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::no_entry_in_db,
                              std::string("There is no entry in the database for %s.")).c_str(),
                                          st[1].c_str());
  return;
}

if (!bot->isTempBlocked(theChan->getChannel())) {
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::channel_not_temp_blocked,
                              std::string("The channel %s is not temp blocked.")).c_str(),
                                          theChan->getChannel().c_str());
  return;
}

/* Remove from temp block list */
bot->tempBlockList.erase(theChan->getChannel());

/* Add note to the channel about this command */
theChan->addNote(sqlChannel::EV_UNTEMPBLOCK, theClient, "");

bot->SendTo(theClient,
            bot->getResponse(theUser,
                            language::channel_untempblocked,
                            std::string("Channel %s has been untempblocked.")).c_str(),
                                        theChan->getChannel().c_str());

/* Log command */
bot->logAdminMessage("%s (%s) UNTEMPBLOCK %s",
		     theUser->getUserName().c_str(),
		     theClient->getRealNickUserHost().c_str(),
		     theChan->getChannel().c_str());

bot->logLastComMessage(theClient, Message);

return;
}
} // namespace cf
} // namespace gnuworld