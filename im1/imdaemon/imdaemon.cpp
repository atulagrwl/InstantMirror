#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
//#include <stdio.h>
#include <unistd.h>

#include "imdaemon.h"
#include "torrentIndex.h"

#define SOCK_PATH "../socketfd"

#define DEFAULT_TORRENT_FILE "test"

int main(void)
{
    int s, s2;
    socklen_t len, t;
    struct sockaddr_un local, remote;
    char str[101];

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(s, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(s, 5) == -1) {
        perror("listen");
        exit(1);
    }

    for(;;) {
        int n;
        printf("Waiting for a connection...\n");
        t = sizeof(remote);
        if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
            perror("accept");
            exit(1);
        }

        printf("Connected.\n");

        n = recv(s2, str, 100, 0);
	if (n <= 0) {
	    if (n < 0) perror("recv");
	}
	
	printf("String passed is :%s\n",str);
	
	int valid = 0;
	valid = checkRequest(str);
	if(valid == 1)
	    send(s2,"1",2,0);
	else
	    send(s2,"0",2,0);
	
	if(valid == 1)
	{
	    std::string pathFile;
	    pathFile = handleRequest(str);
		
	    char* pathToSent = new char[pathFile.length()+1];
	    strcpy(pathToSent,pathFile.c_str());
	    pathToSent[pathFile.length()] = '\0';
	    printf("String passed back is %s\n",pathToSent);
	    if(send(s2,pathToSent,strlen(pathToSent)+1,0) == -1)
		perror("send error");
	}
	close(s2);
    }

    return 0;
}

int checkRequest(const char *url)
{
    std::string torrentPath;
    std::string path(&url[1]);
    torrentPath.assign(TORRENT_LOCATION);
    int index_t = path.find('/');
    if(index_t > -1)
	path = path.substr(0,index_t);
    torrentPath.append(path);
    
    struct stat pathStat;
    
    if (stat(torrentPath.c_str(),&pathStat) == -1)
    {
	std::cout<<"Error occured: Location checked for torrent is "<<torrentPath<<"\n";
	return 0;
	//exit(-1);
    }
    else
    {
	std::cout<<"Verified\n";
	return 1;
    }
}

std::string handleRequest(const char* url)
{
    std::string urlPath;
    std::string torrentName;
    std::string filePath;
    urlPath.assign(&url[1]);
    if(urlPath.length() == 0)
    {
	urlPath.append(DEFAULT_TORRENT_FILE);
    }
    printf("urlPath is %s\n",urlPath.c_str());
    
    int index = urlPath.find('/');
    
    if(index > 0)
    {
	torrentName = urlPath.substr(0,index);
	filePath = urlPath.substr(index+1);
	
	int ck_index = check(torrentName,filePath);
	if(ck_index > -1)
	{
	    std::cout<<"Enterning into getTorrent by index>0\n";
	    return getTorrentFile(torrentName, filePath);
	}
	else
	{
	    std::cout<<"Enterning into getTorrentIndex by index<0\n";
	    return getTorrentIndex(urlPath);
	}
    }
    else
    {
	struct stat pathStat;
    
	if (stat((TORRENT_LOCATION+urlPath).c_str(),&pathStat) == -1)
	{
	    printf("Error occured \n");
	    //exit(-1);
	}
	else
	{
	    if(S_ISREG(pathStat.st_mode))
	    {
		printf("getting into getTorrentindex\n");
		return getTorrentIndex(urlPath);
	    }
	}
    }
    
    return "Error";
    
}

std::string getTorrentIndex(std::string urlPath)
{
    torrentIndex myIndex(urlPath);
    myIndex.init();
    std::string path;
    int index_t = urlPath.find('/');
    if(index_t > -1)
	path = urlPath.substr(0,index_t);
    else
	path = urlPath;
    myIndex.setFileList(getList(2, path.c_str()));
    std::cout<<"before generateIndex\n";
    myIndex.generateIndex();
    std::cout<<"before saveindex\n";
    return myIndex.saveIndexFile();
}

std::string getTorrentFile(std::string torrentName, std::string filePath)
{
    std::cout<<"Torrent Name is "<<torrentName<<" File Path is "<<filePath<<"\n";
    getTorrent((torrentName+"/"+filePath).c_str());
    return torrentName+"/"+filePath;
}
