/**
 * @name:   ISA projekt - Filtrujúci DNS resolver 
 * @author: Martin Fekete <xfeket00@stud.fit.vutbr.cz>
 * @date:   29.10.2020
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <cstdlib>
#include <regex>

#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>

#include "dns.hpp"

#define DNSHDR_LEN 12
#define QNAME_PADDING 2
#define BUFSIZE 1024
#define RCODE_FORMAT_ERR 1
#define RCODE_SERV_FAIL 2
#define RCODE_NOT_IMPLEMENTED 4
#define RCODE_REFUSE 5
#define DNS_QUESTION 0
#define DNS_ANSWER 1
#define CR_ASCII 13

// Must be global for closing the socket after sigint
int sockfd;


/**
 * Ctrl-c signal handler
 */
void sigint_handle(int sigint) {
    close(sockfd);
    exit(0);
}


/**
 * Prints help to stdout and ends the program
 */
void print_help() {
    std::cout << "*------------------------------------------------------------------*\n";
    std::cout << "| Usage: ./dns -s server [-p port] -f filter_file                  |\n";
    std::cout << "*------------------------------------------------------------------*\n";
    std::cout << "| [mandatory] -s server      : DNS server (IP or hostname)         |\n";
    std::cout << "| [mandatory] -f filter_file : file containing blacklisted domains |\n";
    std::cout << "| [optional]  -p port        : port                                |\n";
    std::cout << "*------------------------------------------------------------------*\n";
    exit(0);
}


/**
 * Prints error message to stderr and quits program with error code
 */
void error(std::string err_msg, int err_code) {
    std::cerr << "ERROR: " << err_msg << "\n";
    exit(err_code);
}


/**
 * Checks validity of port provided via argument
 * Checks whether port conains digits only and is not greater than 65 535
 */
void check_port(std::string port) {

    // Check if argument is numeric
    for (unsigned int i = 0; i < port.length(); i++) {
        if (!isdigit(port[i])) {
            error("Port number is not valid", 2);
        }
    }

    // Chceck for range
    if (std::stoi(port) > 65535) {
        error("Port number is greater than 65545", 2);
    }
}


/**
 * Checks if file is readable and is not a directory
 */
void check_filter_file(std::string file) {
    // Check if file is readable
    if (access(file.c_str(), R_OK) != 0) {
        error("Provided filter file does not exists or is not readable", 3);
    }

    // Check if file is not a directory
    struct stat statbuf;
    stat(file.c_str(), &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
        error("Provided file is a directory", 3);
    }
}


/**
 * Sets socket timeout value in seconds
 */
void set_sock_timeout(int *socket, int timeout) {
    struct timeval t_val;
    t_val.tv_sec = timeout;
    t_val.tv_usec = 0;
    if (setsockopt(*socket, SOL_SOCKET, SO_RCVTIMEO, &t_val, sizeof(t_val)) < 0) {
        error("socket timeout setting failed", errno);
    }
}


/**
 * Turns off IPV6_V6ONLY flag so IPv4 adresses can be used
 */
void ipv6only_off(int *socket) {
    int off = 0;
    if (setsockopt(*socket, IPPROTO_IPV6, IPV6_V6ONLY, (char*) &off, sizeof(off)) < 0) {
        error("socket ipv6_only_off setting failed", errno);
    }
}


/**
 * Divides input domain by delimiter "." and returns it in vector
 */ 
std::vector<std::string> domain_to_vector(std::string domain) {

    size_t last = 0;
    size_t next = 0;
    std::vector<std::string> exploded;

    while ((next = domain.find(".", last)) != std::string::npos) {
        exploded.push_back(domain.substr(last, next-last));
        last = next + 1;
    }

    exploded.push_back(domain.substr(last));
    return exploded;
}


/**
 * Checks whether domain in the query is blaclisted in the file
 * Binary search is used to optimize speed
 */
bool is_filtered(std::string domain, std::vector<std::string> blacklist) {

    std::vector<std::string> exploded = domain_to_vector(domain);
    std::string searched_domain;

    // Iterate through domain
    for (auto unused : exploded) {
        
        // Firs item, e.g. com in < docs | google | com >
        if (searched_domain == "") {
            searched_domain = exploded.back();
        // Any other item
        } else {
            searched_domain = exploded.back() + "." + searched_domain;
        }
        
        // Delete item from the vector
        exploded.pop_back();

        // Search for it in the list, if it is there, it should be filtered
        if (std::binary_search(blacklist.begin(), blacklist.end(), searched_domain)) {
            return true;
        }
    }

    // Domain is not filtered
    return false;
}


/**
 * Checks if file provided exists and can be opened
 * If not, server ends with error message
 */
