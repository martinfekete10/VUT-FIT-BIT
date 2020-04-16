/**
 * IPK project 2 - packet sniffer
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
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>


// Ethernet header len is constant
#define SIZE_ETHERNET 14


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


/**
 * Function prototypes
 */
void print_err(char *);
void check_int(char *);
void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void print_first_line(const u_char *, int, bool);
void print_tcp_packet(const u_char *, int);
void print_udp_packet(const u_char *, int);
void print_ascii(const u_char *, int, int);
int print_data(const u_char *, int, int);
int print_interfaces();


/** 
 * Parses arguments and sets the interface for sniffing
 */
int main(int argc, char **argv) {

    // ------------------- Argument parse -------------------

    // Arg variables
    char interface[128] = "";
    char port[16] = "";
    bool tcp = false;
    bool udp = false;
    int num = 1;

    // Parse arguments
    int c;
    while (1) {
        static struct option longopts[] = {
            { "tcp",   no_argument,            NULL,          't' },
            { "udp",   no_argument,            NULL,          'u' },
            { NULL,    0,                      NULL,           0 }
        };
        
        int option_index = 0;

        c = getopt_long (argc, argv, "i:p:tun:", longopts, &option_index);
        
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
            default:
                print_err("unknown switch used");
        }
    }

    // Interface was not specified, write it to stdout
    if (interface[0] == '\0') {
        printf("X--------------------------------------------------------------------------------X\n");
        printf("| Usage: ./ipk-sniffer -i interface [-p­­port] [--tcp|-t] [--udp|-u] [-n num]    |\n");
        printf("|--------------------------------------------------------------------------------|\n");
        printf("| [mandatory] -i interface : defines sniffed interface                           |\n");
        printf("| [optional]  -p port      : defines sniffed port                                |\n");
        printf("| [optional]  -t or --tcp  : only TCP packet are sniffed                         |\n");
        printf("| [optional]  -u or --udp  : only UDP packet are sniffed                         |\n");
        printf("| [optional]  -n num       : defines number of packets that will be sniffed      |\n");
        printf("X--------------------------------------------------------------------------------X\n\n");
        printf("Available interfaces:\n");
        return print_interfaces();
    }

    // Define filter
    char filter[32] = "";

    if (tcp && !udp) {
        strcpy(filter, "tcp");
    } else if (udp && !tcp) {
        strcpy(filter, "udp");
    } else {
        strcpy(filter, "tcp or udp");
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

    // ------------------- Prepare interfaces -------------------

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *interfaces;
    
    if (pcap_findalldevs(&interfaces, errbuf) == -1) {
        print_err(errbuf);
    }

    // ---------------------- Open device -----------------------
    
    pcap_t *handle;
    handle = pcap_open_live(interface, BUFSIZ , 1, 1000, errbuf);

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

    // ------------------------- Sniff --------------------------

    pcap_loop(handle, num, process_packet, NULL);

    // ------------------------ Cleanup -------------------------

    pcap_freecode(&fp);
    pcap_close(handle);

    return(0);
}


/**
 * Callback function
 * Decides packet protocol and calls function to parse and print the packet accordingly
 */
void process_packet(u_char *nothing, const struct pcap_pkthdr *header, const u_char *packet) {
    
    // Gather timestamp from header and print it formatted
    struct tm *ts;
    ts = localtime(&header->ts.tv_sec);
    printf("%02d:%02d:%02d.%04d ", ts->tm_hour, ts->tm_min, ts->tm_sec, header->ts.tv_usec);
    
    int size = header->len;
    struct ip *iphdr = (struct ip *)(packet + SIZE_ETHERNET);
    
    switch(iphdr->ip_p) {
        case IPPROTO_TCP:
            print_tcp_packet(packet, size);
            break;
        case IPPROTO_UDP:
            print_udp_packet(packet, size);
            break;
        default:
            // This should never be printed
            printf("Err: %hhu should not be printed\n", iphdr->ip_p);
            break;
    } 
}


/**
 * Prints first line of each packet in format:
 * time IP|FQDN : port > IP|FQDN : port
 */
