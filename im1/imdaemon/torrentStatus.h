#ifndef TORRENT_STATUS_H
#define TORRENT_STATUS_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "libtorrent/torrent_info.hpp"

class torrentStatus
{
    private:
	bool empty;
	
    public:
	std::string fileName;
	std::string filePath;
	std::time_t timestamp;
	int totalFiles;
	//std::vector<bool> downloadedFile;
	bool *fileStatus;
	libtorrent::torrent_info *torrent_details;
	
    public:
	torrentStatus()
	{
	    empty=true;
	    std::cout<<"New torrentStatus object created\n";
	}
	
    public:
	void snapshotMigration(const char* path);
	
    public:
	bool isTorrentValid (std::time_t oldTime);
	
    public:
	void init();
	
    public:
	bool isEmpty();
	
    public:
	void print();
	
    public:
	void setEmpty(bool e);
};

#endif