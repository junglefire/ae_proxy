#ifndef SMART_ASYNC_EVENT_H__
#define SMART_ASYNC_EVENT_H__

#include <functional>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include <vector>
#include <mutex>
#include <map>

#include <uv.h>

#ifndef NAMESPACE_BEGIN
#define NAMESPACE_BEGIN(x) namespace x {
#endif //NAMESPACE_BEGIN

#ifndef NAMESPACE_END
#define NAMESPACE_END(x) }
#endif //NAMESPACE_END

NAMESPACE_BEGIN(smart)

/**
 * Forward Class Declaration
 */
class ConnectionWrapper;
class TcpConnection;
class PacketBuffer;
class SocketAddr;
class EventLoop;
class TcpClient;
class Packet;
class Async;
class Timer;


/**
 * Common Struct Defines
 */
struct WriteInfo {
	static const int Disconnected = -1;
	int status;
	char* buf;
	unsigned long size;
};

enum ConnectStatus {
	ConnectSuccess,
	ConnectFail,
	ConnectClose
};


/**
 * Smart Pointer Defines
 */
using ConnectionWrapperPtr	= std::shared_ptr<ConnectionWrapper>;
using TcpConnectionPtr		= std::shared_ptr<TcpConnection>;
using TcpClientPtr 			= std::shared_ptr<TcpClient>;
using EventLoopPtr 			= std::shared_ptr<EventLoop>;
using UVTcpPtr 				= std::shared_ptr<uv_tcp_t>;


/**
 * Callback Function Defines
 */
using DefaultCallback			= std::function<void()>;
using OnUdpMessageCallback  	= std::function<void(SocketAddr&, const char*, unsigned)>;
using TimerCallback	  			= std::function<void(Timer*)>;
using TimerCloseComplete		= std::function<void(Timer*)>;
// tcp_acceptor
using NewConnectionCallback 	= std::function<void(EventLoop* ,UVTcpPtr)> ;
// tcp_connection
using AfterWriteCallback		= std::function<void(WriteInfo& )>;
using OnMessageCallback  		= std::function<void(TcpConnectionPtr, const char*, ssize_t)>;
using OnCloseCallback	  		= std::function<void(std::string& )>;
using CloseCompleteCallback 	= std::function<void(std::string&)>;
// tcp_client
using NewMessageCallback		= std::function<void(const char*,ssize_t)>;
using ConnectStatusCallback 	= std::function<void(ConnectStatus)>;
using TcpClientCloseCallback 	= std::function<void(TcpClient*)>;
// tcp_server
using OnConnectionStatusCallback=  std::function<void (std::weak_ptr<TcpConnection> )>;
// packet
using PacketBufferPtr 			= std::shared_ptr<PacketBuffer>;
// global config
using ReadBufferStringFunc 		= std::function<int(PacketBuffer*, std::string&)>;
using ReadBufferPacketFunc 		= std::function<int(PacketBuffer*, Packet&)>;
using ReadBufferVoidFunc 		= std::function<int(PacketBuffer*, void*)>;
// log writer
using WriteLogCallback 			= std::function<void(int,const std::string&)>;


NAMESPACE_END(smart)
#endif