#include <cstdlib>
#include <string>
#include <limits>

#include <cxxopts.hpp>
#include <logger.h>
#include <pcap.h>

using namespace cxxopts;

std::string get_default_device();
void got_packet_cb(u_char*, const struct pcap_pkthdr*, const u_char*);

#define SNAP_LEN		1518	/* default snap length (maximum bytes per packet to capture) */
#define SIZE_ETHERNET 	14		/* ethernet headers are always exactly 14 bytes [1] */
#define ETHER_ADDR_LEN	6		/* Ethernet addresses are 6 bytes */

// Ethernet header
struct sniff_ethernet {
	u_char  ether_dhost[ETHER_ADDR_LEN];	/* destination host address */
	u_char  ether_shost[ETHER_ADDR_LEN];	/* source host address */
	u_short ether_type;						/* IP? ARP? RARP? etc */
};

// IP header 
struct sniff_ip {
	u_char  ip_vhl;				/* version << 4 | header length >> 2 */
	u_char  ip_tos;				/* type of service */
	u_short ip_len;				/* total length */
	u_short ip_id;				/* identification */
	u_short ip_off;				/* fragment offset field */
	#define IP_RF 0x8000		/* reserved fragment flag */
	#define IP_DF 0x4000		/* don't fragment flag */
	#define IP_MF 0x2000		/* more fragments flag */
	#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
	u_char  ip_ttl;				/* time to live */
	u_char  ip_p;				/* protocol */
	u_short ip_sum;				/* checksum */
	struct  in_addr ip_src;		/* source address */
	struct 	in_addr ip_dst; 	/* dest address */
};

#define IP_HL(ip)	(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)	(((ip)->ip_vhl) >> 4)

// TCP header
typedef u_int tcp_seq;

struct sniff_tcp {
	u_short th_sport;		/* source port */
	u_short th_dport;		/* destination port */
	tcp_seq th_seq;			/* sequence number */
	tcp_seq th_ack;			/* acknowledgement number */
	u_char  th_offx2;		/* data offset, rsvd */
	#define TH_OFF(th) (((th)->th_offx2 & 0xf0) >> 4)
	u_char  th_flags;
	#define TH_FIN  0x01
	#define TH_SYN  0x02
	#define TH_RST  0x04
	#define TH_PUSH 0x08
	#define TH_ACK  0x10
	#define TH_URG  0x20
	#define TH_ECE  0x40
	#define TH_CWR  0x80
	#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;			/* window */
	u_short th_sum;			/* checksum */
	u_short th_urp;			/* urgent pointer */
};

int main(int argc, char* argv[]) {
	ParseResult args;
	Options options("test_pcap", "libpcap test app");

	try {
		options.add_options()
			("d,dev", "Device Name", value<std::string>()->default_value(""))
			("b,bpf", "[*required] Berkeley Packet Filter", value<std::string>())
			("h,help", "Print usage")
		;
		args = options.parse(argc, argv);
	} catch (cxxopts::OptionException &e){
		fprintf(stderr, "parse error: %s", e.what());
		exit(-1);
	}

	if (args.count("help")) {
		_info("%s", options.help().c_str());
		exit(0);
	}

	if (!args.count("bpf")) {
		_info("%s", options.help().c_str());
		exit(0);
	}

	std::string bpf = args["bpf"].as<std::string>();
	std::string dev = args["dev"].as<std::string>();
	if (dev.length() == 0) {
		dev = get_default_device();
	}

	_info("default device: `%s`", dev.c_str());

	pcap_t* handle = nullptr;
	char errbuf[PCAP_ERRBUF_SIZE];
	handle = pcap_open_live(dev.c_str(), BUFSIZ, 1, 1000, errbuf);
	if (handle == nullptr) {
		_error("couldn't open device `%s`, err: %s", dev.c_str(), errbuf);
		exit(-1);
	}

	// make sure we're capturing on an Ethernet device
	if (pcap_datalink(handle) != DLT_EN10MB) {
		_error("device `%s` doesn't provide Ethernet headers - not supported, abort!", dev.c_str());
		exit(-1);
	}

	bpf_u_int32 mask;	/* The netmask of our sniffing device */
	bpf_u_int32 net;	/* The IP of our sniffing device */

	if (pcap_lookupnet(dev.c_str(), &net, &mask, errbuf) == -1) {
		_error("can't get netmask for device %s", dev.c_str());
		net = 0;
		mask = 0;
	}

	struct bpf_program fp;			/* The compiled filter expression */
	char filter_exp[] = "tcp port 12122";	/* The filter expression */

	_info("comple BFP filter: %s", bpf.c_str());
	if (pcap_compile(handle, &fp, bpf.c_str(), 0, net) == -1) {
		_error("couldn't parse filter `%s`, err: %s", bpf.c_str(), pcap_geterr(handle));
		exit(-1);
	}

	if (pcap_setfilter(handle, &fp) == -1) {
		_error("couldn't install filter `%s`, err: %s", bpf.c_str(), pcap_geterr(handle));
		exit(-1);
	}

	// now we can set our callback function
	pcap_loop(handle, -1, got_packet_cb, NULL);

	// cleanup 
	pcap_freecode(&fp);
	pcap_close(handle);
	return 0;
}

