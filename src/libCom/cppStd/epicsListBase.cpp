/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
// epicsListBase.cpp
//	Author:	Andrew Johnson
//	Date:	October 2000

#ifdef __GNUG__
  #pragma implementation
#endif

#define epicsExportSharedSymbols
#include "epicsListBase.h"
#include "cantProceed.h"

// epicsListNodePool
epicsMutex epicsListNodePool::_mutex;
epicsListLink epicsListNodePool::_store;

epicsShareFunc epicsShareAPI epicsListNodePool::~epicsListNodePool() {
    while (_blocks.hasNext()) {
	epicsListNodeBlock* block = 
		static_cast<epicsListNodeBlock*>(_blocks.extract());
	block->reset();
	_mutex.lock();
	_store.append(block);
	_mutex.unlock();
    }
}

epicsShareFunc void epicsShareAPI epicsListNodePool::extend() {
    assert(!_free.hasNext());
    epicsListNodeBlock* block = 0;
    if (_store.hasNext()) {
	_mutex.lock();
	if (_store.hasNext())
	    block = static_cast<epicsListNodeBlock*>(_store.extract());
	_mutex.unlock();
    }
    if (block == 0)
	block = new epicsListNodeBlock;
    if (block == 0) // in case new didn't throw...
	cantProceed("epicsList: out of memory");
    _blocks.append(block);
    _free.set(block->first());
}
