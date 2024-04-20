#include <cstdlib>
#include <string>
#include <limits>

#include <cxxopts.hpp>
#include <logger.h>
#include <pcap.h>

using namespace cxxopts;

std::string get_default_device();
void got_packet_cb(u_char*, const struct pcap_pkthdr*, const u_char*);

// ARP Header, (assuming Ethernet+IPv4)
#define ARP_REQUEST 1 	/* ARP Request             */ 
#define ARP_REPLY 	2 	/* ARP Reply               */ 

typedef struct arphdr { 
    u_int16_t htype;    /* Hardware Type           */ 
    u_int16_t ptype;    /* Protocol Type           */ 
    u_char hlen;        /* Hardware Address Length */ 
    u_char plen;        /* Protocol Address Length */ 
    u_int16_t oper;     /* Operation Code          */ 
    u_char sha[6];      /* Sender hardware address */ 
    u_char spa[4];      /* Sender IP address       */ 
    u_char tha[6];      /* Target hardware address */ 
    u_char tpa[4];      /* Target IP address       */ 
}arphdr_t; 

#define MAXBYTES2CAPTURE 2048 

int main(int argc, char* argv[]) {
	ParseResult args;
	Options options("test_pcap", "libpcap test app");

	try {
		options.add_options()
			("d,dev", "Device Name", value<std::string>()->default_value(""))
			("h,help", "Print usage")
		;
		args = options.parse(argc, argv);
	} catch (cxxopts::OptionException &e){
		fprintf(stderr, "parse error: %s", e.what());
		exit(-1);
	}

	std::string dev = args["dev"].as<std::string>();
	if (dev.length() == 0) {
		dev = get_default_device();
	}

	_info("default device: `%s`", dev.c_str());

	pcap_t* handle = nullptr;
	char errbuf[PCAP_ERRBUF_SIZE];
	handle = pcap_open_live(dev.c_str(), MAXBYTES2CAPTURE, 1, 512, errbuf);
	if (handle == nullptr) {
		_error("couldn't open device `%s`, err: %s", dev.c_str(), errbuf);
		exit(-1);
	}

	bpf_u_int32 mask;	/* The netmask of our sniffing device */
	bpf_u_int32 net;	/* The IP of our sniffing device */

	if (pcap_lookupnet(dev.c_str(), &net, &mask, errbuf) == -1) {
		_error("can't get netmask for device %s", dev.c_str());
		net = 0;
		mask = 0;
	}

	struct bpf_program fp;		/* The compiled filter expression */
	char filter_exp[] = "arp";	/* The filter expression */

	_info("comple BFP filter: %s", filter_exp);
	if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
		_error("couldn't parse filter `%s`, err: %s", filter_exp, pcap_geterr(handle));
		exit(-1);
	}

	if (pcap_setfilter(handle, &fp) == -1) {
		_error("couldn't install filter `%s`, err: %s", filter_exp, pcap_geterr(handle));
		exit(-1);
	}

	int i = 0;
	struct pcap_pkthdr pkthdr;
	arphdr_t* arpheader = NULL;
	const unsigned char* packet=NULL;

	while(1) {
		if ((packet = pcap_next(handle,&pkthdr)) == nullptr) { 
			_error("get the packet failed, err: %s", errbuf);
			exit(-1);
		}

		arpheader = (struct arphdr *)(packet+14);
		_info("received packet size: `%d` bytes", pkthdr.len); 
		_info("hardware type: %s", (ntohs(arpheader->htype) == 1) ? "Ethernet" : "Unknown"); 
		_info("protocol type: %s", (ntohs(arpheader->ptype) == 0x0800) ? "IPv4" : "Unknown"); 
		_info("pperation: %s", (ntohs(arpheader->oper) == ARP_REQUEST)? "ARP Request" : "ARP Reply"); 

		// If is Ethernet and IPv4, print packet contents
		if (ntohs(arpheader->htype) == 1 && ntohs(arpheader->ptype) == 0x0800) {
			printf("Sender MAC: ");
			for(i=0; i<6;i++)
				printf("%02X:", arpheader->sha[i]); 

			printf("\nSender IP: "); 
			for(i=0; i<4;i++)
				printf("%d.", arpheader->spa[i]); 

			printf("\nTarget MAC: "); 
			for(i=0; i<6;i++)
				printf("%02X:", arpheader->tha[i]); 

			printf("\nTarget IP: "); 
			for(i=0; i<4; i++)
				printf("%d.", arpheader->tpa[i]); 
			
			printf("\n"); 
		} 

	} 

	// cleanup 
	pcap_freecode(&fp);
	pcap_close(handle);
	return 0;
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