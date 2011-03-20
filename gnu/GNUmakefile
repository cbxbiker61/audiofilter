
TOP = ..
VPATH = $(TOP)/tools:$(TOP)/lib:$(TOP)/lib/dsp:$(TOP)/lib/filters

ARCH=$(shell uname -m)
CXX = g++
DEFS = -D_GNU_SOURCE
OPTFLAGS = -g -O -Wall
INCLUDES = -I$(TOP)/include -I$(TOP)/lib
CXXFLAGS = $(ARCHFLAG) $(DEFS) $(OPTFLAGS) $(INCLUDES)
BINDIR = /usr/bin
LibName := AudioFilter
LIBS := -L. -l$(LibName)
acLib := lib$(LibName).a
acLibObjs := Ac3HeaderParser.o Ac3Parser.o AgcFilter.o AutoFile.o BitStream.o \
	Converter.o ConvertFunc.o Convolver.o ConvolverMch.o \
	DtsHdHeaderParser.o DtsHeaderParser.o DtsFrameParser.o FileParser.o \
	FilterGraph.o Fir.o Generator.o LinearFilter.o \
	MpaHeaderParser.o MpaFrameParser.o MpaSynth.o MpegDemuxer.o \
	MultiHeaderParser.o Parser.o Rng.o \
	SpdifHeaderParser.o SpdifFrameParser.o \
	SpdifWrapper.o Speakers.o SyncScan.o VArgs.o VTime.o \
	WavSink.o WavSource.o WinSpk.o

# equalizer requires Fft.o to build properly
# valdec.cpp has some dsound stuff that needs to be conditioned out
acLibObjsNotBuilding := Fft.o

progs := bsconvert noise mpeg_demux spdifer swab wavdiff
progsNotBuilding := equalizer valdec

default: all

ifeq ($(ARCH),x86_64)
LIBDIR = /usr/lib64
all :
	mkdir -p out.$(ARCH) out.x86
	$(MAKE) -C out.$(ARCH) -f ../GNUmakefile TOP=../$(TOP) $(acLib) $(progs)
	$(MAKE) -C out.x86 -f ../GNUmakefile TOP=../$(TOP) ARCHFLAG=-m32 $(acLib) $(progs)
install:
	make -C out.$(ARCH) -f ../GNUmakefile TOP=../$(TOP) install_bins install_libs install_headers
else
LIBDIR = /usr/lib
all :
	mkdir -p out.$(ARCH)
	$(MAKE) -C out.$(ARCH) -f ../GNUmakefile TOP=../$(TOP) $(progs)
install:
	make -C out.$(ARCH) -f ../GNUmakefile TOP=../$(TOP) install_bins install_libs install_headers
endif

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

ac3enc: ac3enc.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

bsconvert: bsconvert.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

equalizer: equalizer.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

noise: noise.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

mpeg_demux: mpeg_demux.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

spdifer: spdifer.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

swab: swab.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

valdec: valdec.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

wavdiff: wavdiff.o $(acLib)
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

$(acLib): $(acLibObjs)
	ar cru $@ $^

install_bins:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m755 $(progs) $(DESTDIR)$(BINDIR)

install_libs:
	mkdir -p $(DESTDIR)$(LIBDIR)
	install -m644 $(acLib) $(DESTDIR)$(LIBDIR)

install_headers:
	rm -rf $(DESTDIR)/usr/include/AudioFilter
	mkdir -p $(DESTDIR)/usr/include
	cp -a $(TOP)/include/AudioFilter $(DESTDIR)/usr/include

clean:
	rm -rf out.$(ARCH) out.x86

