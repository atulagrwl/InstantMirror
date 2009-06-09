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
	g++ -c dump_torrent.cpp -g
im0: mini_httpd.o match.o tdate_parse.o htpasswd.o dump_torrent.o
	g++ -ftemplate-depth-50 -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -DHAVE_SSL -DTORRENT_USE_OPENSSL -DTORRENT_LINKING_SHARED -I/usr/local/include -I/usr/local/include/libtorrent -I/usr/include/openssl -L/usr/local/lib -ltorrent-rasterbar -o im0 dump_torrent.o mini_httpd.o match.o tdate_parse.o -lcrypt -g

clean:
	rm -f *.o
	rm -f im0