std::vector<std::string> make_blacklist(std::string file) {
    
    std::string line;
    std::ifstream myfile (file);
    std::vector<std::string> blacklist;

    // File was not opened successfully
    if (!myfile.is_open()) {
        error("File cannot be opened", 3);
    }

    // File was opened
    while (getline(myfile, line)) {
        
        // When line begins with "#" or line is empty, skip to the next line
        if (line.front() == '#' || line.empty()) {
            continue;
        }

        // Line ends with carriage return
        if (line.back() == CR_ASCII) {
            line.pop_back();
        }

        // All to lower-case, domains are case insensitive
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        blacklist.push_back(line);

    }

    // Sort the vector so binary search can be used
    std::sort(blacklist.begin(), blacklist.end());
    return blacklist;
}


/**
 * Checks if DNS query is of type A
 */
bool is_type_a(char *buffer, int domain_len) {

    // Check for type of query, copy 16 bytes from the raw buffer data.
    // QTYPE is stored after the QNAME part, which starts and ends with
    // 1B labels, therefore QNAME_PADDING (2)
    
    uint16_t qtype;
    memcpy(&qtype, &buffer[DNSHDR_LEN + QNAME_PADDING + domain_len], sizeof(qtype));

    if (ntohs(qtype) != DNS_ANSWER) {
        return false;
    }
    
    return true;
}


/**
 * Resolves server hostname to IPv4 address format
 */
std::string resolve_hostname(std::string server) {

    struct addrinfo hints, *result;
    memset((char *)&hints, 0, sizeof(hints));
    memset((char *)&result, 0, sizeof(result));
    hints.ai_family = AF_INET;
    
    if (getaddrinfo(server.c_str(), NULL, &hints, &result)) {
        freeaddrinfo(result);
        error("getaddrinfo() failed!", 1);
    }

    char host_ip[INET_ADDRSTRLEN];
    if (getnameinfo(result->ai_addr, result->ai_addrlen, host_ip, sizeof(host_ip), NULL, 0, NI_NUMERICHOST)) {
        freeaddrinfo(result);
        error("getnameinfo() failed!", 1);
    }

    freeaddrinfo(result);
    return host_ip;
}


/**
 * Checks if provided server address is IPv4/IPv6 or hostname
 * Returns string containing IPv6 address format
 */
std::string get_dns_ip(std::string server) {

    std::vector<std::string> server_ip;
    char buff[128];

    // Server in argument was IPv4
    if (inet_pton(AF_INET, server.c_str(), buff)) {
        return "::ffff:" + server;
    
    // Server in argument was IPv6
    } else if (inet_pton(AF_INET6, server.c_str(), buff)) {
        return server;
    
    // Server in argument was host name
    } else {
        return "::ffff:" + resolve_hostname(server);
    }
}


/**
 * Gets resolved domain name from the DNS query
 */
std::string get_domain(char *buffer) {
    
    std::string domain;
    unsigned char *qname = (unsigned char *) &buffer[sizeof(struct dnshdr)];
    int prevlabel = 1;
    int label = qname[0];

    while (label != 0) {

        for (int i = 0; i < label; i++) {
            domain += qname[i+prevlabel];
        }

        prevlabel += label + 1;
        label = qname[prevlabel - 1];
        
        if (label != 0) {
            domain += ".";
        }
    }

    std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);
    return domain;
}


