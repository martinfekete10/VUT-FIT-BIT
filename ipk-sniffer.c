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
#include <regex.h>

#include <netdb.h>
#include <pcap/pcap.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>


/**
 * Function prototypes
 */
void print_err(char *);
void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);
void print_first_line(const u_char *, int, bool);
void print_tcp_packet(const u_char *, int);
void print_udp_packet(const u_char *, int);
int print_data(const u_char *, int, int);
void print_ascii(const u_char *, int, int);
void print_time();
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
                strcpy(port, optarg);
                break;
            case 't':
                tcp = true;
                break;
            case 'u':
                udp = true;
                break;
            case 'n':
                num = atoi(optarg);
                break;
            default:
                return(1);
        }
    }

    // Interface was not specified, write it to stdout
    if (interface[0] == '\0') {
        printf("Usage: ./ipk-sniffer -i interface [-p ­­port] [--tcp|-t] [--udp|-u] [-n num]\n\n");
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

    printf("%s\n", filter);

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

    return(0);
}


/**
 * Decides packet protocol and calls function to parse and print the packet accordingly
 */
void process_packet(u_char *nothing, const struct pcap_pkthdr *header, const u_char *packet) {
    int size = header->len;
    struct iphdr *iphdr = (struct iphdr*)(packet + sizeof(struct ethhdr));
    
    switch(iphdr->protocol) {
        case IPPROTO_TCP:
            print_tcp_packet(packet, size);
            break;
        case IPPROTO_UDP:
            print_udp_packet(packet, size);
            break;
        default:
            // This should never be printed
            printf("Err: %hhu should not be printed\n", iphdr->protocol);
            break;
    } 
}


/**
 * Prints first line of each packet in format:
 * time IP|FQDN : port > IP|FQDN : port
 */
void print_first_line(const u_char *packet, int size, bool packet_t) {

    struct iphdr *iph = (struct iphdr *)(packet  + sizeof(struct ethhdr));
    int iphdrlen = iph->ihl * 4;

    // Get FQDN of source and destination adresses
    struct sockaddr_in source, dest;
    socklen_t len_s, len_d;
    char hbuf_s[NI_MAXHOST], hbuf_d[NI_MAXHOST];
    
    memset(&source, 0, sizeof(source));
    memset(&dest, 0, sizeof(dest));

    source.sin_family = AF_INET;
    source.sin_addr.s_addr = iph->saddr;
    len_s = sizeof(struct sockaddr_in);

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = iph->daddr;
    len_d = sizeof(struct sockaddr_in);

    // Only FQDN should be printed, thus flag NI_NOFQDN
    // If FQDN is not found, IP adress is printed
    getnameinfo((struct sockaddr *) &source, len_s, hbuf_s, sizeof(hbuf_s), NULL, 0, NI_NOFQDN);
    getnameinfo((struct sockaddr *) &dest, len_d, hbuf_d, sizeof(hbuf_s), NULL, 0, NI_NOFQDN);


    int s_port, d_port;

    // TCP packet
    if (packet_t) {
        // Needed to get to the source and destination port
        struct tcphdr *tcph = (struct tcphdr*)(packet + iphdrlen + sizeof(struct ethhdr));
        s_port = ntohs(tcph->source);
        d_port = ntohs(tcph->dest);
    // UDP packet
    } else {
        // Needed to get to the source and destination port
        struct udphdr *udph = (struct udphdr*)(packet + iphdrlen  + sizeof(struct ethhdr));
        s_port = ntohs(udph->source);
        d_port = ntohs(udph->dest);
    }

    // ------------- Print first line -------------
    print_time();
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
    struct iphdr *iph = (struct iphdr *)(packet + sizeof(struct ethhdr));
    int iphdrlen = iph->ihl * 4;

    struct tcphdr *tcph=(struct tcphdr*)(packet + sizeof(struct ethhdr) + iphdrlen);
    int header_size =  sizeof(struct ethhdr) + iphdrlen + tcph->doff * 4;

    int cnt = print_data(packet, header_size, 0);
    print_data(packet + header_size, size - header_size, cnt);
}


/**
 * Calls function to print basic info about packet
 * Then calls function to print UDP packet in required format
 */
void print_udp_packet(const u_char *packet, int size) {

    // ------------ Print first line --------------
    print_first_line(packet, size, true);

    // -------------- Print data ------------------	
    struct iphdr *iph = (struct iphdr *)(packet + sizeof(struct ethhdr));
    int iphdrlen = iph->ihl * 4;

    struct udphdr *udph = (struct udphdr*)(packet + iphdrlen  + sizeof(struct ethhdr));
    int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof(udph);

    int cnt = print_data(packet, header_size, 0);
    print_data(packet + header_size, size - header_size, cnt);
}


/** 
 * Function prints time in format: HH:MM:SS.MSMSMSMS
 * Called when printing the first line of packet
 */
void print_time() {
    // For HH:MM:SS
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // For .MSMSMSMS
    struct timeval ms;
    gettimeofday(&ms, NULL);
    
    printf("%02d:%02d:%02d.%04ld ", tm.tm_hour, tm.tm_min, tm.tm_sec, ms.tv_usec);
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
            print_ascii(data, i, 16);
        }
        
        // no_of_printed_bytes_hex
        if (i % 16 == 0) {
            // Last line
            if (i + 16 > size) {
                int leftover_bytes = size - i;
                printf("0x%04x:", i + leftover_bytes);
            // New line except last
            } else {        
                printf("0x%04x:", i + header_body);
            }
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
 * Prints error message to stderr
 * Ends program with return code 1
 */
void print_err(char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}
