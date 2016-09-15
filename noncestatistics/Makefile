# This Makefile is used for STATIC compilation ONLY!
CC=clang
CXX=clang
CXXFLAGS=-g -std=c++11 -Wall -pedantic
STATICLIBS=/usr/local/lib/libplist.a /usr/local/lib/libxml2.a /usr/local/lib/liblzma.a /usr/local/lib/libimobiledevice.a /usr/local/lib/libusbmuxd.a /usr/local/lib/libirecovery.a /usr/local/Cellar/openssl/1.0.2h_1/lib/libcrypto.a
LDFLAGS=-liconv -lz -framework IOKit -lc++ -framework CoreFoundation -lssl 


TARGET=noncestatistics

all: $(TARGET)

noncestatistics: main.o common.o dfu.o idevicerestore.o normal.o recovery.o stats.o
	$(CXX) $(LDFLAGS) $(STATICLIBS) main.o common.o dfu.o idevicerestore.o normal.o recovery.o stats.o -o $(TARGET)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

common.o: common.c
	$(CC) -c common.c

dfu.o: dfu.c
	$(CC) -c dfu.c

idevicerestore.o: idevicerestore.c
	$(CC) -c idevicerestore.c

normal.o: normal.c
	$(CC) -c normal.c

recovery.o: recovery.c
	$(CC) -c recovery.c

stats.o: stats.cpp
	$(CXX) $(CXXFLAGS) -c stats.cpp 

release: noncestatistics.zip

noncestatistics.zip: noncestatistics
	zip -9 $(TARGET).zip $(TARGET)

clean:
	rm $(TARGET).zip
	rm *.o
	rm $(TARGET)
	
