/**
 * chanfix.cc
 * 
 * Copyright (C) 2003	Reed Loden <reed@reedloden.com>
 *			Matthias Crauwels <ultimate_@wol.be>
 *
 * Automatically and manually fix opless and taken over channels
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
 * $Id$
 */

#include        <new>

#include	"config.h"
#include	"client.h"
#include	"EConfig.h"
#include	"Network.h"
#include	"server.h"
#include        "StringTokenizer.h"

#include	"chanfix.h"
#include	"chanfixCommands.h"
#include	"functor.h"
#include	"levels.h"
#include	"sqlChanOp.h"
#include	"sqlUser.h"
#include	"Timer.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::endl;
using std::stringstream;
using std::ends;


/*
 *  Exported function used by moduleLoader to gain an
 *  instance of this module.
 */

extern "C"
{
  xClient* _gnuwinit(const string& args)
  { 
    return new chanfix( args );
  }

} 
 
/**
 * This constructor calls the base class constructor.  The xClient
 * constructor will open the configuration file given and retrieve
 * basic client info (nick/user/host/etc).
 * Any additional processing must be done here.
 */
chanfix::chanfix( const string& configFileName )
 : xClient( configFileName )
{
/* Load the config file */
chanfixConfig = new (std::nothrow) EConfig(configFileName);
assert(chanfixConfig != 0);

/* Config file processing */
consoleChan = chanfixConfig->Require("consoleChan")->second ;
consoleChanModes = chanfixConfig->Require("consoleChanModes")->second ;
operChan = chanfixConfig->Require("operChan")->second ;
operChanModes = chanfixConfig->Require("operChanModes")->second ;
supportChan = chanfixConfig->Require("supportChan")->second ;
supportChanModes = chanfixConfig->Require("supportChanModes")->second ;
enableAutoFix = atob(chanfixConfig->Require("enableAutoFix")->second) ;
enableChanFix = atob(chanfixConfig->Require("enableChanFix")->second) ;
enableChannelBlocking = atob(chanfixConfig->Require("enableChannelBlocking")->second) ;
numServers = atoi((chanfixConfig->Require("numServers")->second).c_str()) ;
minServersPresent = atoi((chanfixConfig->Require("minServersPresent")->second).c_str()) ;
numTopScores = atoi((chanfixConfig->Require("numTopScores")->second).c_str()) ;
minClients = atoi((chanfixConfig->Require("minClients")->second).c_str()) ;
clientNeedsIdent = atob(chanfixConfig->Require("clientNeedsIdent")->second) ;
clientNeedsReverse = atob(chanfixConfig->Require("clientNeedsReverse")->second) ;

/* Initial state */
currentState = INIT;

/* Database processing */
sqlHost = chanfixConfig->Require("sqlHost")->second;
sqlPort = chanfixConfig->Require("sqlPort")->second;
sqlDB = chanfixConfig->Require("sqlDB")->second;
sqlUser = chanfixConfig->Require("sqlUser")->second;
sqlPass = chanfixConfig->Require("sqlPass")->second;

elog    << "chanfix::chanfix::> Configuration loaded!"
        << endl;

string Query = "host=" + sqlHost + " dbname=" + sqlDB + " port=" + sqlPort + " user=" + sqlUser;

elog    << "chanfix::chanfix> Attempting to connect to "
        << sqlHost << " at port " << sqlPort
        << " as User " << sqlUser << " to database: "
        << sqlDB
        << endl;

SQLDb = new (std::nothrow) cmDatabase( Query.c_str() ) ;
assert( SQLDb != 0 ) ;
//-- Make sure we connected to the SQL database; if
// we didn't we exit entirely.
if (SQLDb->ConnectionBad ())
        {
        elog    << "chanfix::chanfix> Unable to connect to SQL server."
                << endl
                << "chanfix::chanfix> PostgreSQL error message: "
                << SQLDb->ErrorMessage()
                << endl ;

        ::exit( 0 ) ;
        }
else
        {
        elog    << "chanfix::chanfix> Connection established to SQL server"
                << endl ;
        }

/* Register the commands we want to use */
RegisterCommand(new QUOTECommand(this, "QUOTE", "<text>"));
RegisterCommand(new SHUTDOWNCommand(this, "SHUTDOWN", "[reason]"));
RegisterCommand(new INVITECommand(this, "INVITE", ""));
RegisterCommand(new RELOADCommand(this, "RELOAD", ""));

/* Preload the ChanOps cache */
preloadChanOpsCache();

/* Set up our timer. */
theTimer = new Timer();

}

