ifneq "$(wildcard dependencies.mk)" "dependencies.mk"
$(error Error: could not find file `dependencies.mk`. Please make a copy of `dependencies.mk.template`, rename the copy to `dependencies.mk`, and adjust the copy as necessary before attempting to build again.)
endif

# Get all of the locations for dependencies
# Modify dependencies.mk to set the locations/commands for each tool
include dependencies.mk

INCLUDE := -i ./ -I- -i $(PATH_TO_GAME) -i $(PATH_TO_RFL) -i $(PATH_TO_RVL) -I- -i $(PATH_TO_TRK) -I- -i $(PATH_TO_RUNTIME) -I- -i $(PATH_TO_MSL_C) -I- -i $(PATH_TO_MSL_CPP) -I- -i $(PATH_TO_JSYS) -I- -i $(PATH_TO_NW4R) -I- -i $(BUSSUN_INCLUDE)

WARNFLAGS := -w all -pragma "warning off (10122)"

CXXFLAGS := -c -Cpp_exceptions off -nodefaults -proc gekko -fp hard -ipa file -inline auto,level=2 -O4,s -rtti off -sdata 0 -sdata2 0 -align powerpc -enum int $(INCLUDE) $(WARNFLAGS)

CFLAGS := -lang c99 $(CXXFLAGS)

#ASMFLAGS := -c -proc gecko

O_FILES := net.o packets.o 

ifndef SYMBOL_MAP
$(error Error: no symbol map specified)
endif

export SYMBOL_MAP
SYMBOL_OSEV_ADDR := $(shell export SYMBOL_MAP="$(SYMBOL_MAP)" ; line=`grep OSExceptionVector $$SYMBOL_MAP` ; echo $${line#*=})

.PHONY: clean

all: interrupts.xml CustomCode_USA.bin

clean:
	rm -f $(O_FILES) $(all)

#%.o: %.s
#	$(AS) $(ASMFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXLAGS) $(DEFINES) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

interrupts.xml: interruptSubs.o
	$(KAMEK) -static=$(SYMBOL_OSEV_ADDR) -externals=$(SYMBOL_MAP) -output-riiv=$@ interruptSubs.o

CustomCode_USA.bin: $(O_FILES)
	$(KAMEK) -externals=$(SYMBOL_MAP) -output-kamek=$@ $(O_FILES)
