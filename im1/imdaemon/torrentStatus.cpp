#include "torrentStatus.h"

namespace lb=libtorrent;

bool torrentStatus::isTorrentValid (time_t oldTime)
{
    if(oldTime == timestamp)
	return true;
    else
	return false;
}

void torrentStatus::init ()
{
    std::cout<<"torrentStatus.init() called \n";
    totalFiles = torrent_details->num_files();
    fileStatus = new bool[totalFiles];
    for(int i=0; i<totalFiles; i++)
	fileStatus[i] = false;
    //for(int i=0; i< totalFiles; i++)
	//downloadedFile.push_back(false);
    //downloadedFile.resize(totalFiles, false);
}

void torrentStatus::snapshotMigration (const char* path)
{
    lb::torrent_info *new_torrent_details = new lb::torrent_info(path);
    struct stat stt;
    if (lstat(path, &stt) >= 0) 
	timestamp = stt.st_mtime;
    int newTotalFiles = new_torrent_details->num_files();
    
    bool *newFileStatus = new bool[newTotalFiles];
    
    for(int i=0; i<newTotalFiles; i++)
	newFileStatus[i] = false;
    
    int indexint = 0;
    for(lb::torrent_info::file_iterator i = torrent_details->begin_files() ;
		i != torrent_details->end_files(); ++i, ++indexint)
    {
	if(fileStatus[indexint] == false)
	    continue;
	
	int indexintj = 0;
	for(lb::torrent_info::file_iterator j = new_torrent_details->begin_files();
		i != new_torrent_details->end_files(); ++j, ++indexintj)
	{
	    if(i->path.string() == j->path.string())
	    {
		if(i->mtime == j->mtime)
		    newFileStatus[indexintj] = true;
	    }
	}
    }
    fileStatus = newFileStatus;
    totalFiles = newTotalFiles;
}

bool torrentStatus::isEmpty()
{
    return empty;
}

void torrentStatus::print()
{
    std::cout << "Inside torrentStatus::print() \n";
    std::cout << "Size of vector is :"<<totalFiles << "\n";
    for(int i = 0 ; i < totalFiles ; i++)
    {
	std::cout<<i<<":"<<fileStatus[i]<<"\t";
    }
}

void torrentStatus::setEmpty(bool e)
{
    empty = e;
}
