#ifndef DUMP_TORRENT_H
#define DUMP_TORRENT_H

struct torfile_list
{
	int size;
	std::string name;
	int node_s;
	int node_f;
};

class tor
{
	public:
	tor() {};
	std::vector<torfile_list> gettor(int argc, char* argv);

};

std::vector<torfile_list> getList(int , char* );

extern "C" int sampleEx (char *);

extern "C" char** fileList (const char *);

extern "C" int check_file( const char *file );

extern "C" int getTorrent(const char* torrent);

int check (const std::string torrent_name, const std::string path);
#endif
