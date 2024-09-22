# workspace (all)

###########################################################

ifeq (,$(PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
$(error please specify PLATFORM, eg. PLATFORM=trimui make)
endif

###########################################################

.PHONY: all

all:
	cd ./all/minui/ && make
	cd ./all/minarch/ && make
	cd ./all/clear_recent/ && make
	cd ./all/set_poweroff_timeout/ && make

clean:
	cd ./all/minui/ && make clean
	cd ./all/minarch/ && make clean
	cd ./$(PLATFORM) && make clean