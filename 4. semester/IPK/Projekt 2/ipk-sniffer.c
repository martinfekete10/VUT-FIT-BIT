/**
 * IPK projekt 2 - [ZETA] Packet sniffer
 * @author: Martin Fekete <xfeket00@fit.vutbr.cz>
 * @date:   15.4.2020
 */


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include <netdb.h>
#include <pcap/pcap.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>


// Constant header lengths
#define SIZE_SLL 16
#define SIZE_ETHERNET 14
#define SIZE_IPV6 40


/**
 * These defines ensure that sniffer is fully functional on wide range unix systems
 * Linux systems have different tcphdr and udphdr member names than other unix systems (i.e. MacOS)
 */
#ifndef __linux

    #define tcph_s tcph->th_sport
    #define tcph_d tcph->th_dport
    #define tcph_off tcph->th_off

    #define udph_s udph->uh_sport
    #define udph_d udph->uh_dport

#else

    #define tcph_s tcph->source
    #define tcph_d tcph->dest
    #define tcph_off tcph->doff

    #define udph_s udph->source
    #define udph_d udph->dest

#endif

#ifdef _IP_VHL
    #define ipversion iphdr->ip_vhl >> 4
#else
    #define ipversion iphdr->ip_v
#endif


/**
 * Function prototypes
 */
void print_err(char *);
void check_int(char *);
void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void print_first_line_ipv4(const u_char *, bool);
void print_first_line_ipv6(const u_char *, bool);
void print_tcp_packet(const u_char *, int);
void print_udp_packet(const u_char *, int);
void print_ascii(const u_char *, int, int);
void print_data(const u_char *, int, int);
int print_interfaces();

// To differentiate between linux cooked-headers and ethernet
int data_link_offset;


/** 
 * Parses arguments and sets the interface for sniffing
 */
int main(int argc, char **argv) {

    // ------------------- Argument parse -------------------

    // Arg variables
    char interface[128] = "";
    char port[16] = "";
    bool help = false;
    bool tcp = false;
    bool udp = false;
    int num = 1;

    // Parse arguments
    int c;
    while (1) {
        static struct option longopts[] = {
            { "help",   no_argument,           NULL,          'h' },
            { "tcp",   no_argument,            NULL,          't' },
            { "udp",   no_argument,            NULL,          'u' },
            { NULL,    0,                      NULL,           0 }
        };
        
        int option_index = 0;

        c = getopt_long (argc, argv, "i:p:tun:h", longopts, &option_index);
        
        if (c == -1) break;

        switch (c) {
            case 'i':
                strcpy(interface, optarg);
                break;
            case 'p':
                check_int(optarg);
                strcpy(port, optarg);
                break;
            case 't':
                tcp = true;
                break;
            case 'u':
                udp = true;
                break;
            case 'n':
                check_int(optarg);
                num = atoi(optarg);
                break;
            case 'h':
                help = true;
                break;
            default:
                print_err("Unknown switch used");
        }
    }

    // Interface was not specified or --help | -h switch was used
    // Print help message to stdout with all available interfaces
    if (interface[0] == '\0' || help) {
        printf("*--------------------------------------------------------------------------------*\n");
        printf("| Usage: ./ipk-sniffer -i interface [-p­­port] [--tcp|-t] [--udp|-u] [-n num]    |\n");
        printf("|--------------------------------------------------------------------------------|\n");
        printf("| [mandatory] -i interface : defines sniffed interface                           |\n");
        printf("| [optional]  -p port      : defines sniffed port                                |\n");
        printf("| [optional]  -t or --tcp  : only TCP packet are sniffed                         |\n");
        printf("| [optional]  -u or --udp  : only UDP packet are sniffed                         |\n");
        printf("| [optional]  -n num       : defines number of packets that will be sniffed      |\n");
        printf("*--------------------------------------------------------------------------------*\n\n");
        printf("Available interfaces:\n");
        return print_interfaces();
    }

    // -------------------- Define filter --------------------
    
    char filter[32] = "";

    if (tcp && !udp) {
        strcpy(filter, "tcp");
    } else if (udp && !tcp) {
        strcpy(filter, "udp");
    } else {
        strcpy(filter, "(tcp or udp)");
    }

    if (port[0] != '\0') {
        if (filter[0] == '\0') {
            strcpy(filter, "port ");
            strcat(filter, port);
        } else {
            strcat(filter, " and port ");
            strcat(filter, port);
        }
    }

    // ---------------------- Open device -----------------------

    char errbuf[PCAP_ERRBUF_SIZE];
    
    pcap_t *handle;
    handle = pcap_open_live(interface, BUFSIZ , 1, 100, errbuf);

    if (!handle) {
        print_err(errbuf);
    }

    // ------------------------ Set mask ------------------------

    bpf_u_int32 net;
    struct bpf_program fp;

    if (pcap_compile(handle, &fp, filter, 0, net) == -1) {
        print_err("Could not parse filter");
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        print_err("Could not install filter");
    }

    // ----------------- Check data link type ------------------

    int data_link = pcap_datalink(handle);

    if (data_link == DLT_EN10MB) {
        data_link_offset = SIZE_ETHERNET;
    } else if (data_link == DLT_LINUX_SLL) {
        data_link_offset = SIZE_SLL;
    }

    // ------------------------- Sniff --------------------------

    pcap_loop(handle, num, process_packet, NULL);

    // ------------------------ Cleanup -------------------------

    pcap_freecode(&fp);
    pcap_close(handle);

    return(0);
}


