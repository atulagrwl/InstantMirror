#ifndef DAEMON_H
#define DAEMON_H

#include "torrentStatus.h"

struct torfile_list
{
	long size;
	std::string name;
	int node_s;
	int node_f;
	time_t mtime;
	bool symlink_attribute;
	std::string symlink_path;
};

typedef struct 
{
	int size;
	char* name;
	int node_s;
	int node_f;
	time_t mtime;
	int symlink_attribute;
	char* symlink_path;
}torfile_list_C;

class tor
{
	public:
	tor() {};
	std::vector<torfile_list> gettor(int argc, const char* argv);

};

std::vector<torfile_list> getList(int , const char* );

int tor_list_C (const char* file, torfile_list_C* list[]);

char** fileList (const char *);

int check_file( const char *file );

int getTorrent(const char* torrent, torrentStatus &torrent_status_obj);

int check (const std::string torrent_name, const std::string path);


#endif
