NDK_BUILD:=/home/aaron/share-aaron-1/android-ndk-r10e/ndk-build
SRCS=\
./dloader/pac.c ./dloader/nv.c ./dloader/test.c ./dloader/crc.c ./dloader/xml.c ./dloader/j_queue.c\
./FBFDownloader/WtptpDownloader.c ./FBFDownloader/asr1.c \
./aboot/ethos.c ./aboot/uip.c ./aboot/aboot.c \
./usbfs/usbfs.c main.c percent.c

CFLAGS += -Wall -Werror -O1 -s
LDFLAGS += -lpthread -ldl
OUT_DIR := out

ifeq ($(CC),cc)
CC=${CROSS_COMPILE}gcc
endif

all: check_dir clean
	#sed -i 's/\s\+$$//g' *.c *.h Makefile
	${CC} ${CFLAGS} ${SRCS} -o out/QDloader ${LDFLAGS}
	#gcc -Wall -g -o0 -DNV_MAIN dloader/nv.c -o list_nv

android: clean
	rm -rf android
	$(NDK_BUILD) V=0 APP_BUILD_SCRIPT=Android.mk NDK_PROJECT_PATH=`pwd` NDK_DEBUG=0 APP_ABI='armeabi-v7a,arm64-v8a'
	rm -rf obj
	mv libs android

clean:
	rm -rf list_nv out/QDloader lib

check_dir:
	@echo "Checking for '$(OUT_DIR)' directory..."
	@if [ ! -d "$(OUT_DIR)" ]; then \
		echo "'$(OUT_DIR)' directory does not exist. Creating..."; \
		mkdir $(OUT_DIR); \
	else \
		echo "'$(OUT_DIR)' directory already exists."; \
	fi