/**
 * Callback function
 * Decides packet IP version, protocol and calls function to parse and print the packet accordingly
 */
void process_packet(u_char *nothing, const struct pcap_pkthdr *header, const u_char *packet) {
    
    // Get timestamp from header and print it formatted
    struct tm *ts = localtime(( const time_t *) &header->ts.tv_sec );
    printf("%02d:%02d:%02d.%04ld ", ts->tm_hour, ts->tm_min, ts->tm_sec, header->ts.tv_usec);
    
    int size = header->len;

    // Get IP version
    // ipversion is macro that expands to iphdr->ip_vhl >> 4 or iphdr->ip_v based on the OS
    struct ip *iphdr = (struct ip *)(packet + data_link_offset);

    // IPv4
    if (ipversion == 4) {
        switch(iphdr->ip_p) {
            case IPPROTO_TCP:
                print_tcp_packet(packet, size);
                break;
            case IPPROTO_UDP:
                print_udp_packet(packet, size);
                break;
            default:
                break;
        }
    
    // IPv6
    } else {
        struct ip6_hdr *ip6hdr = (struct ip6_hdr *)(iphdr);
        int protocol = ip6hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt;
        int header_size = data_link_offset + SIZE_IPV6;
        
        switch(protocol) {
            case IPPROTO_TCP:
                // Last parameter is true as the parameter means that protocol is TCP
                print_first_line_ipv6(packet + data_link_offset, true);
                // Print contents of the packet
                struct tcphdr *tcph=(struct tcphdr*)(packet + data_link_offset + SIZE_IPV6);
                header_size += tcph_off * 4;
                print_data(packet, header_size, 0);
                print_data(packet + header_size, size - header_size, header_size);
                break;
            case IPPROTO_UDP:
                // Last parameter is false as the parameter means that protocol is UDP
                print_first_line_ipv6(packet + data_link_offset, false); 
                // Print contents of the packet
                header_size += sizeof(struct udphdr);
                print_data(packet, header_size, 0);
                print_data(packet + header_size, size - header_size, header_size);
                break;
            default:
                break;
        }
    }
}



/**
 * Prints first line of each packet in format:
 * time IP|FQDN : port > IP|FQDN : port
 */
void print_first_line_ipv6(const u_char *packet, bool packet_t) {

    struct ip6_hdr *iph = (struct ip6_hdr *)(packet);

    // ---------------- Get IPs ----------------
    
    struct sockaddr_in6 source, dest;
    socklen_t len_s, len_d;
    char hbuf_s[NI_MAXHOST], hbuf_d[NI_MAXHOST];
    
    memset(&source, 0, sizeof(source));
    memset(&dest, 0, sizeof(dest));

    source.sin6_family = AF_INET6;
    source.sin6_addr = iph->ip6_src;

    dest.sin6_family = AF_INET6;
    dest.sin6_addr = iph->ip6_dst;

    char src_addr[INET6_ADDRSTRLEN];
    char dst_addr[INET6_ADDRSTRLEN];
    

    // --------------- Get port ---------------

    int s_port, d_port;

    // TCP packet
    if (packet_t) {
        // Needed to get to the source and destination port
        struct tcphdr *tcph = (struct tcphdr*)(packet + SIZE_IPV6);
        s_port = ntohs(tcph_s);
        d_port = ntohs(tcph_d);
    // UDP packet
    } else {
        // Needed to get to the source and destination port
        struct udphdr *udph = (struct udphdr*)(packet + SIZE_IPV6);
        s_port = ntohs(udph_s);
        d_port = ntohs(udph_d);
    }


    // ------------- Print first line -------------
    printf("%s : ", inet_ntop(AF_INET6, &(source.sin6_addr), src_addr, sizeof(src_addr)));
    printf("%d > ", s_port);
    printf("%s : ", inet_ntop(AF_INET6, &(dest.sin6_addr), dst_addr, sizeof(dst_addr)));
    printf("%d\n", d_port);
    printf("\n");
}


/**
 * Prints first line of each packet in format:
 * time IP|FQDN : port > IP|FQDN : port
 */