chanfix::~chanfix()
{
}

/* Register a new command */
bool chanfix::RegisterCommand( Command *theCommand )
{
UnRegisterCommand( theCommand->getName() ) ;
return commandMap.insert( commandMapType::value_type(theCommand->getName(), theCommand)).second;
}

/* UnRegister a command */
bool chanfix::UnRegisterCommand( const string& commName )
{
commandMapType::iterator ptr = commandMap.find( commName ) ;
if ( ptr == commandMap.end() ) {
  return false ;
}
delete ptr->second ;
commandMap.erase( ptr ) ;
return true ;
}

/* OnAttach */
void chanfix::OnAttach()
{
for (commandMapType::iterator ptr = commandMap.begin(); ptr != commandMap.end(); ++ptr) {
  ptr->second->setServer(MyUplink);
}

/* Register for global network events */
MyUplink->RegisterEvent( EVT_KILL, this );
MyUplink->RegisterEvent( EVT_QUIT, this );
MyUplink->RegisterEvent( EVT_BURST_ACK, this );
MyUplink->RegisterEvent( EVT_NETJOIN, this );
MyUplink->RegisterEvent( EVT_NETBREAK, this );

/* Register for all channel events */
MyUplink->RegisterChannelEvent( xServer::CHANNEL_ALL, this );

/* Set up delays */
// TODO: move this to chanfix_config.h
checkOpsDelay = 300; 
updateDelay = 3600;

/* Start timers */
tidCheckOps = MyUplink->RegisterTimer(time(NULL) + checkOpsDelay, this, NULL);
tidAutoFix = MyUplink->RegisterTimer(time(NULL) + AUTOFIX_INTERVAL, this, NULL);
tidUpdateDB = MyUplink->RegisterTimer(time(NULL) + updateDelay, this, NULL);

/* When attached state should be RUN */
changeState(RUN);

xClient::OnAttach() ;
}

/* OnDetach */
void chanfix::OnDetach( const string& reason )
{
/* Delete our config */
delete chanfixConfig; chanfixConfig = 0;

/* Delete our connection to the database */
delete SQLDb; SQLDb = 0;

/* Delete our timer */
delete theTimer; theTimer = 0;

/* Delete our commands */
for(commandMapType::iterator ptr = commandMap.begin() ;
    ptr != commandMap.end() ; ++ptr) {
        delete ptr->second;
}
commandMap.clear();

xClient::OnDetach( reason ) ;
}

/* OnConnect */
void chanfix::OnConnect()
{
xClient::OnConnect() ;
}

/* OnDisconnect */
void chanfix::OnDisconnect()
{
xClient::OnDisconnect() ;
}

void chanfix::OnTimer(xServer::timerID theTimer, void*)
{
time_t theTime;

if (theTimer == tidCheckOps) {
  CheckOps();

  /* Refresh Timer */
  theTime = time(NULL) + checkOpsDelay;
  tidCheckOps = MyUplink->RegisterTimer(theTime, this, NULL);
  }
else if (theTimer == tidAutoFix) {
  AutoFix();

  /* Refresh Timer */
  theTime = time(NULL) + AUTOFIX_INTERVAL;
  tidAutoFix = MyUplink->RegisterTimer(theTime, this, NULL);
  }
else if (theTimer == tidUpdateDB) {
  UpdateOps();

  /* Refresh Timer */
  theTime = time(NULL) + updateDelay;
  tidUpdateDB = MyUplink->RegisterTimer(theTime, this, NULL);
  }
}

void chanfix::OnPrivateMessage( iClient* theClient,
	const string& Message, bool secure)
{
if (!theClient->isOper()) {
  return;
}

if( currentState == BURST ) {
  Notice(theClient, "I'm in BURST-mode! Please try again later!");
  return;
}

StringTokenizer st(Message) ;
if ( st.empty() ) {
  return;
}

const string Command = string_upper(st[0]);

/*
 *  Just quickly, abort if someone tries to LOGIN or NEWPASS
 *  unsecurely.
 */
if (!secure && ((Command == "LOGIN") || (Command == "NEWPASS"))) {
  Notice(theClient, "To use %s, you must /msg %s@%s",
	 Command.c_str(), nickName.c_str(), getUplinkName().c_str());
  return ;
}

commandMapType::iterator commHandler = commandMap.find(Command);
if(commHandler == commandMap.end()) {
  return ;
}

commHandler->second->Exec(theClient, Message);

xClient::OnPrivateMessage(theClient, Message);
}

