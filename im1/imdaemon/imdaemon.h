#ifndef IMDAEMON_H
#define IMDAEMON_H

#define DISK_LOCATION "../www/"
#define TORRENT_LOCATION "../torrents/"

int checkRequest(const char *url);

std::string handleRequest(const char* url);

std::string getTorrentIndex(std::string urlPath);

std::string getTorrentFile(std::string torrentName, std::string filePath);

#endif