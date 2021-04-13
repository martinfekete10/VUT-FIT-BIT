/**
 * @name:   ISA projekt - Filtrujúci DNS resolver 
 * @author: Martin Fekete <xfeket00@stud.fit.vutbr.cz>
 * @date:   29.10.2020
 */

/**
 * Struct was written according to RFC 1035
 */
typedef struct dnshdr {
    
    unsigned id: 16;
    
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    unsigned rd: 1;
    unsigned tc: 1;
    unsigned aa: 1;
    unsigned opcode: 4;
    unsigned qr: 1;
    unsigned rcode: 4;
    unsigned z: 3;
    unsigned ra: 1;
    #else
    unsigned qr: 1;
    unsigned opcode: 4;
    unsigned aa: 1;
    unsigned tc: 1;
    unsigned rd: 1;
    unsigned ra: 1;
    unsigned z: 3;
    unsigned rcode: 4;    
    #endif

    unsigned qdcount: 16;
    unsigned ancount: 16;
    unsigned nscount: 16;
    unsigned arcount: 16;

} t_dnshdr;


/**
 * Struct was written according to RFC 1035
 */
typedef struct question {
    
    unsigned char *qname;
    unsigned qtype: 16;
    unsigned qclass: 16;

} t_question;