int main(int argc, char **argv) {

    signal(SIGINT, sigint_handle);

    // ------------------- Argument parse -------------------

    // Arg variables
    std::string server;
    std::string filter_file;
    std::string port = "53";

    // Vector for saving blacklisted domains
    std::vector<std::string> blacklist;

    int opt;

    // Parse arguments
    // Inspired by:
    // https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
    while ((opt = getopt(argc, argv, "s:f:p:h")) != -1) {
        switch (opt) {
            case 's':
                server = optarg;
                break;
            case 'f':
                filter_file = optarg;
                check_filter_file(filter_file);
                blacklist = make_blacklist(filter_file);
                break;
            case 'p':
                check_port(optarg);
                port = optarg;
                break;
            case 'h':
                print_help();
                break;
            default:
                error("Unknown argument", 1);
        }
    }

    // Check for mandatory arguments
    if (server.empty()) {
        error("DNS server was not provided (-s server)", 2);
    }
    
    if (filter_file.empty()) {
        error("Filter file was not provided (-f filter_file)", 2);
    }
        
    // ----------------- Get DNS IP address -----------------

    std::string server_ip = get_dns_ip(server);

    // ------------------- Set UDP listen -------------------

    struct sockaddr_in6 myaddr6;

    // Create IPv6 socket
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        error("Could not create socket", errno);
    }

    // Turn off IPV6_V6ONLY flag
    ipv6only_off(&sockfd);

    memset((char *) &myaddr6, 0, sizeof(myaddr6));
    myaddr6.sin6_family = AF_INET6;
    myaddr6.sin6_addr = in6addr_any;
    myaddr6.sin6_port = htons(std::stoi(port));
        
    // Bind socket
    if (bind(sockfd, (struct sockaddr *) &myaddr6, sizeof(myaddr6)) < 0) {
        close(sockfd);
        error("Bind failed", errno);
    }

    char *host = (char *)server_ip.c_str();
    
    while (1) {

        // Buffer for recieving DNS requests
        char buffer[BUFSIZE] = {0};
        
        // Client address (e.g. dig)
        struct sockaddr_in6 clientaddr6;
        socklen_t clientaddrlen6 = sizeof(clientaddr6);
        
        ssize_t recvlen;
        recvlen = recvfrom(sockfd, (char *) buffer, BUFSIZE, 0, (struct sockaddr *) &clientaddr6, &clientaddrlen6);

        // ------------------- Multiprocessing -------------------

        int pid;
        if ((pid = fork()) > 0) {
            
            // Parent process
            // Continue waiting at the beginning of the cycle
        
        } else if (pid == 0) {
        
            // Child process
            // Process DNS query

            struct dnshdr *hdr = (dnshdr *)buffer;
            
            // ------------------- Find all QNAMEs -------------------

            std::string domain = get_domain(buffer);

            // ------------- Check if type of query is A -------------
            
            if (!is_type_a(buffer, domain.length())) {

                hdr->rcode = RCODE_NOT_IMPLEMENTED;
                hdr->qr = DNS_ANSWER;

                if (sendto(sockfd, buffer, recvlen, 0, (const struct sockaddr*) &clientaddr6, clientaddrlen6) < 0) {
                    error("sendto() failed!", errno);
                }

                exit(0);
            }

            // ------------------- Check filter file -------------------

            // If the domain from the query is subdomain of one of the domains from
            // the file, ignore the request and continue in the new iteration
            if (is_filtered(domain, blacklist)) {

                hdr->rcode = RCODE_REFUSE;
                hdr->qr = DNS_ANSWER;
                
                if (sendto(sockfd, buffer, recvlen, 0, (const struct sockaddr*) &clientaddr6, clientaddrlen6) < 0) {
                    error("sendto() failed!", errno);
                }
                
                exit(0);
            }
            
            // ------------------- Send DNS query -------------------

            // New socket to send and recieve messages to and from specified DNS server
            int newsockfd;

            // DNS server address
            struct sockaddr_in6 dnsaddr6;

            if ((newsockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
                error("could not create socket", errno);
            }

            // Set timeout for 3 seconds
            set_sock_timeout(&newsockfd, 3);
            
            // Turn off IPV6_V6ONLY flag
            ipv6only_off(&newsockfd);
            
            memset((char *) &dnsaddr6, 0, sizeof(dnsaddr6));
            dnsaddr6.sin6_family = AF_INET6;
            inet_pton(AF_INET6, host, &(dnsaddr6.sin6_addr));
            dnsaddr6.sin6_port = htons(53);

            if (sendto(newsockfd, buffer, recvlen, 0, (const struct sockaddr*) &dnsaddr6, sizeof(dnsaddr6)) < 0) {
                close(newsockfd);
                error("sendto() DNS server failed!", errno);
            }

            // ------------------- Recieve DNS answer -------------------

            socklen_t dnsaddrlen6 = sizeof(dnsaddr6);
            ssize_t recvlendns = recvfrom(newsockfd, buffer, BUFSIZE, 0, (struct sockaddr *) &dnsaddr6, &dnsaddrlen6);
            
            // --------------- Send DNS answer to client ---------------

            // Message was not recieved in timeout window (3s) => send error message
            if (recvlendns < 0) {
                hdr->rcode = RCODE_SERV_FAIL;
                hdr->qr = DNS_ANSWER;
                
                if (sendto(sockfd, buffer, recvlen, 0, (const struct sockaddr*) &clientaddr6, clientaddrlen6) < 0) {
                    close(newsockfd);
                    error("sendto() client (timeout) failed!", errno);
                }
            
            // Forward answer from DNS server to client
            } else {
                if (sendto(sockfd, buffer, recvlendns, 0, (const struct sockaddr*) &clientaddr6, sizeof(myaddr6)) < 0) {
                    close(newsockfd);
                    error("sendto() client failed!", errno);
                }
            }

            close(newsockfd);
            exit(0);
        }
    }
}
