
#include <stdio.h>
#include "common_audio/signal_processing/include/signal_processing_library.h"
#include "neteq/expand.h"
#include "neteq/merge.h"
#include "neteq/audio_multi_vector.h"
#include "neteq/sync_buffer.h"
#include "neteq/background_noise.h"
#include "neteq/random_vector.h"
#include "neteq/post_decode_vad.h"

static const int kInputSizeMs = 50;
static const int kOutputSizeMs = 30;
static const size_t kMaxFrameSize = 5760;  // 120 ms @ 48 kHz.
static const size_t kSyncBufferSize = kMaxFrameSize + 60 * 48;


#define kMode_Normal 0
#define kMode_Expand 1
#define kMode_Merge  2

using namespace webrtc;

int main(){
	printf("hello! plc!\n");

	//test signal processing lib
	int16_t input[16] = {100};
  	WebRtcSpl_ComplexBitReverse(input, 3);
	int ret = WebRtcSpl_ComplexFFT(input, 3, 1);
	printf("fft result: %d\n", ret);

	//test plc
	int fs_hz = 48000;
	size_t channels = 1;

	std::unique_ptr<BackgroundNoise> background_noise_;
	std::unique_ptr<AudioMultiVector> algorithm_buffer_;
	std::unique_ptr<SyncBuffer> sync_buffer_;
	std::unique_ptr<Expand> expand_;
    	std::unique_ptr<ExpandFactory> expand_factory_;
	std::unique_ptr<Merge> merge_;
	RandomVector random_vector_;
  	std::unique_ptr<PostDecodeVad> vad_;

	int fs_mult_ = fs_hz / 8000;
	int samples_10ms = static_cast<size_t>(10 * 8 * fs_mult_);
	int input_size_samples_ = static_cast<size_t>(kInputSizeMs * 8 * fs_mult_);
	int output_size_samples_ = static_cast<size_t>(kOutputSizeMs * 8 * fs_mult_);
	int decoder_frame_length_ = 3 * output_size_samples_;  // Initialize to 30ms.

	// Reinit post-decode VAD with new sample rate.
	vad_.reset(new PostDecodeVad());
	vad_->Init();

	// Delete algorithm buffer and create a new one.
	algorithm_buffer_.reset(new AudioMultiVector(channels));

	// Delete sync buffer and create a new one.
	sync_buffer_.reset(new SyncBuffer(channels, kSyncBufferSize * fs_mult_));

	// Delete BackgroundNoise object and create a new one.
	background_noise_.reset(new BackgroundNoise(channels));

	// Reset random vector.
	random_vector_.Reset();

	expand_factory_.reset(new ExpandFactory);
	expand_.reset(expand_factory_->Create(background_noise_.get(),
					sync_buffer_.get(), &random_vector_,
					fs_hz, channels));

  	merge_.reset(new Merge(fs_hz, channels, expand_.get(), sync_buffer_.get()));

	// Move index so that we create a small set of future samples (all 0).
	sync_buffer_->set_next_index(sync_buffer_->next_index() -
			       expand_->overlap_length());

	FILE *pcm = fopen("48.pcm", "rb");
	if(pcm == NULL){
		printf("open pcm file failed!\n");
		return 0;
	}
	FILE *outfile = fopen("out.pcm", "wb");
	FILE *plcfile = fopen("plc.pcm", "wb");
	AudioFrame frame;

	AudioMultiVector *mv = algorithm_buffer_.get();
	AudioVector &v = (*mv)[0];	
	int16_t buf[480];
	int read;
	int last_mode = kMode_Normal;
	int count = 0;
	while(!feof(pcm)){
		read = fread(buf, sizeof(int16_t), samples_10ms, pcm);
		if(read != samples_10ms){
			printf("read: %d\n", read);
			break;
		}
		count++;
		int lost = (count % 10 == 0);
		//int lost = 0;
		if(!lost){
			printf("not lost\n");
			if(last_mode == kMode_Expand){
				//merge
      				merge_->Process(buf, samples_10ms, algorithm_buffer_.get());
				last_mode = kMode_Merge;
			}else{
				//normal_->Process(buf, samples_10ms, last_mode, algorithm_buffer_.get());
				//normal
				v.PushBack(buf, samples_10ms);
				last_mode = kMode_Normal;
			}
			sync_buffer_->PushBack(*algorithm_buffer_);
			algorithm_buffer_->Clear();
		}else{
			printf("lost\n");
			//lost. do plc and get 10 ms data
			int samples = 0;
			while(samples < samples_10ms){
				algorithm_buffer_->Clear();
				int return_value = expand_->Process(algorithm_buffer_.get());
				size_t length = algorithm_buffer_->Size();
				if (return_value < 0) {
					printf("expand return %d, length:%lu\n", return_value, length);
					break;
				}
				if(expand_->MuteFactor(0) == 0){
					printf("only noise generated\n");
				}else{
					printf("more than noise generated\n");
				}
				sync_buffer_->PushBack(*algorithm_buffer_);
				last_mode = kMode_Expand;

				int16_t tmp[480];
				algorithm_buffer_->ReadInterleaved(length, tmp);
				fwrite(tmp, sizeof(int16_t), length, plcfile);

				algorithm_buffer_->Clear();
				//samples += length;
				samples += samples_10ms;
				printf("adding %d samples\n", samples);
			}
		}

		if(sync_buffer_->Size() > 5 * samples_10ms){
			sync_buffer_->GetNextAudioInterleaved(samples_10ms, &frame);	
			if(frame.data()){
				fwrite(frame.data(), sizeof(int16_t), samples_10ms, outfile);
			}	
		}
	}

	
	fflush(outfile);
	fclose(outfile);
	fclose(pcm);

	printf("leaving application!\n");
	return 0;
}
