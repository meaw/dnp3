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

#include "SlaveDemo.h"

using namespace std;

namespace apl
{
namespace dnp
{

SlaveDemoBase::SlaveDemoBase(Logger* apLogger) :
	IOService(),
	IOServiceThread(apLogger, this->Get()),
	mTimerSource(this->Get()),
	mpInfiniteTimer(mTimerSource.StartInfinite())
{
	// Create a notifier that when called will post a call to OnCommandNotify
	INotifier* pNotifier = mPostSource.Get(boost::bind(&SlaveDemoBase::OnCommandNotify, this), &mTimerSource);

	// Hand the notifier to the command source, so this happens whenever a new command is present
	mCommandQueue.SetNotifier(pNotifier);




}
void SlaveDemoApp::Timer(int offset) {

	auto tid = t.create(5000, 1000, std::bind(&SlaveDemoApp::Tick, this));
	counter = offset;
}

void SlaveDemoApp::Tick() {
	counter++;
	std::cout << "Update "<< counter <<" "<<name<< std::endl;
	Binary b((counter & 0x0001), BQ_ONLINE);
	b.SetToNow();
	Counter c(counter, CQ_ONLINE);
	c.SetToNow();
	Analog a(counter*1.0, CQ_ONLINE);
	c.SetToNow();
	// We would like all updates to be sent at one time.When the Transaction object
	// goes out of scope, all updates will be sent in one block to do the slave database.
	Transaction t1(mpObserver1);
	mpObserver1->Update(a, 0);
	mpObserver1->Update(c, 0);
	mpObserver1->Update(b, 0);

	Transaction t2(mpObserver2);
	mpObserver2->Update(a, 0);
	mpObserver2->Update(c, 0);
	mpObserver2->Update(b, 0);
}

void SlaveDemoBase::Shutdown()
{
	// this is the only outstanding event, so this will cause the
	// io_service thread to exit cleanly
	mpInfiniteTimer->Cancel();
}

void SlaveDemoBase::OnCommandNotify()
{
	// execute a single command
	mCommandQueue.ExecuteCommand(this);
}




 SlaveDemoApp::SlaveDemoApp(Logger* apLogger,char sname[]) :
	SlaveDemoBase(apLogger),
	mCountSetPoints(0),
	mCountBinaryOutput(0),
	mpObserver1(NULL), mpObserver2(NULL) {
	 sprintf(name, "%s",sname);
	// Timer fires every second, starting now    
//	
}





void SlaveDemoApp::SetDataObserver(IDataObserver* apObserver, int instance)
{ if (instance==1){
	mpObserver1 = apObserver;}
if (instance == 2) {
	mpObserver2 = apObserver;
}
}

CommandStatus SlaveDemoApp::HandleControl(Setpoint& aControl, size_t aIndex)
{
	LOG_BLOCK(LEV_APP, "Received " << aControl.ToString() << " on index: " << aIndex);


	/*
	// Update a  feedback point that has the same value as the setpoint we were
	// given from the master. Configure it with the current time and good quality
	Analog a(aControl.GetValue(), AQ_ONLINE);
	a.SetToNow();

	// Create an additional counter to let the master know how many setpoints
	// we've receieved
	Counter c(++mCountSetPoints, CQ_ONLINE);
	c.SetToNow();


	Binary b((mCountSetPoints & 00001), BQ_ONLINE);
	b.SetToNow();

	// We would like all updates to be sent at one time.When the Transaction object
	// goes out of scope, all updates will be sent in one block to do the slave database.
	Transaction t(mpObserver);

	// Push the prepared datapoints to the database of this slave. The slave
	// can now transmit the changes to the master (polling or unsol)
	mpObserver->Update(a, aIndex);
	mpObserver->Update(c, 1);
	mpObserver->Update(b, 0);
	*/

	// This is the control code returned to the slave stack, and forwared
	// on to the master. These are DNP3 control codes.
	int xv = aControl.GetValue();
	if (xv % 100 == 0) {
		return  CS_TIMEOUT;
	}else{
	
	return CS_SUCCESS;}
}

// same as for the setpoint
CommandStatus SlaveDemoApp::HandleControl(BinaryOutput& aControl, size_t aIndex)
{
	LOG_BLOCK(LEV_APP, "Received " << aControl.ToString() << " on index: " << aIndex);
	
	// set the binary to ON if the command  code was LATCH_ON, otherwise set it off (LATCH_OFF)
	apl::Binary b(aControl.GetCode() == CC_LATCH_ON, BQ_ONLINE);
	b.SetToNow();
    /*
	// count how many BinaryOutput commands we recieve
	apl::Counter c(++mCountBinaryOutput, CQ_ONLINE);
	c.SetToNow();

	Transaction t(mpObserver);
	mpObserver->Update(b, aIndex);
	mpObserver->Update(c, 2);
	*/
	return CS_SUCCESS;
}

}
}
