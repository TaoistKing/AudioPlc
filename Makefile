CC = llvm-gcc
CXX = llvm-g++ -std=c++11
CFLAGS = -Os -fno-exceptions -fvisibility=hidden -DNDEBUG -DWEBRTC_POSIX
INC = -I.

APP = plctest
DSPSRC = $(wildcard common_audio/signal_processing/*.c)
#OBJS = $(patsubst %.c, %.o, $(SRC))
DSPOBJS = $(DSPSRC:%.c=%.o)
DSPLIB = libsignal_processing.a
BASESRC = $(wildcard base/*.cc)
BASEOBJS = $(BASESRC:%.cc=%.o)
BASELIB = libbase.a
VADSRC = $(wildcard common_audio/vad/*.c)
VADOBJS = $(VADSRC:%.c=%.o)
VADLIB = libwebrtc_vad.a
DYLIBS = libbase.dylib libvad.dylib libdsp.dylib

SCALESRC = common_audio/signal_processing/dot_product_with_scale.cc
SCALEOBJ = common_audio/signal_processing/dot_product_with_scale.o
DSPSRC += common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor.c

app: $(APP)
dsp: $(DSPLIB)
base: $(BASELIB)
vad: $(VADLIB)
mute: mutetest
copy: copytest
noise: noisetest
dylib: $(DYLIBS)

#$(APP): test.cc $(DSPLIB) $(BASELIB)
#	$(CXX) $< -o $@ -I. -L. -lsignal_processing -lrtc_base

$(APP): test_plc.cc $(DSPLIB) $(VADLIB) $(BASELIB)
	$(CXX) $< -o $@ -I. -L. -Lneteq -lsignal_processing -lwebrtc_vad -lbase -lplc

$(DSPLIB): $(DSPOBJS) $(SCALEOBJ)
	ar cr $@ $^

$(BASELIB): $(BASEOBJS)
	ar cr $@ $^

$(VADLIB): $(VADOBJS)
	ar cr $@ $^

$(DSPOBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)

$(SCALEOBJ): $(SCALESRC)
	$(CXX) $(CFLAGS) -c $< -o $@ $(INC)

$(BASEOBJS): %.o: %.cc
	$(CXX) $(CFLAGS) -c $< -o $@ $(INC)

$(VADOBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INC)

mutetest: test_mute.cc
	$(CXX) $(CFLAGS) $< -o $@

copytest: test_copy.cc
	$(CXX) $(CFLAGS) $< -o $@ $(INC)

noisetest: test_noise.cc
	$(CXX) $(CFLAGS) $< -o $@ $(INC) -L. -Lneteq -lsignal_processing -lplc -lwebrtc_vad -lbase

libbase.dylib: $(BASEOBJS)
	$(CXX) -dynamiclib -undefined suppress -flat_namespace $^ -o $@
libvad.dylib: $(VADOBJS)
	$(CXX) -dynamiclib -undefined suppress -flat_namespace $^ -o $@
libdsp.dylib: $(DSPOBJS) $(SCALEOBJ)
	$(CXX) -dynamiclib -undefined suppress -flat_namespace $^ -o $@
	
all: $(APP) $(BASELIB) $(VADLIB)

.PHONY: clean

clean:
	-rm $(DSPOBJS) $(DSPLIB) $(VADOBJS) $(VADLIB) $(BASEOBJS) $(BASELIB) $(APP) $(DYLIBS)
