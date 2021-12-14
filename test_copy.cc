
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(){
	printf("hello! copy!\n");

	//test plc
	int fs_hz = 48000;
	size_t channels = 1;

	int fs_mult_ = fs_hz / 8000;
	int samples_10ms = static_cast<size_t>(10 * 8 * fs_mult_);

	FILE *pcm = fopen("48.pcm", "rb");
	if(pcm == NULL){
		printf("open pcm file failed!\n");
		return 0;
	}
	FILE *outfile = fopen("copy.pcm", "wb");

	int16_t buf[480];
	int16_t lastbuf[480];
	int read;
	int count = 0;
	while(!feof(pcm)){
		read = fread(buf, sizeof(int16_t), samples_10ms, pcm);
		if(read != samples_10ms){
			printf("read: %d\n", read);
			break;
		}
		count++;
		int lost = (count % 10 == 0);
		if(!lost){
			fwrite(buf, sizeof(int16_t), samples_10ms, outfile);
			memcpy(lastbuf, buf, sizeof(int16_t) * samples_10ms);
		}else{
			fwrite(lastbuf, sizeof(int16_t), samples_10ms, outfile);
		}
	}
	
	fflush(outfile);
	fclose(outfile);
	fclose(pcm);

	printf("leaving application!\n");
	return 0;
}
