/*获取TS文件视频长度
 *本篇大部分还是扣的live555的代码。live555代码中有计算ts的clock的值的，根据clock决定rtp的发包间隔。将clock收集起来，从最后一个clock减去第
 *一个clock就能得到TS文件的大概长度。 本小程序前提是视频文件必须是TS文件，第一个字节是0x47，每个TS包大小188，其余情况未考虑。
 */

#include <stdio.h>
#define TS_SYNC_BYTE 0x47
#define TS_PACKET_SIZE 188
typedef struct {
	unsigned pid;
	double clock_begin;
	double clock_end;
}pid_t;
pid_t pid_array[8191];
unsigned char buf[TS_PACKET_SIZE];
void get_length(unsigned char *pkt);
void store_pid(unsigned pid, double clock);
int main(int argc, const char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "please use %s \n", argv[0]); 
		return -1;
	}
	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		perror("fopen");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	unsigned int size = ftell(fp);
	rewind(fp);
	while(size > 0) {
		unsigned read_size = fread(buf, 1, sizeof(buf), fp);
		size -= read_size;
		get_length(buf);
	}
	int i;
	for (i = 0; i < 8191; i++) {
		if (pid_array[i].pid != 0) {
			printf("PID:0x%x length:%fs\n", pid_array[i].pid,pid_array[i].clock_end - pid_array[i].clock_begin); 
		} else {
			break;
		}
	}
	
	
	return 0;
}
void get_length(unsigned char *pkt) {
	// Sanity check: Make sure we start with the sync byte:
	if (pkt[0] != TS_SYNC_BYTE) {
		fprintf(stderr,"Missing sync byte!\n");
		return ;
	}
	// If this packet doesn't contain a PCR, then we're not interested in it:
	unsigned char const adapter_field_control = (pkt[3] & 0x30) >> 4;
	if (adapter_field_control != 0x2 && adapter_field_control != 0x3) {
		
		return ;
	}
	// there's no adaptation_field 
	unsigned char const adapter_field_lenght = pkt[4];
	if (adapter_field_lenght == 0) {
		
		return ;
	}
	// no PCR  
	unsigned char const pcr_flag = pkt[5] & 0x10;
	if (pcr_flag == 0) {
		
		return ;
	}
	// yes, we get a pcr  
	unsigned int  pcr_base_high = (pkt[6] << 24) | (pkt[7] << 16) | (pkt[8] << 8) | (pkt[9]);
	// caculate the clock  
	double clock = pcr_base_high / 45000.0;
	if (pkt[10] & 0x80) {
		clock += 1 / 90000.0; // add in low-bit (if set)  
	}
	unsigned short pcr_extra = ((pkt[10] & 0x01) << 8) | pkt[11];
	clock += pcr_extra / 27000000.0;
	unsigned pid = ((pkt[1] & 0x1f) << 8) | pkt[2];
	store_pid(pid, clock);
}
void store_pid(unsigned pid, double clock) {
	int i;
	for (i = 0; i < 8191; i++) {
		if (pid == pid_array[i].pid) {
			break;
		}
	}
	if (i == 8191) {
		for (i = 0; i < 8191; i++) {
			if (pid_array[i].pid == 0) {
				break;
			}
		}
		pid_array[i].pid = pid;
		pid_array[i].clock_begin = clock;
	} else {
		pid_array[i].clock_end = clock;
	}
}
 
