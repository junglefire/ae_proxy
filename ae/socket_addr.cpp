#include "socket_addr.h"

#include <iostream>

using namespace smart;

SocketAddr::SocketAddr(const std::string&& ip, unsigned short port, IPV ipv): 
	ip_(ip), 
	port_(port), 
	hash_code_(0),
	ipv_(ipv) {
	// constructor
	if (ipv == Ipv6) {
		::uv_ip6_addr(ip.c_str(), port, &ipv6_);
	} else {
		::uv_ip4_addr(ip.c_str(), port, &ipv4_);
	}
}

SocketAddr::SocketAddr(const std::string& ip, unsigned short port, IPV ipv):
	SocketAddr(std::move(ip), port, ipv) {
}

SocketAddr::SocketAddr(const sockaddr* addr, IPV ipv):
	ipv_(ipv),
	hash_code_(0) {
	// constructor
	if (ipv_ == Ipv4) {
		ipv4_ = *(reinterpret_cast<const sockaddr_in*>(addr));
	} else {
		ipv6_ = *(reinterpret_cast<const sockaddr_in6*>(addr));
	}
	port_ = get_ip_and_port((const sockaddr_storage *)(addr), ip_, ipv);
}

const sockaddr* SocketAddr::addr() {
	return (ipv_ == Ipv6) ? reinterpret_cast<const sockaddr*>(&ipv6_) : reinterpret_cast<const sockaddr*>(&ipv4_);
}

void SocketAddr::to_string(std::string& str) {
	str = ip_ + ":" + std::to_string(port_);
}

int SocketAddr::port() {
	return this->port_;
}

std::string SocketAddr::ip() {
	return this->ip_;
}

std::string SocketAddr::to_string() {
	std::string str = ip_ + ":" + std::to_string(port_);
	return str;
}

SocketAddr::IPV SocketAddr::ipv() {
	return ipv_;
}

std::size_t SocketAddr::hash_code() {
	if (this->hash_code_ == 0) {
		std::hash<std::string> hash_fn;
		std::string str = ip_ + ":" + std::to_string(port_);
		this->hash_code_ = hash_fn(str);
	}
	return this->hash_code_;
}

void SocketAddr::addr_to_str(uv_tcp_t* client, std::string& addrStr, IPV ipv) {
	struct sockaddr_storage addr;
	int len = sizeof(struct sockaddr_storage);
	::uv_tcp_getpeername(client, (struct sockaddr *)&addr, &len);
	uint16_t port = get_ip_and_port(&addr, addrStr, ipv);
	addrStr += ":" + std::to_string(port);
}

uint16_t SocketAddr::get_ip_and_port(const sockaddr_storage* addr, std::string& out, IPV ipv) {
	auto inet = (Ipv6 == ipv) ? AF_INET6 : AF_INET;
	if (Ipv6 == ipv) {
        char ip[64];
        struct sockaddr_in6* addr6 = (struct sockaddr_in6 *)addr;
        //低版本windows可能找不到inet_ntop函数。
#if defined WIN32_MSVC
        DWORD size = sizeof(ip);
        WSAAddressToString((LPSOCKADDR)addr6, sizeof(sockaddr_in6), NULL, ip, &size);
        out = std::string(ip);
        auto index = out.rfind(":");
        if (index >= 0)
        {
            out.resize(index);
        }
        return (htons(addr6->sin6_port));
#else
        std::string str(inet_ntop(inet, (void *)&(addr6->sin6_addr), ip, 64));
        out.swap(str);
        return(htons(addr6->sin6_port));
#endif
	} else {
		struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
		std::string str(inet_ntoa(addr4->sin_addr));
		out.swap(str);
		return htons(addr4->sin_port);
	}
}


