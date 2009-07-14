#ifndef TORRENT_INDEX_H
#define TORRENT_INDEX_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "daemon.h"

enum Type {
	T_DIRECTORY,
	T_FILE,
    };
    
class torrentIndex
{
    private:
	std::string torrentPath;
	std::string indexstr;
	std::vector<torfile_list> fileList;
	std::string torrentName;
	std::string torrentFilePath;
	int fileNumber;
	
    
    public:
	torrentIndex(const std::string str);
    
    public:
	void init();
        
    private:
	void addTitleAndHtml();
    
    private:
	void addHeader();
    
    public:
	void setFileList(std::vector<torfile_list> list);
    
    public:
	void generateIndex();

    private:
	void addToList(Type fileType, std::string baseurl, std::string filename, time_t mtime, long size);
    
    private:
	void addFooter();
    
    public:
	std::string saveIndexFile();
    
    
};

#endif