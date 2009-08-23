all: im1

daemon.o: daemon.cpp
	gcc -o daemon.o daemon.cpp -c -pthread -I/usr/include -ftemplate-depth-50 -Ilibtorrent/include -Ilibtorrent/include/libtorrent -Os -pthread -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -g

imdaemon.o: imdaemon.cpp
	gcc -o imdaemon.o imdaemon.cpp -c -pthread -I/usr/include -ftemplate-depth-50 -Ilibtorrent/include -Ilibtorrent/include/libtorrent -Os -pthread -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -g

torrentIndex.o: torrentIndex.cpp
	gcc -o torrentIndex.o torrentIndex.cpp -c -pthread -I/usr/include -ftemplate-depth-50 -Ilibtorrent/include -Ilibtorrent/include/libtorrent -Os -pthread -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -g
	
torrentStatus.o: torrentStatus.cpp
	gcc -o torrentStatus.o torrentStatus.cpp -c -pthread -I/usr/include -ftemplate-depth-50 -Ilibtorrent/include -Ilibtorrent/include/libtorrent -Os -pthread -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -g
	
im1: daemon.o imdaemon.o torrentIndex.o torrentStatus.o 
	g++ -ftemplate-depth-50 -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -DHAVE_SSL -DTORRENT_USE_OPENSSL -DTORRENT_LINKING_SHARED -Ilibtorrent/include -Ilibtorrent/include/libtorrent -I/usr/include/openssl -L/libtorrent/src/.libs -Wl,-rpath libtorrent/src/.libs -o im libtorrent/src/.libs/libtorrent-rasterbar.so daemon.o imdaemon.o torrentIndex.o torrentStatus.o -g

clean:
	rm -f *.o
	rm -f im
