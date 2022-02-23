// AE Class Diagram

class TcpClient {
public:
	TcpClient(EventLoop* loop, bool tcpNoDelay = true);
	virtual ~TcpClient();
public:
	bool is_tcp_no_delay();
	void set_tcp_no_delay(bool isNoDelay);
	void connect(SocketAddr& addr);
	void close(TcpClientCloseCallback callback);
	void set_connect_status_callback(ConnectStatusCallback callback);
	void set_message_callback(NewMessageCallback callback);
public:
	int write(const char* buf, unsigned int size, AfterWriteCallback callback = nullptr);
	void write_in_loop(const char* buf, unsigned int size, AfterWriteCallback callback);
protected:
	void on_message(TcpConnection* connection, const char* buf, ssize_t size);
	void on_connect_close(std::string& name);
	void on_connect(bool success);
	void after_connect_fail();
private:
	void run_connect_callback(ConnectStatus success);
	void on_close(std::string& name);
	void create_tcp_socket();
private:
	TcpConnection* connection_;
	uv_connect_t* connect_;
	SocketAddr::IPV ipv;
	UVTcpPtr socket_;
	bool tcpNoDelay_;
	EventLoop* loop_;
private:
	ConnectStatusCallback connectCallback_;
	NewMessageCallback onMessageCallback_;
};

class TcpAcceptor {
public:
	TcpAcceptor(EventLoop* loop, bool tcpNoDelay);
	virtual ~TcpAcceptor();
	int bind(SocketAddr& addr);
	int listen();
	bool is_listen();
	void close(DefaultCallback callback);
	bool is_tcp_no_delay();
	void set_new_connection_callback(NewConnectionCallback callback);
	EventLoop* loop();
private:
	bool listened_;
	bool tcpNoDelay_;
	EventLoop* loop_;
	NewConnectionCallback callback_;
	DefaultCallback onCloseCompletCallback_;
	uv_tcp_t server_;
	void on_new_connect(UVTcpPtr client);
	void on_close_complete();
};