void print_first_line(const u_char *packet, int size, bool packet_t) {

    struct ip *iph = (struct ip *)(packet + SIZE_ETHERNET);
    int iphdrlen = iph->ip_hl * 4;

    // Get FQDN of source and destination adresses
    struct sockaddr_in source, dest;
    socklen_t len_s, len_d;
    char hbuf_s[NI_MAXHOST], hbuf_d[NI_MAXHOST];
    
    memset(&source, 0, sizeof(source));
    memset(&dest, 0, sizeof(dest));

    source.sin_family = AF_INET;
    source.sin_addr.s_addr = iph->ip_src.s_addr;
    len_s = sizeof(struct sockaddr_in);

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = iph->ip_dst.s_addr;
    len_d = sizeof(struct sockaddr_in);

    // Only FQDN should be printed, thus the flag NI_NOFQDN
    // If FQDN is not found, IP adress is printed
    getnameinfo((struct sockaddr *) &source, len_s, hbuf_s, sizeof(hbuf_s), NULL, 0, NI_NOFQDN);
    getnameinfo((struct sockaddr *) &dest, len_d, hbuf_d, sizeof(hbuf_s), NULL, 0, NI_NOFQDN);


    int s_port, d_port;

    // TCP packet
    if (packet_t) {
        // Needed to get to the source and destination port
        struct tcphdr *tcph = (struct tcphdr*)(packet + iphdrlen + SIZE_ETHERNET);
        s_port = ntohs(tcph_s);
        d_port = ntohs(tcph_d);
    // UDP packet
    } else {
        // Needed to get to the source and destination port
        struct udphdr *udph = (struct udphdr*)(packet + iphdrlen  + SIZE_ETHERNET);
        s_port = ntohs(udph_s);
        d_port = ntohs(udph_d);
    }

    // ------------- Print first line -------------
    printf("%s : ", hbuf_s);
    printf("%d > ", s_port);
    printf("%s : ", hbuf_d);
    printf("%d\n", d_port);
    printf("\n");
}


/**
 * Calls function to print basic info about packet
 * Then calls function to print TCP packet in required format
 */
void print_tcp_packet(const u_char *packet, int size) {

    // ------------ Print first line --------------
    print_first_line(packet, size, true);

    // -------------- Print data ------------------	
    struct ip *iph = (struct ip *)(packet + SIZE_ETHERNET);
    int iphdrlen = iph->ip_hl * 4;

    struct tcphdr *tcph=(struct tcphdr*)(packet + SIZE_ETHERNET + iphdrlen);
    int header_size =  SIZE_ETHERNET + iphdrlen + tcph_off * 4;

    //int cnt = print_data(packet, header_size, 0);
    //print_data(packet + header_size, size - header_size, cnt);
    print_data(packet, size, 0);
}


/**
 * Calls function to print basic info about packet
 * Then calls function to print UDP packet in required format
 */
void print_udp_packet(const u_char *packet, int size) {

    // ------------ Print first line --------------
    print_first_line(packet, size, false);

    // -------------- Print data ------------------	
    struct ip *iph = (struct ip *)(packet + SIZE_ETHERNET);
    int iphdrlen = iph->ip_hl * 4;

    struct udphdr *udph = (struct udphdr*)(packet + iphdrlen  + SIZE_ETHERNET);
    int header_size =  SIZE_ETHERNET + iphdrlen + sizeof(udph);

    //int cnt = print_data(packet, header_size, 0);
    //print_data(packet + header_size, size - header_size, cnt);
    print_data(packet, size, 0);
}


/**
 * Function prints whole packet in format:
 * no_of_printed_bytes_hex: bytes_hex bytes_ASCII
 * Header of packet and body is separated by newline char
 */
int print_data(const u_char* data , int size, int header_body) {

    int cnt = 0;

    for (int i = 0; i < size ; i++) {
        cnt++;
        // bytes_ASCII
        if (i % 16 == 0 && i != 0) {
            print_ascii(data, i - 1, 15);
        }
        
        // no_of_printed_bytes_hex
        if (i % 16 == 0) {
            printf("0x%04x:", i);
            /* // Last line
            if (i + 16 > size) {
                int leftover_bytes = size - i;
                printf("0x%04x:", i + leftover_bytes + header_body);
            // New line except last
            } else {
                if (header_body) {
                    printf("0x%04x:", i + 16 + header_body);
                } else {
                    printf("0x%04x:", i + header_body);
                }
            } */
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
        }
    }
    
    printf("\n");
    return cnt;
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
        if (data[j] >= 32 && data[j] <= 126) {
            printf("%c", data[j]);
        // Char is not printable - replace with "."
        } else {
            printf(".");
        }
    }
    
    // Printing at newline
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
