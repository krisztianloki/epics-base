/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/*
 *      $Id$
 *
 *      Author  Jeffrey O. Hill
 *              johill@lanl.gov
 *              505 665 1831
 */

#define CAS_VERSION_GLOBAL

#include "epicsGuard.h"

#include "addrList.h"

#define caServerGlobal
#include "server.h"
#include "casCtxIL.h" // casCtx in line func

//
// the maximum beacon period if EPICS_CA_BEACON_PERIOD isnt available
//
static const double CAServerMaxBeaconPeriod = 15.0; // seconds

//
// the initial beacon period
//
static const double CAServerMinBeaconPeriod = 1.0e-3; // seconds

//
// caServerI::caServerI()
//
caServerI::caServerI (caServer &tool) :
    //
    // Set up periodic beacon interval
    // (exponential back off to a plateau
    // from this intial period)
    //
    beaconPeriod (CAServerMinBeaconPeriod),
    adapter (tool),
    debugLevel (0u),
    nEventsProcessed (0u),
    nEventsPosted (0u),
    beaconCounter (0u)
{
	caStatus status;
	double maxPeriod;

	assert (&adapter != NULL);

    //
    // create predefined event types
    //
    this->valueEvent = registerEvent ("value");
	this->logEvent = registerEvent ("log");
	this->alarmEvent = registerEvent ("alarm");
	
	status = envGetDoubleConfigParam (&EPICS_CA_BEACON_PERIOD, &maxPeriod);
	if (status || maxPeriod<=0.0) {
		this->maxBeaconInterval = CAServerMaxBeaconPeriod;
		errlogPrintf (
			"EPICS \"%s\" float fetch failed\n", EPICS_CA_BEACON_PERIOD.name);
		errlogPrintf (
			"Setting \"%s\" = %f\n", EPICS_CA_BEACON_PERIOD.name, 
			this->maxBeaconInterval);
	}
	else {
		this->maxBeaconInterval = maxPeriod;
	}

    this->locateInterfaces ();

	if (this->intfList.count()==0u) {
		errMessage (S_cas_noInterface, 
            "- CA server internals init unable to continue");
        throw S_cas_noInterface;
	}

	return;
}

/*
 * caServerI::~caServerI()
 */
caServerI::~caServerI()
{
    epicsGuard < epicsMutex > locker ( this->mutex );

	//
	// delete all clients
	//
	tsDLIter <casStrmClient> iter = this->clientList.firstIter ();
    while ( iter.valid () ) {
		tsDLIter <casStrmClient> tmp = iter;
		++tmp;
		//
		// destructor takes client out of list
		//
		iter->destroy ();
		iter = tmp;
	}

	casIntfOS *pIF;
	while ( (pIF = this->intfList.get()) ) {
		delete pIF;
	}
}

//
// caServerI::installClient()
//
void caServerI::installClient(casStrmClient *pClient)
{
    epicsGuard < epicsMutex > locker ( this->mutex );
	this->clientList.add(*pClient);
}

//
// caServerI::removeClient()
//
void caServerI::removeClient (casStrmClient *pClient)
{
    epicsGuard < epicsMutex > locker ( this->mutex );
	this->clientList.remove (*pClient);
}

//
// caServerI::connectCB()
//
void caServerI::connectCB ( casIntfOS & intf )
{
    casStreamOS * pClient = intf.newStreamClient ( *this );
    if ( pClient ) {
        pClient->sendVersion ();
        pClient->flush ();
    }
}

//
// caServerI::advanceBeaconPeriod()
//
// compute delay to the next beacon
//
void caServerI::advanceBeaconPeriod()
{
	//
	// return if we are already at the plateau
	//
	if (this->beaconPeriod >= this->maxBeaconInterval) {
		return;
	}

	this->beaconPeriod += this->beaconPeriod;

	if (this->beaconPeriod >= this->maxBeaconInterval) {
		this->beaconPeriod = this->maxBeaconInterval;
	}
}