void chanfix::OnCTCP( iClient* theClient, const string& CTCP,
		const string& Message, bool Secure )
{
StringTokenizer st(CTCP);

if(st.empty()) return;

const string Command = string_upper(st[0]);

if (Command == "DCC") {
  DoCTCP(theClient, CTCP, "REJECT");
} else if (Command == "PING" || Command == "ECHO") {
  DoCTCP(theClient, CTCP, Message);
} else if (Command == "VERSION") {
  DoCTCP(theClient, CTCP, "evilnet development - chanfix v1.0.0");
}

xClient::OnCTCP(theClient, CTCP, Message, Secure);
}

// Burst any channels.
bool chanfix::BurstChannels()
{
Join(consoleChan, consoleChanModes, 0, true);
Join(operChan, operChanModes, 0, true);
Join(supportChan, supportChanModes, 0, true);

return xClient::BurstChannels();
}

/* OnChannelEvent */
void chanfix::OnChannelEvent( const channelEventType& whichEvent,
        Channel* theChan,
        void* data1, void* data2, void* data3, void* data4 )
{
iClient* theClient = 0;

/* If we are not running, we don't want to be giving points. */
if (currentState != RUN) return;

/* If this channel is too small, don't worry about it. */
if (theChan->size() < minClients) return;

switch( whichEvent )
        {
        case EVT_CREATE:
        case EVT_JOIN:
		{
                theClient = static_cast< iClient* >( data1 ) ;

                if( theClient->isOper() && theChan->getName() == operChan)
                        {
                        Op( theChan, theClient ) ;
                        }
                break ;
		}
	case EVT_PART:
		{
                theClient = static_cast< iClient* >( data1 ) ;
		if(wasOpped(theClient, theChan))
			{
			givePoint(theClient, theChan);
			}
		break ;
		}
        default:
                break ;
        }

xClient::OnChannelEvent( whichEvent, theChan,
        data1, data2, data3, data4 ) ;
}

/* OnEvent */
void chanfix::OnEvent( const eventType& whichEvent,
        void* data1, void* data2, void* data3, void* data4 )
{
switch(whichEvent)
	{
	case EVT_BURST_ACK:
		{
		changeState(RUN);
		break;
		}
	case EVT_NETJOIN:
	case EVT_NETBREAK:
		{
		CheckNet();
		break;
		}

	}
xClient::OnEvent( whichEvent, data1, data2, data3, data4 ) ;
}

void chanfix::preloadChanOpsCache()
{
        stringstream theQuery;
        theQuery        << "SELECT channel,userhost,last_seen_as,points,account FROM chanOps"
                                << ends;

        elog            << "*** [chanfix::preloadChanOpsCache]: Loading chanOps and their points" 
                                << endl;

        ExecStatusType status = SQLDb->Exec(theQuery.str().c_str()) ;

        if( PGRES_TUPLES_OK == status )
        {
                for (int i = 0 ; i < SQLDb->Tuples(); i++)
                        {
                                sqlChanOp* newOp = new (std::nothrow) sqlChanOp(SQLDb);
                                assert( newOp != 0 ) ;

                                newOp->setAllMembers(i);
				pair<string, string> thePair (newOp->getAccount(), newOp->getChannel());
                                sqlChanOps.insert(sqlChanOpsType::value_type(thePair, newOp));
                        }
        } else	{
		elog << "[chanfix::preloadChanOpsCache] Something went wrong: "
        	        << SQLDb->ErrorMessage()
                	<< endl;
	}

        elog    << "*** [chanfix::preloadChanOpsCache]: Done. Loaded "
                        << SQLDb->Tuples()
                        << " chanops."
                        << endl;
}

