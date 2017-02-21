#ifndef __WRAPPED_TCP_PIPE_H_
#define __WRAPPED_TCP_PIPE_H_

#include <opendnp3/APL/PhysicalLayerAsyncTCPClient.h>
#include <opendnp3/APL/PhysicalLayerAsyncTCPServer.h>

#include "PhysicalLayerWrapper.h"

namespace boost
{
namespace asio
{
class io_service;
}
}

namespace apl
{

class WrappedTcpPipe
{
public:
	WrappedTcpPipe(Logger*, boost::asio::io_service*, boost::uint16_t aPort);

private:
	PhysicalLayerAsyncTCPClient clientTcp;
	PhysicalLayerAsyncTCPServer serverTcp;

public:
	PhysicalLayerWrapper client;
	PhysicalLayerWrapper server;
};

}

#endif