//
// casVerifyFunc()
//
void casVerifyFunc (const char *pFile, unsigned line, const char *pExp)
{
    fprintf(stderr, "the expression \"%s\" didnt evaluate to boolean true \n",
        pExp);
    fprintf(stderr,
        "and therefore internal problems are suspected at line %u in \"%s\"\n",
        line, pFile);
    fprintf(stderr,
        "Please forward above text to johill@lanl.gov - thanks\n");
}

//
// serverToolDebugFunc()
// 
void serverToolDebugFunc (const char *pFile, unsigned line, const char *pComment)
{
    fprintf (stderr,
"Bad server tool response detected at line %u in \"%s\" because \"%s\"\n",
                line, pFile, pComment);
}

//
// caServerI::attachInterface()
//
caStatus caServerI::attachInterface (const caNetAddr &addr, bool autoBeaconAddr,
                            bool addConfigBeaconAddr)
{
    casIntfOS *pIntf;
    
    pIntf = new casIntfOS (*this, addr, autoBeaconAddr, addConfigBeaconAddr);
    if (pIntf==NULL) {
        return S_cas_noMemory;
    }
    
    {
        epicsGuard < epicsMutex > locker ( this->mutex );
        this->intfList.add (*pIntf);
    }

    return S_cas_success;
}


//
// caServerI::sendBeacon()
// (implemented here because this has knowledge of the protocol)
//
void caServerI::sendBeacon()
{
	//
	// send a broadcast beacon over each configured
	// interface unless EPICS_CA_AUTO_ADDR_LIST specifies
	// otherwise. Also send a beacon to all configured
	// addresses.
	// 
    {
        epicsGuard < epicsMutex > locker ( this->mutex );
	    tsDLIter <casIntfOS> iter = this->intfList.firstIter ();
	    while ( iter.valid () ) {
		    iter->sendBeacon ( this->beaconCounter );
		    iter++;
	    }
    }

    this->beaconCounter++;
 
	//
	// double the period between beacons (but dont exceed max)
	//
	this->advanceBeaconPeriod();
}

//
// caServerI::getBeaconPeriod()
//
double caServerI::getBeaconPeriod() const 
{ 
    return this->beaconPeriod; 
}

//
// caServerI::show()
//
void caServerI::show (unsigned level) const
{
    int bytes_reserved;
    
    printf( "Channel Access Server Status V%s\n",
        CA_VERSION_STRING ( CA_MINOR_PROTOCOL_REVISION ) );
    
    this->mutex.show(level);
    
    {
        epicsGuard < epicsMutex > locker ( this->mutex );
        tsDLIterConst<casStrmClient> iterCl = this->clientList.firstIter ();
        while ( iterCl.valid () ) {
            iterCl->show (level);
            ++iterCl;
        }
    
        tsDLIterConst<casIntfOS> iterIF = this->intfList.firstIter ();
        while ( iterIF.valid () ) {
            iterIF->casIntfOS::show ( level );
            ++iterIF;
        }
    }
    
    bytes_reserved = 0u;
#if 0
    bytes_reserved += sizeof(casClient) *
        ellCount(&this->freeClientQ);
    bytes_reserved += sizeof(casChannel) *
        ellCount(&this->freeChanQ);
    bytes_reserved += sizeof(casEventBlock) *
        ellCount(&this->freeEventQ);
    bytes_reserved += sizeof(casAsyncIIO) *
        ellCount(&this->freePendingIO);
#endif
    if (level>=1) {
        printf(
            "There are currently %d bytes on the server's free list\n",
            bytes_reserved);
#if 0
        printf(
            "%d client(s), %d channel(s), %d event(s) (monitors), and %d IO blocks\n",
            ellCount(&this->freeClientQ),
            ellCount(&this->freeChanQ),
            ellCount(&this->freeEventQ),
            ellCount(&this->freePendingIO));
#endif
        printf( 
            "The server's integer resource id conversion table:\n");
        {
            epicsGuard < epicsMutex > locker ( this->mutex );
            this->chronIntIdResTable<casRes>::show(level);
        }
    }
        
    return;
}

//
// casEventRegistry::~casEventRegistry()
//
// (must not be inline because it is virtual)
//
casEventRegistry::~casEventRegistry()
{
    this->traverse ( &casEventMaskEntry::destroy );
}