void chanfix::changeState(CHANFIX_STATE newState)
{
if (currentState == newState) return;

/* Start our own timer just for ourself */
Timer stateTimer;
stateTimer.Start();

/* First, do what we need to exit our current state */
switch( currentState ) {
	case BURST:
	{
	elog	<< "chanfix::changeState> Exiting state BURST"
		<< endl;
	break;
	}
	case RUN:
	{
	elog	<< "chanfix::changeState> Exiting state RUN"
		<< endl;
	}
        case SPLIT:
        {
        elog    << "chanfix::changeState> Exiting state SPLIT"
                << endl;
        }
        case INIT:
        {
        elog    << "chanfix::changeState> Exiting state INIT"
                << endl;
        }

}

currentState = newState;

switch( currentState ) {
	case BURST:
	{
	elog	<< "chanfix::changeState> Entering state BURST"
		<< endl;
	BurstOps();
	break;
	}
	case RUN:
	{
	elog	<< "chanfix::changeState> Entering state RUN"
		<< endl;
	CheckOps();
	break;
	}
        case SPLIT:
        {
        elog    << "chanfix::changeState> Entering state SPLIT"
                << endl;
        break;
        }
        case INIT:
        {
        elog    << "chanfix::changeState> Entering state INIT"
                << endl;
        break;
        }

}

elog	<< "Changed state in: "
	<< stateTimer.stopTimeMS()
	<< "ms"
	<< endl;
}

sqlChanOp* chanfix::findChanOp(const string& account, const string& channel)
{

elog << "DEBUG: Searching ..." << endl;
sqlChanOpsType::iterator ptr = sqlChanOps.find(pair<string,string>(account, channel));
if(ptr != sqlChanOps.end())
	{
	elog << "DEBUG: We've got a winner: " << account << " on " << channel << "!!" << endl;
        return ptr->second ;
	}

return 0;
}

sqlChanOp* chanfix::newChanOp(const string& account, const string& channel)
{
sqlChanOp* newOp = new (std::nothrow) sqlChanOp(SQLDb);
assert( newOp != 0 ) ;

sqlChanOps.insert(sqlChanOpsType::value_type(pair<string,string>(account, channel), newOp));
elog << "DEBUG: Added new operator: " << account << " on " << channel << "!!" << endl;

newOp->setAccount(account);
newOp->setChannel(channel);
newOp->Insert();

return newOp;
}

sqlChanOp* chanfix::findChanOp(iClient* theClient, Channel* theChan) 
{
return findChanOp(theClient->getAccount(), theChan->getName());
}

sqlChanOp* chanfix::newChanOp(iClient* theClient, Channel* theChan)
{
return newChanOp(theClient->getAccount(), theChan->getName());
}

const string gnuworld::escapeSQLChars(const string& theString)
{
string retMe ;

for( string::const_iterator ptr = theString.begin() ;
        ptr != theString.end() ; ++ptr )
        {
        if( *ptr == '\'' )
                {
                retMe += "\\\047" ;
                }
        else if ( *ptr == '\\' )
                {
                retMe += "\\\134" ;
                }
        else
                {
                retMe += *ptr ;
                }
        }
return retMe ;
}

void chanfix::BurstOps()
{
if(currentState != BURST) return;
elog << "DEBUG: Bursting Ops ..." << endl;

Channel* thisChan;
ChannelUser* curUser;
for(xNetwork::channelIterator ptr = Network->channels_begin(); ptr != Network->channels_end(); ptr++) {
        thisChan = ptr->second;
	for(Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
		curUser = ptr->second;
		if(curUser->isModeO() && !curUser->getClient()->getMode(iClient::MODE_SERVICES) && 
			curUser->getClient()->getAccount() != "")
			{
			lastOps.push_back(pair<string, string>(curUser->getClient()->getAccount(), thisChan->getName()));
			gotOpped(curUser->getClient(), thisChan);
			}
		}
        }

}

void chanfix::CheckOps()
{
if(currentState != RUN) return;
elog << "DEBUG: Checking Ops ..." << endl;

Channel* thisChan;
ChannelUser* curUser;
lastOpsType curOps;

for(xNetwork::channelIterator ptr = Network->channels_begin(); ptr != Network->channels_end(); ptr++) {
        thisChan = ptr->second;
	if(thisChan->size() > minClients) 
		{
	        for(Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
        	        curUser = ptr->second;
                	if(curUser->isModeO() && !curUser->getClient()->getMode(iClient::MODE_SERVICES) &&
                        	curUser->getClient()->getAccount() != "")
	                        {
				curOps.push_back(pair<string, string>(curUser->getClient()->getAccount(), thisChan->getName()));
				if(wasOpped(curUser->getClient(), thisChan))
					 givePoint(curUser->getClient(), thisChan);
				else gotOpped(curUser->getClient(), thisChan);
        	                }
			}
                }
        }
lastOps = curOps;
}

