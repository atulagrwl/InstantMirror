#ifndef DUMP_TORRENT_H
#define DUMP_TORRENT_H

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

#ifdef __cplusplus
struct torfile_list
{
	int size;
	std::string name;
	int node_s;
	int node_f;
	std::time_t mtime;
	bool symlink_attribute;
	std::string symlink_path;
};

class tor
{
	public:
	tor() {};
	std::vector<torfile_list> gettor(int argc, char* argv);

};

std::vector<torfile_list> getList(int , char* );

extern "C" int tor_list_C (const char* file, torfile_list_C* list[]);

extern "C" char** fileList (const char *);

extern "C" int check_file( const char *file );

extern "C" int getTorrent(const char* torrent);

int check (const std::string torrent_name, const std::string path);
#endif

#endif
