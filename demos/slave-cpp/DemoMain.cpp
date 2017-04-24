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

#include <signal.h>
#include <string>

#include <opendnp3/APL/Log.h>
#include <opendnp3/APL/Lock.h>
#include <thread>

using namespace std;
using namespace apl;
using namespace apl::dnp;

SigLock gLock;
SlaveDemoBase* gpDemo = NULL;
void SetDemo(SlaveDemoBase* apDemo)
{
	CriticalSection cs(&gLock);
	gpDemo = apDemo;
}

void Terminate(int sig)
{
	CriticalSection cs(&gLock);
	std::cout << "Signal " << sig << ", shutdown... " << std::endl;
	if(gpDemo) gpDemo->Shutdown();
}

/*
 * Command line syntax:
 *
 *    ./demo-slave-cpp [remote-dnp3] [local-dnp3] [local-ip] [local-port]
 *
 * Defaults:
 *
 *    remote-dnp3    100
 *    local-dnp3     1
 *    local-ip       127.0.0.1
 *    local-port     4999
 */


#define NUM_THREADS     5

void call_from_thread(SlaveDemoApp* IED, int IEDN) {
	       std::cout << "Launching " <<IEDN << std::endl;
		   SetDemo(IED);
		   IED->Run();
}

int main(int argc, char* argv[])
{
	// Default values
	unsigned remote_dnp3 = 100;
	unsigned local_dnp3  = 1;
	string   local_ip    = "127.0.0.1";
	unsigned local_port  = 20000;

	// Parse the command line arguments using a "fall-through"
	// switch statement.
	if (argc > 1 && std::strcmp("help", argv[1]) == 0) {
		cout << argv[0] << " [remote-dnp3] [local-dnp3] [local-ip] [local-port]" << endl;
		return -1;
	}

	switch (argc) {
	case 5:
		{
			istringstream iss(argv[4]);
			iss >> local_port;
		}
	case 4:
		local_ip = argv[3];
	case 3:
		{
			istringstream iss(argv[2]);
			iss >> local_dnp3;
		}
	case 2:
		{
			istringstream iss(argv[1]);
			iss >> remote_dnp3;
		}
	}

	// Create a log object for the stack to use and configure it
	// with a subscriber that print alls messages to the stdout
	EventLog log;
	log.AddLogSubscriber(LogToStdio::Inst());

	// Specify a FilterLevel for the stack/physical layer to use.
	// Log statements with a lower priority will not be logged.
	const FilterLevel LOG_LEVEL = LEV_APP;
	vector <SlaveDemoApp*> IEDS;
	vector <AsyncStackManager*> mgr;
	vector <AsyncStackManager*> mgr2;


	vector <SlaveStackConfig*> stackConfig;
	vector <SlaveStackConfig*> stackConfig2;
	vector <DeviceTemplate*> device;
	vector <DeviceTemplate*> device2;
	vector <IDataObserver*>pDataObserver;
	vector <IDataObserver*>pDataObserver2;


	IEDS.reserve(10);

	int i;
	for (i = 0; i < 5; i++) {
		char name[64];
			sprintf(name,"IED[%d]", i);
	IEDS.push_back(new SlaveDemoApp(log.GetLogger(LOG_LEVEL, name),name));
	IEDS[i]->Timer(i*1000);

	mgr.push_back(new AsyncStackManager(log.GetLogger(LOG_LEVEL, "dnp")));
	mgr2.push_back(new AsyncStackManager(log.GetLogger(LOG_LEVEL, "dnp")));


	// Add a TCPServer to the manager with the name "tcpserver".
	// The server will wait 3000 ms in between failed bind calls.
	char tcp_B[64];
	char tcp_A[64];
	sprintf(tcp_A, "tcpserverA%d", i);
	sprintf(tcp_B, "tcpserverB%d", i);
	mgr[i]->AddTCPServer(
		tcp_A,
		PhysLayerSettings(LOG_LEVEL, 3000),
		local_ip,
		local_port+i
	);

	mgr2[i]->AddTCPServer(
		tcp_B,
		PhysLayerSettings(LOG_LEVEL, 3000),
		local_ip,
		local_port + 10000+i
	);
	stackConfig.push_back(new SlaveStackConfig());
	stackConfig2.push_back(new SlaveStackConfig());

	stackConfig[i]->link.LocalAddr = local_dnp3;
	stackConfig[i]->link.RemoteAddr = remote_dnp3;
	stackConfig[i]->slave.mDisableUnsol = true;
	stackConfig[i]->slave.mEventMaxConfig.mMaxAnalogEvents = 5;
	stackConfig[i]->slave.mEventMaxConfig.mMaxBinaryEvents = 5;
	stackConfig[i]->slave.mEventMaxConfig.mMaxCounterEvents = 5;

	
	 device.push_back(new DeviceTemplate(2, 2, 2, 2, 2, 2, 2));
	stackConfig[i]->device = *device[i];


	stackConfig2[i]->link.LocalAddr = local_dnp3 + 10000;
	stackConfig2[i]->link.RemoteAddr = remote_dnp3 + 10000;
	stackConfig2[i]->slave.mDisableUnsol = true;
	stackConfig2[i]->slave.mEventMaxConfig.mMaxAnalogEvents = 5;
	stackConfig2[i]->slave.mEventMaxConfig.mMaxBinaryEvents = 5;
	stackConfig2[i]->slave.mEventMaxConfig.mMaxCounterEvents = 5;

	device2.push_back(new DeviceTemplate(2, 2, 2, 2, 2, 2, 2));
	stackConfig2[i]->device = *device2[i];
	
	
	
	char slave[64];
	sprintf(slave, "slave%d", i);

	pDataObserver2.push_back(mgr2[i]->AddSlave(tcp_B, slave, LOG_LEVEL, IEDS[i]->GetCmdAcceptor(), *stackConfig2[i]));
	pDataObserver.push_back(mgr[i]->AddSlave(tcp_A, slave, LOG_LEVEL, IEDS[i]->GetCmdAcceptor(), *stackConfig[i])) ;


//	IDataObserver* pDataObserver2 = mgr2[i]->AddSlave("tcpserver", "slave", LOG_LEVEL, IEDS[i]->GetCmdAcceptor(), *stackConfig2[i]);
//	IDataObserver* pDataObserver = mgr[i]->AddSlave("tcpserver", "slave", LOG_LEVEL, IEDS[i]->GetCmdAcceptor(), *stackConfig[i]);

	// Tell the app where to write updates
	IEDS[i]->SetDataObserver(pDataObserver2[i], 2);
	IEDS[i]->SetDataObserver(pDataObserver[i], 1);



	

	//IEDS[i]->Run();
	}
	signal(SIGTERM, &Terminate);
	signal(SIGABRT, &Terminate);
	signal(SIGINT, &Terminate);
	vector <std::thread*> th;
	for (i = 0; i < 5; i++) {
		th.push_back(new thread(call_from_thread, IEDS[i], i));
	}
	for (i = 0; i < 5; i++) {
		th[i]->join();
	}
	/*std::thread t1(call_from_thread,IEDS[0],0);
	std::thread t2(call_from_thread, IEDS[1],1);
	std::thread t3(call_from_thread, IEDS[2], 2);
	std::thread t4(call_from_thread, IEDS[3], 3);
	std::thread t5(call_from_thread, IEDS[4], 4);

    t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	*/

/*

	// create our demo application that handles commands and
	// demonstrates how to publish data give it a loffer with a
	// unique name and log level
	SlaveDemoApp app(log.GetLogger(LOG_LEVEL, "IED[0]"), "IED[00]");
	app.Timer();  //Start the timer for catching data
	// This is the main point of interaction with the stack. The
	// AsyncStackManager object instantiates master/slave DNP
	// stacks, as well as their physical layers
	AsyncStackManager mgr(log.GetLogger(LOG_LEVEL, "dnp"));
	AsyncStackManager mgr2(log.GetLogger(LOG_LEVEL, "dnp"));

	// Add a TCPServer to the manager with the name "tcpserver".
	// The server will wait 3000 ms in between failed bind calls.
	mgr.AddTCPServer(
		"tcpserver",
		PhysLayerSettings(LOG_LEVEL, 3000),
		local_ip,
		local_port
	);

	mgr2.AddTCPServer(
		"tcpserver",
		PhysLayerSettings(LOG_LEVEL, 3000),
		local_ip,
		local_port+10000
	);

	// The master config object for a slave. The default are
	// useable, but understanding the options are important.
	SlaveStackConfig stackConfig;

	// Override the default link addressing
	stackConfig.link.LocalAddr  = local_dnp3;
	stackConfig.link.RemoteAddr = remote_dnp3;
	stackConfig.slave.mDisableUnsol = true;
	stackConfig.slave.mEventMaxConfig.mMaxAnalogEvents = 5;
	stackConfig.slave.mEventMaxConfig.mMaxBinaryEvents = 5;
	stackConfig.slave.mEventMaxConfig.mMaxCounterEvents = 5;

	// The DeviceTemplate struct specifies the structure of the
	// slave's database, as well as the index range of controls and
	// setpoints it accepts.
	DeviceTemplate device(2, 2, 2, 2, 2, 2, 2);
	stackConfig.device = device;



	SlaveStackConfig stackConfig2;

	// Override the default link addressing
	stackConfig2.link.LocalAddr = local_dnp3+10000;
	stackConfig2.link.RemoteAddr = remote_dnp3+10000;
	stackConfig2.slave.mDisableUnsol = true;
	stackConfig2.slave.mEventMaxConfig.mMaxAnalogEvents = 5;
	stackConfig2.slave.mEventMaxConfig.mMaxBinaryEvents = 5;
	stackConfig2.slave.mEventMaxConfig.mMaxCounterEvents = 5;

	// The DeviceTemplate struct specifies the structure of the
	// slave's database, as well as the index range of controls and
	// setpoints it accepts.
	DeviceTemplate device2(2, 2, 2, 2, 2, 2, 2);
	stackConfig2.device = device2;

	// Create a new slave on a previously declared port, with a
	// name, log level, command acceptor, and config info This
	// returns a thread-safe interface used for updating the slave's
	// database.
	IDataObserver* pDataObserver2 = mgr2.AddSlave("tcpserver", "slave", LOG_LEVEL, app.GetCmdAcceptor(), stackConfig2);
	IDataObserver* pDataObserver = mgr.AddSlave("tcpserver", "slave", LOG_LEVEL, app.GetCmdAcceptor(), stackConfig);

	// Tell the app where to write updates
	app.SetDataObserver(pDataObserver2,2);
	app.SetDataObserver(pDataObserver,1);
	*/
	// Configure signal handlers so we can exit gracefully
	/*SetDemo(&app);
	signal(SIGTERM, &Terminate);
	signal(SIGABRT, &Terminate);
	signal(SIGINT,  &Terminate);

	app.Run();

	SetDemo(NULL);
	*/
	return 0;
}
