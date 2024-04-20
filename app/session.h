#ifndef SMART_SESSION_H__
#define SMART_SESSION_H__

#include <logger.h>
#include <ae.h>

NAMESPACE_BEGIN(smart)

typedef struct session_key_s_ {
	std::size_t client;
	std::size_t server;
} session_key_s;

template <typename V>
class Session
{
public:
	Session();
	~Session();
	Session(const Session&) = delete;
	Session& operator=(const Session&) = delete;
public:
	int insert(const char* cip, int cport, const char* sip, int sport, std::unique_ptr<V> value);
private:
	std::map<session_key_s, std::unique_ptr<V>> map_;
};

NAMESPACE_END(smart)
#endif