void chanfix::givePoint(iClient* thisClient, Channel* thisChan)
{
sqlChanOp* thisOp = findChanOp(thisClient, thisChan);

thisOp->addPoint(); 
thisOp->setLastSeenAs(thisClient->getNickUserHost()); 
//thisOp->Update();

elog << "DEBUG: Gave " << thisOp->getAccount() 
	<< " on " << thisOp->getChannel() 
	<< " a point" 
	<< endl;

}

void chanfix::givePoint(sqlChanOp* thisOp)
{

thisOp->addPoint(); 
//thisOp->Update();

elog << "DEBUG: Gave " << thisOp->getAccount()
        << " on " << thisOp->getChannel()
        << " a point"
        << endl;
}

void chanfix::gotOpped(iClient* thisClient, Channel* thisChan)
{

sqlChanOp* thisOp = findChanOp(thisClient, thisChan);
if(!thisOp) thisOp = newChanOp(thisClient, thisChan);

thisOp->setTimeOpped(::time(0));
thisOp->setLastSeenAs(thisClient->getNickUserHost());
}

bool chanfix::wasOpped(iClient* thisClient, Channel* thisChan)
{
pair<string, string> curPair; 
for(lastOpsType::iterator ptr = lastOps.begin(); ptr != lastOps.end(); ptr++)
	{
	curPair = *ptr;
	if(curPair.first == thisClient->getAccount() && curPair.second == thisChan->getName())
		{
		elog << "DEBUG: " << thisClient->getAccount() << " was opped on " << thisChan->getName() << endl;
		return true;
		}
	}
elog << "DEBUG: " << thisClient->getAccount() << " was not opped on " << thisChan->getName() << endl;
return false;
}

void chanfix::CheckNet()
{
if(100 * Network->serverList_size() < numServers * minServersPresent)
	{
	elog << "DEBUG: Not enough servers linked! Going to SPLIT-state" << endl;
	changeState(SPLIT);
	return;
	}

if(currentState == SPLIT)
	{
        elog << "DEBUG: Enough servers linked! Going to BURST-state" << endl;
        changeState(BURST);
        return;

	}
}

void chanfix::AutoFix()
{
if( currentState != RUN ) {
  elog << "DEBUG: skipping autofix!" << endl;
  return;
}

Channel* thisChan;
ChannelUser* curUser;
for(xNetwork::channelIterator ptr = Network->channels_begin(); ptr != Network->channels_end(); ptr++) {
        thisChan = ptr->second;
	bool opLess = true;
	bool hasService = false;
        if(thisChan->size() > minClients)
		{
                for(Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
                        curUser = ptr->second;
                        if(curUser->isModeO()) opLess = false;
			if(curUser->getClient()->getMode(iClient::MODE_SERVICES)) hasService = true;
			}
		if(opLess && !hasService)
			{
			elog << "DEBUG: Autofix " << thisChan->getName() << "!" << endl;
			//autoFixChan(thisChan);
			}
		}
	}
}

void chanfix::ManFix(Channel* thisChan)
{
ChannelUser* curUser;
bool opLess = true;
bool hasService = false;

if(thisChan->size() > minClients)
	{
	for(Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
		curUser = ptr->second;
		if(curUser->isModeO()) opLess = false;
		if(curUser->getClient()->getMode(iClient::MODE_SERVICES)) hasService = true;
		}
	if(opLess && !hasService)
                {
                elog << "DEBUG: Manual fix " << thisChan->getName() << "!" << endl;
                //manFixChan(thisChan);
                }
	}

}

void chanfix::UpdateOps()
{
elog << "DEBUG: Updating SQL ..." << endl;

for(sqlChanOpsType::iterator ptr = sqlChanOps.begin(); ptr != sqlChanOps.end(); ptr++)
	ptr->second->Update();
}

void Command::Usage( iClient* theClient )
{
bot->Notice( theClient, string( "SYNTAX: " ) + getInfo() ) ;
}

} // namespace gnuworld

