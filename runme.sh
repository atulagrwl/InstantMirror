#!/bin/bash

#killall run
rm run
gcc -c mini_httpd.c match.c tdate_parse.c htpasswd.c -g
g++ -c dump_torrent.cpp -g
g++ -ftemplate-depth-50 -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -DHAVE_SSL -DTORRENT_USE_OPENSSL -DTORRENT_LINKING_SHARED -I/usr/local/include -I/usr/local/include/libtorrent -I/usr/include/openssl -L/usr/local/lib -ltorrent-rasterbar -o run dump_torrent.o mini_httpd.o match.o tdate_parse.o -lcrypt -g
./run -d www/
