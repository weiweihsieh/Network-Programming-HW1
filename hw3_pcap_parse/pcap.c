#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <time.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>

#define MAX 1024

int count = 1;

void packetHandler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet);

int main(int argc, char *argv[])
{
    pcap_t *src;
    char *filename;
    char *filter = ""; //filter expression
    char errbuf[PCAP_ERRBUF_SIZE];    
    struct bpf_program fcode; //compiled filter expression

    /*command line*/
    filename = argv[1];
    if(argc == 3)
        filter = argv[2];

    /*open the pcap file*/ 
    src = pcap_open_offline(filename, errbuf);
    if(src == NULL)
    {
        fprintf(stderr, "%s\n", errbuf);
        return 1;
    }
    
    /*filter*/
    if(pcap_compile(src, &fcode, filter, 1, PCAP_NETMASK_UNKNOWN) == -1) //compile string to a filter program
    {
        fprintf(stderr, "%s\n", pcap_geterr(src));
        return 1;
    }
    
    if(pcap_setfilter(src, &fcode) == -1) //specify a filter program
    {
        fprintf(stderr, "%s\n", pcap_geterr(src));
        return 1;
    }

    /*process packets*/
    printf("%-5s%-22s%-18s%-18s%-10s%-10s%-5s\n", "No.", "Time", "Src IP", "Dst IP", "Src Port", "Dst Port", "Length");
    printf("-------------------------------------------------------------------------------------------\n");

    if(pcap_loop(src, 0, packetHandler, NULL) < 0)
    {
        fprintf(stderr, "%s\n", pcap_geterr(src));
        return 1;
    }
    
    
    pcap_close(src);

    return 0;
}

void packetHandler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet)
{
    int packetLen; 
    char time_format[30];
    struct tm *ltime;
    struct ether_header *ethernetHeader;
    struct ip *ipHeader;
    char srcIp[INET_ADDRSTRLEN] = {0};
    char dstIp[INET_ADDRSTRLEN] = {0};
    u_char protocal;
    struct udphdr *udpHeader;
    struct tcphdr *tcpHeader;
    u_int16_t srcPort;
    u_int16_t dstPort;
    
    /*parse packet*/
    ethernetHeader = (struct ether_header *)packet;
    if(ntohs(ethernetHeader->ether_type) == ETHERTYPE_IP) //IPv4
    {
        ipHeader = (struct ip *)(packet + sizeof(struct ether_header));
        
        /*IP*/
        inet_ntop(AF_INET, &ipHeader->ip_src, srcIp, INET_ADDRSTRLEN); //source IP
        inet_ntop(AF_INET, &ipHeader->ip_dst, dstIp, INET_ADDRSTRLEN); //destination IP
        
        protocal = ipHeader->ip_p; //protocal of next layer
        
        /*port*/
        if(protocal == IPPROTO_UDP) //UDP
        {
            udpHeader = (struct udphdr *)(packet + sizeof(struct ether_header) + sizeof(struct ip));
            srcPort = ntohs(udpHeader->uh_sport); //source port
            dstPort = ntohs(udpHeader->uh_dport); //destination port
        }
        else if(protocal == IPPROTO_TCP) //TCP
        {
            tcpHeader = (struct tcphdr *)(packet + sizeof(struct ether_header) + sizeof(struct ip));
            srcPort = ntohs(tcpHeader->th_sport); //source port
            dstPort = ntohs(tcpHeader->th_dport); //destination port
        }
    }

    /*time*/
    ltime = localtime(&header->ts.tv_sec);
    strftime(time_format, sizeof(time_format), "%Y/%m/%d %H:%M:%S", ltime);
    
    /*length*/
    packetLen = header->len;       
    
    /*printf the information*/
    printf("%-5d%-22s%-18s%-18s%-10d%-10d%-5d\n", count, time_format, srcIp, dstIp, srcPort, dstPort, packetLen);
    count++;
}
