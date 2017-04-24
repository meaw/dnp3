//
// Licensed to Green Energy Corp (www.greenenergycorp.com) under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Green Enery Corp licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//

#ifndef __MASTER_DEMO_H_
#define __MASTER_DEMO_H_
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opendnp3/APL/CommandQueue.h>
#include <opendnp3/APL/FlexibleDataObserver.h>
#include <opendnp3/APL/IOService.h>
#include <opendnp3/APL/IOServiceThread.h>
#include <opendnp3/APL/IPhysicalLayerAsync.h>
#include <opendnp3/APL/Log.h>
#include <opendnp3/APL/LogToStdio.h>
#include <opendnp3/APL/Loggable.h>
#include <opendnp3/APL/PostingNotifierSource.h>
#include <opendnp3/APL/QueueingFDO.h>
#include <opendnp3/APL/TimerSourceASIO.h>

#include <opendnp3/DNP3/AsyncStackManager.h>
#include <opendnp3/DNP3/MasterStackConfig.h>
#include "Tmer.h"
namespace apl { namespace dnp {

/**
 * This class takes care of all the plumbing and bingings between
 * application code and the stack.
 */
class MasterDemoBase
	: private IOService
	, public IOServiceThread
	, public IResponseAcceptor
{
public:
	/**
	 * Default constructor for the object.
	 */
	MasterDemoBase(Logger* apLogger);

	/**
	 * Tells the io_service to exit.
	 */
	void Shutdown();

	/**
	 * Returns the data observer for the stack.
	 */
	IDataObserver* GetDataObserver()
	{
		return &mFDO;
	}

	ICommandAcceptor* GetCommandAcceptor()
	{
		return mpCommandAcceptor;
	}

	void SetCommandAcceptor(ICommandAcceptor* apCommandAcceptor)
	{
		mpCommandAcceptor = apCommandAcceptor;
	}

	void AcceptResponse(const CommandResponse& aResponse,
			int aSequence);
	void Accessor(int counter);
private:
	/**
	 * A notifier that gets triggered when data updates are
	 * received.
	 */
	void ResetStartupTasks();
	void OnDataUpdate();
	/**
	 * The QueueingFDO object which is responsible for keeping track
	 * of new data updates.
	 */
	QueueingFDO mFDO;

	/**
	 * The ICommandAcceptor instance created for the stack.
	 */
	ICommandAcceptor* mpCommandAcceptor;

	PostingNotifierSource mPostSource;	// bridge between Notifiers and IO services that marshalls method calls between threads
	TimerSourceASIO mTimerSource;		// boost timer source, interface into the io service system
	ITimer* mpInfiniteTimer;		// timer used to keep the boost io service alive

	int16_t mTestCounter;
};


/**
 * Something, not sure what yet.
 */
class MasterDemoApp : public MasterDemoBase, public IStackObserver
{
public:
	void Tick();
	void Timer(int interval);
	int counter;
	Tmer t;
	MasterDemoApp(Logger* apLogger);

	void OnStateChange(StackStates aState);
};

}} // end namespaces apl and apl::dnp

#endif