class TcpConnection {
public :
	TcpConnection(EventLoop* loop, std::string& name, UVTcpPtr client, bool isConnected = true);
	virtual ~TcpConnection();
public:
	void on_socket_close();
	void close(std::function<void(std::string&)> callback);
	int write(const char* buf, ssize_t size, AfterWriteCallback callback);
	void write_in_loop(const char* buf, ssize_t size, AfterWriteCallback callback);
	void set_message_callback(OnMessageCallback callback);
	void set_connect_close_callback(OnCloseCallback callback);
	void set_connect_status(bool status);
	bool is_connected();
	const std::string& name();
private:
	void on_message(const char* buf, ssize_t size);
	void close_complete();
	char* resize_data(size_t size);
	static void on_mesage_receive(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
private:
	std::string name_;
	bool connected_;
	EventLoop* loop_;
	UVTcpPtr handle_;
	std::string data_;
	PacketBufferPtr buffer_;
private:
	OnMessageCallback onMessageCallback_;
	OnCloseCallback onConnectCloseCallback_;
	CloseCompleteCallback closeCompleteCallback_;
};

class TcpServer {
public:
	TcpServer(EventLoop* loop, bool tcpNoDelay = true);
	~TcpServer();
public:
	int listen(SocketAddr& addr);
	void close(DefaultCallback callback);
	TcpConnection* get_connection(const std::string& name);
	void close_connection(const std::string& name);
public:
	void set_new_connect_callback(OnConnectionStatusCallback callback);
	void set_connect_close_callback(OnConnectionStatusCallback callback);
	void set_message_callback(OnMessageCallback callback);
public:
	void write(TcpConnection* connection,const char* buf,unsigned int size, AfterWriteCallback callback = nullptr);
	void write(std::string& name,const char* buf,unsigned int size, AfterWriteCallback callback =nullptr);
	void write_in_loop(TcpConnection* connection,const char* buf,unsigned int size,AfterWriteCallback callback);
	void write_in_loop(std::string& name,const char* buf,unsigned int size,AfterWriteCallback callback);
public:
	void set_timeout(unsigned int);
private:
	void on_accept(EventLoop* loop, UVTcpPtr client);
	void add_connection(std::string& name, TcpConnection* connection);
	void remove_connection(std::string& name);
	void on_message(TcpConnection* connection, const char* buf, ssize_t size);
protected:
	EventLoop* loop_;
private:
	bool tcpNoDelay_;
	SocketAddr::IPV ipv_;
	TcpAcceptor* accetper_;
	std::map<std::string, TcpConnection*> connnections_;
	OnMessageCallback onMessageCallback_;
	OnConnectionStatusCallback onNewConnectCallback_;
	OnConnectionStatusCallback onConnectCloseCallback_;
	TimerWheel<TcpConnection*> timerWheel_;
};

class EventLoop {
public:
	EventLoop();
	~EventLoop();
	static EventLoop* default_loop();
	static const char* get_error_message(int status);
public:
	int run();
	int run_no_wait();
	int stop();
	bool is_stopped();
	Status get_status();
	bool is_run_in_loop_thread();
	void run_in_this_loop(const DefaultCallback func);
	uv_loop_t* handle();
private:
	EventLoop(Mode mode);
	std::thread::id loopThreadId_;
	uv_loop_t* loop_;
	Async* async_;
	std::atomic<Status> status_;
};

class Packet {
public:
	Packet();
	~Packet();
public:
	void pack(const char* data, uint16_t size);
	const char* get_data();
	const uint16_t data_size();
	const std::string& buffer();
	const uint32_t packet_size();
	void swap(std::string& str);
public:	
	static int read_from_buffer(PacketBuffer*, Packet&);
	static uint32_t packet_min_size();
	static void unpack_num(const uint8_t* data, NumType& num);
	static void pack_num(char* data, NumType num);
protected:
	std::string buffer_;
	uint16_t dataSize_;
};

class PacketBuffer {
public:
	PacketBuffer() {}
	virtual ~PacketBuffer(){}
public:
	virtual int append(const char* data, uint64_t size) = 0;
	virtual int read_buffer_n(std::string& data, uint64_t N) = 0;
	virtual int clear_buffer_n(uint64_t N) = 0;
	virtual int clear() = 0;
	virtual uint64_t read_size() = 0;
	int read_string(std::string& out);
	int read_packet(Packet& out);
	int read_generic(void* out);
};

class ListBuffer: public PacketBuffer
{
public:
	ListBuffer();
	~ListBuffer();
	int append(const char* data, uint64_t size) override;
	int read_buffer_n(std::string& data, uint64_t N) override;
	uint64_t read_size() override;
	int clear_buffer_n(uint64_t N) override;
	int clear() override;
private:
	std::list<uint8_t> buffer_;
};

class SocketAddr {
public:
	SocketAddr(const std::string& ip, unsigned short port, IPV ipv);
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

class Timer {
public:
	Timer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback);
	virtual ~Timer();
	void start();
	void close(TimerCloseComplete callback);
	void set_timer_repeat(uint64_t ms);
private:
	bool started_;
	uv_timer_t* handle_;
	uint64_t timeout_;
	uint64_t repeat_;
	TimerCallback callback_;
	TimerCloseComplete closeComplete_;
private:
	void on_time_out();
	void close_complete();
private:
	static void process(uv_timer_t* handle);
};

template<typename T>
class TimerWheel {
public:
	TimerWheel(EventLoop* loop);
	TimerWheel(EventLoop* loop, unsigned int timeout);
	void set_timeout(unsigned int seconds);
	int get_timeout();
	void start();
	void insert(std::shared_ptr<T> value);
private:
	unsigned int index_;
	unsigned int timeoutSec_;
	Timer timer_;
	std::vector<std::set<std::shared_ptr<T>>> wheel_;
	void wheel_callback();
};



