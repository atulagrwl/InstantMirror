#ifndef IMDAEMON_H
#define IMDAEMON_H

#include "torrentStatus.h"

#define DISK_LOCATION "../www/"
#define TORRENT_LOCATION "../torrents/"

int checkRequest(const char *url);

std::string handleRequest(const char* url, torrentStatus &torrent_status_obj);

std::string getTorrentIndex(std::string urlPath);

std::string getTorrentFile(std::string torrentName, std::string filePath, torrentStatus &torrent_status_obj);

#endif