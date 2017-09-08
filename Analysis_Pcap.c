#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 10240
#define STRSIZE 1024
struct pcap_file_header{
	unsigned int magic_number;
	unsigned short version_major;
	unsigned short version_minor;
	unsigned int thiszone;
	unsigned int sigfigs;
	unsigned int snaplen;
	unsigned int linktype;
};
struct pcap_packet_header{
	unsigned int GMTtime;
	unsigned int MicroTime;
	unsigned int caplen;
	unsigned int len;
};
struct ethernet_header{
	unsigned char DstMac[6];
	unsigned char SrcMac[6];
	unsigned short Type;
	  		
};
void ShowHeader(void *buf,int type);

int main(int argc, const char *argv[])
{
	struct pcap_file_header FileHeader;
	struct pcap_packet_header PktHeader;
	struct ethernet_header EthHeader;
	int rfd = 0,wfd = 0;
	unsigned int nread = 0;
	char buf[BUFSIZE] = {0};
	unsigned long count = 0;
	rfd = open(argv[1],O_RDONLY);
	if (rfd < 0) {
		printf("open pcap faild\n");
		return -1;
	} 
	wfd = open(argv[2],O_WRONLY|O_CREAT);
	if (wfd < 0) {
		printf("open out faild\n");
		return -1;
	}
	nread = read(rfd,buf,24);
	if (nread < 24) {
		printf("read pcap file header faild\n");
		return -1;
	}
	memcpy(&FileHeader,buf,sizeof(FileHeader));		
	ShowHeader((void *)&FileHeader,0);	
	
	while(nread > 0) {
		count++;
		nread = read(rfd,buf,16);
		if (nread < 16) {
			printf("read pacp packet header faild nread:%x,count:%x\n",nread,count);
			return -1;	
		}	
		memset(&PktHeader,0,sizeof(PktHeader));
		memcpy(&PktHeader,buf,16);
		ShowHeader(&PktHeader,1);
		nread = read(rfd,buf,14);
		if (nread < 14) {
			printf("read Ethernet Header faild\n");
			return -1;		
		}
		memset(&EthHeader,0,sizeof(struct ethernet_header));
		memcpy(&EthHeader,buf,14);
		ShowHeader(&EthHeader,2);
		nread = read(rfd,buf,PktHeader.len - 14);
		if (nread < PktHeader.len - 14) {
			printf("read Data faild\n");
			return -1;
		}
		write(wfd,buf,nread);	
	}		



	return 0;
}

void ShowHeader(void *buf,int type)
{
	if (type == 0) {
		struct pcap_file_header *p = (struct pcap_file_header *)buf;		
		printf("magic_number:%x,version_major:%x,version_minor:%x,thiszone:%x,sigfigs:%x,snaplen:%x,linktype:%x\n",
			p->magic_number,p->version_major,p->version_minor,p->thiszone,p->sigfigs,p->snaplen,p->linktype);	
	}else if (type == 1) {
		struct pcap_packet_header *p = (struct pcap_packet_header *)buf;
		printf("GMTtime:%x,MicroTime:%x,caplen:%x,len:%x\n",p->GMTtime,p->MicroTime,p->caplen,p->len);
	}else if(type == 2){
		struct ethernet_header *p = (struct ethernet_header *)buf;
		printf("DstMac:%x:%x:%x:%x:%x:%x,SrcMac:%x:%x:%x:%x:%x:%x,Type:%x\n",(p->DstMac)[0],(p->DstMac)[1],(p->DstMac)[2],(p->DstMac)[3],(p->DstMac)[4],(p->DstMac)[5],
			(p->SrcMac)[0],(p->SrcMac)[1],(p->SrcMac)[2],(p->SrcMac)[3],(p->SrcMac)[4],(p->SrcMac)[5],p->Type);
	}else {
		printf("Show buf faild type not found\n");
		return ;
	}
}