void print_first_line_ipv4(const u_char *packet, bool packet_t) {

    struct ip *iph = (struct ip *)(packet + data_link_offset);
    int iphdrlen = iph->ip_hl * 4;

    // ---------------- Get IPs ----------------
    
    struct sockaddr_in source, dest;
    socklen_t len_s, len_d;
    char hbuf_s[NI_MAXHOST], hbuf_d[NI_MAXHOST];
    
    memset(&source, 0, sizeof(source));
    memset(&dest, 0, sizeof(dest));

    source.sin_family = AF_INET;
    source.sin_addr.s_addr = iph->ip_src.s_addr;

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = iph->ip_dst.s_addr;

    // --------------- Get port ---------------

    int s_port, d_port;

    // TCP packet
    if (packet_t) {
        // Needed to get to the source and destination port
        struct tcphdr *tcph = (struct tcphdr*)(packet + iphdrlen + data_link_offset);
        s_port = ntohs(tcph_s);
        d_port = ntohs(tcph_d);
    // UDP packet
    } else {
        // Needed to get to the source and destination port
        struct udphdr *udph = (struct udphdr*)(packet + iphdrlen  + data_link_offset);
        s_port = ntohs(udph_s);
        d_port = ntohs(udph_d);
    }

    // ------------- Print first line -------------
    printf("%s : ", inet_ntoa(source.sin_addr));
    printf("%d > ", s_port);
    printf("%s : ", inet_ntoa(dest.sin_addr));
    printf("%d\n", d_port);
    printf("\n");
}


/**
 * Calls function to print basic info about packet
 * Then calls function to print TCP packet in required format
 */
void print_tcp_packet(const u_char *packet, int size) {

    // ------------ Print first line --------------
    print_first_line_ipv4(packet, true);

    // -------------- Print data ------------------	
    struct ip *iph = (struct ip *)(packet + data_link_offset);
    int iphdrlen = iph->ip_hl * 4;

    struct tcphdr *tcph=(struct tcphdr*)(packet + data_link_offset + iphdrlen);
    int header_size =  data_link_offset + iphdrlen + tcph_off * 4;

    // Print headers and payload of packet
    print_data(packet, header_size, 0);
    print_data(packet + header_size, size - header_size, header_size);
    printf("\n");
}


/**
 * Calls function to print basic info about packet
 * Then calls function to print UDP packet in required format
 */
void print_udp_packet(const u_char *packet, int size) {

    // ------------ Print first line --------------
    print_first_line_ipv4(packet, false);

    // -------------- Print data ------------------	
    struct ip *iph = (struct ip *)(packet + data_link_offset);
    int iphdrlen = iph->ip_hl * 4;

    struct udphdr *udph = (struct udphdr*)(packet + iphdrlen  + data_link_offset);
    int header_size =  data_link_offset + iphdrlen + sizeof(struct udphdr);

    // Print headers and payload of packet
    print_data(packet, header_size, 0);
    print_data(packet + header_size, size - header_size, header_size);
    printf("\n");
}


/**
 * Function prints whole packet in format:
 * no_of_printed_bytes_hex: bytes_hex bytes_ASCII
 * Header of packet and body is separated by newline char
 * Inspired by: https://www.tcpdump.org/pcap.html
 */
void print_data(const u_char* data , int size, int header_body) {

    int adder = header_body % 16;

    for (int i = 0; i < size ; i++) {
        // bytes_ASCII
        if (i % 16 == 0 && i != 0) {
            print_ascii(data, i - 1, 15);
        }
        
        // no_of_printed_bytes_hex
        if (i % 16 == 0) {
            printf("0x%04x:", i + header_body);
        }

        // bytes_hex
        printf(" %02x",(unsigned int)data[i]);

        // End of header/packed body, print ASCII representation of last line
        if (i == size - 1) {

            // Align ASCII print into 1 collumn
            for (int j = 0; j < 15 - i % 16; j++) {
                printf("   ");
            }

            // Print remainding data
            print_ascii(data, i, i % 16);
            
            // Newline after ASCII print
            printf("\n");
        }
    }
}


/**
 * Prints ASCII representation of bytes in packet
 * Called after printing out every 16 bytes from packet,
 * and at the end of the packet 
 */
void print_ascii(const u_char *data, int i, int line_len) {
    
    // bytes_hex and bytes_ASCII are separated with two spaces
    printf("  ");
    
    for (int j = i - line_len; j <= i; j++) {
        // Char is printable - print ASCII representation
        if (isprint(data[j])) {
            printf("%c", data[j]);
        // Char is not printable - replace with "."
        } else {
            printf(".");
        }
    }
    
    // Newline after ASCII print
    printf("\n");
}


/**
 * Prints all available interfaces
 * Returns 0 if success, 1 otherwise
 */
int print_interfaces() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *interfaces, *temp;

    if (pcap_findalldevs(&interfaces, errbuf) == -1) {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        return 1;
    }

    int i = 0;
    for (pcap_if_t *temp = interfaces; temp != NULL; temp = temp->next) {
        printf("%d  :  %s\n", i++, temp->name);
    }
    
    return 0;
}


/**
 * Function check if argument is of a type int
 * Used to check arguments of switches -p and -n
 */
void check_int(char *arg) {
    int arglen = strlen(arg);
    for (int i = 0; i < arglen; i++) {
        if (!isdigit(arg[i])) {
            print_err("argument is not integer");
        }
    }
}


/**
 * Prints error message to stderr
 * Ends program with return code 1
 */
void print_err(char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}
