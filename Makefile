all: im0

htpasswd.o: htpasswd.c
	gcc -c htpasswd.c -g
match.o: match.c
	gcc -c match.c -g
mini_httpd.o: mini_httpd.c
	gcc -c mini_httpd.c -g
tdate_parse.o: tdate_parse.c
	gcc -c tdate_parse.c -g
dump_torrent.o: dump_torrent.cpp
	gcc -o dump_torrent.o dump_torrent.cpp -c -pthread -I/usr/include -ftemplate-depth-50 -Ilibtorrent/include -Ilibtorrent/include/libtorrent -Os -pthread -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -g
im0: mini_httpd.o match.o tdate_parse.o htpasswd.o dump_torrent.o
	g++ -ftemplate-depth-50 -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -DHAVE_SSL -DTORRENT_USE_OPENSSL -DTORRENT_LINKING_SHARED -Ilibtorrent/include -Ilibtorrent/include/libtorrent -I/usr/include/openssl -L/libtorrent/src/.libs -Wl,-rpath libtorrent/src/.libs -o im0 libtorrent/src/.libs/libtorrent-rasterbar.so dump_torrent.o mini_httpd.o match.o tdate_parse.o -lcrypt -g

clean:
	rm -f *.o
	rm -f im0
