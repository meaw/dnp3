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
unsigned int Qcounter;
uint64_t Qticker;
uint64_t tid;
void SlaveDemoApp::Timer(int offset) {
	InputQueue = new SafeQueue<CommandsQueue>();
	InputQueue->SetSize(10);
	Qticker=t.create(5000, InputQueue->Interval, std::bind(&SlaveDemoApp::QTick, this));  /*Queue runs every 10 ms*/
    tid = t.create(5000, 1000, std::bind(&SlaveDemoApp::Tick, this));
	counter = offset;
	Qcounter = 0;
}

void SlaveDemoApp::QTick() {  /*Handles incoming queue items*/
	Qcounter++;
	if (InputQueue->Size() > 0) {
		CommandsQueue command=InputQueue->Dequeue();
		if (command.type == 1) { //Setpoint
			command.type = 2;
			//do something
		}
	}

	if (Qcounter % InputQueue->Period == 0) {
		cout << name << " Qcounter " << Qcounter / 100 << " R:" << InputQueue->Received << " L:" << InputQueue->Lost << endl;
		InputQueue->Reset();
	}
}


void SlaveDemoApp::Tick() {
	counter++;
	//std::cout << "Update "<< counter <<" "<<name<< std::endl;
	Binary b((counter & 0x0001), BQ_ONLINE);
	b.SetToNow();
	Counter c(counter, CQ_ONLINE);
	c.SetToNow();
	Analog a(counter*1.0, CQ_ONLINE);
	a.SetToNow();
	// We would like all updates to be sent at one time.When the Transaction object
	// goes out of scope, all updates will be sent in one block to do the slave database.

	//mpQueue1
	SlaveEventBuffer* mBuffer;
		mBuffer = mpQueue1->GetBufferS();
	std::cout << "Out Q1: Bin="<<mBuffer->Size(BT_BINARY) << " An="<<mBuffer->Size(BT_ANALOG) << " Ct=" << mBuffer->Size(BT_COUNTER)<< " " << name << std::endl;
mBuffer = mpQueue2->GetBufferS();
	std::cout << "Out Q2: Bin=" << mBuffer->Size(BT_BINARY) << " An=" << mBuffer->Size(BT_ANALOG) << " Ct=" << mBuffer->Size(BT_COUNTER) << " " << name << std::endl;

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
	mpObserver1(NULL), mpObserver2(NULL){//, InputQueue(){
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

void SlaveDemoApp::SetOutputQueue(ResponseContext* apQueue, int instance)  //DJSC TEST DJSC
{
	if (instance == 1) {
		mpQueue1 = apQueue;
	}
	if (instance == 2) {
		mpQueue2 = apQueue;
	}
}


CommandStatus SlaveDemoApp::HandleControl(Setpoint& aControl, size_t aIndex)
{
	//LOG_BLOCK(LEV_APP, "Received " << aControl.ToString() << " on index: " << aIndex);
	CommandsQueue temp(CommandTypes::CT_SETPOINT, aControl.GetValue(),aIndex);
	int result=InputQueue->Enqueue(temp);
	if (result == 1) {
		return CS_SUCCESS;
	} 
	else {
		return CS_TIMEOUT;
	}	
}

// same as for the setpoint
CommandStatus SlaveDemoApp::HandleControl(BinaryOutput& aControl, size_t aIndex)
{
	
	CommandsQueue temp(CommandTypes::CT_BINARY_OUTPUT, aControl.GetCode(), aIndex);
	int result = InputQueue->Enqueue(temp);
	if (result == 1) {
		return CS_SUCCESS;
	}
	else {
		return CS_TIMEOUT;
	}
	/*
	return CS_SUCCESS;
	*/
}

}
}
