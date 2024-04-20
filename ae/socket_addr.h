#ifndef SMART_SOCKET_ADDR_HPP
#define SMART_SOCKET_ADDR_HPP

#include "ae.h"

NAMESPACE_BEGIN(smart)

class SocketAddr {
public:
	enum IPV {
		Ipv4 = 0,
		Ipv6
	};
public:
	SocketAddr(const std::string&& ip, unsigned short port, IPV ipv = Ipv4);
	SocketAddr(const std::string& ip, unsigned short port, IPV ipv = Ipv4);
	SocketAddr(const sockaddr* addr, IPV ipv = Ipv4);
public:
	const sockaddr* addr();
	void to_string(std::string& str);
	std::string to_string();
	std::size_t hash_code();
	int port();
	std::string ip();
	IPV ipv();
public:
	static void addr_to_str(uv_tcp_t* client, std::string& addrStr, IPV ipv = Ipv4);
	static uint16_t get_ip_and_port(const sockaddr_storage* addr, std::string& out, IPV ipv = Ipv4);
private:
	std::string ip_;
	unsigned short port_;
	const IPV ipv_;
	sockaddr_in ipv4_;
	sockaddr_in6 ipv6_;
	std::size_t hash_code_;
};

NAMESPACE_END(smart)
#endif