// dissect/print packet callback function
void got_packet_cb(u_char* args, const struct pcap_pkthdr* header, const u_char* packet) {
	static int count = 1;                   /* packet counter */

	// declare pointers to packet headers
	const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
	const struct sniff_ip *ip;              /* The IP header */
	const struct sniff_tcp *tcp;            /* The TCP header */
	u_char *payload;                    	/* Packet payload */

	int size_ip;
	int size_tcp;
	int size_payload;

	// define ethernet header
	ethernet = (struct sniff_ethernet*)(packet);

	// define/compute ip header offset 
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		_error("invalid IP header length: `%u` bytes, abort!", size_ip);
		return;
	}

	count++;

	// print source and destination IP addresses
	// [Warn] don't print src ip and dst ip in one line, inet_ntoa() return static pointer to char array
	// _info("IP packet: `%s` -> `%s`", inet_ntoa(ip->ip_src), inet_ntoa(ip->ip_dst));
	std::string sip = inet_ntoa(ip->ip_src);
	std::string dip = inet_ntoa(ip->ip_dst);
	_info("IP packet[#%d]: `%s` -> `%s`", count, sip.c_str(), dip.c_str());

	// determine protocol 
	switch(ip->ip_p) {
		case IPPROTO_TCP:
			_info("Protocol: TCP");
			break;
		case IPPROTO_UDP:
			_info("Protocol: UDP");
			return;
		case IPPROTO_ICMP:
			_info("Protocol: ICMP");
			return;
		case IPPROTO_IP:
			_info("Protocol: IP");
			return;
		default:
			_info("Protocol: unknown");
			return;
	}

	// OK, parse the tcp packet
	tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
	size_tcp = TH_OFF(tcp)*4;
	if (size_tcp < 20) {
		_error("invalid TCP header length: `%u` bytes", size_tcp);
		return;
	}

	int sport = ntohs(tcp->th_sport);
	int dport = ntohs(tcp->th_sport);

	_info("TCP packet: `%s:%d` -> `%s:%d`", sip.c_str(), sport, dip.c_str(), dport);

	// define/compute tcp payload (segment) offset 
	payload = (u_char*)(packet + SIZE_ETHERNET + size_ip + size_tcp);

	// compute tcp payload (segment) size 
	size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);
	_info("payload size: %d", size_payload);
	return;
}

std::string get_default_device() {
	char* dev; 
	char errbuf[PCAP_ERRBUF_SIZE];
	dev = pcap_lookupdev(errbuf);
	if (dev == nullptr) {
		throw std::runtime_error("get default device failed, err: "+std::string(errbuf));
	}
	return std::string(dev);
}