#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/lazy_entry.hpp"
#include "libtorrent/magnet_uri.hpp"
#include <boost/filesystem/operations.hpp>
#include "libtorrent/session.hpp"
#include "libtorrent/alert.hpp"
#include "libtorrent/alert_types.hpp"

#include "dump_torrent.h"

using namespace libtorrent;
using namespace boost::filesystem;

std::vector<torfile_list> tor::gettor(int argc, char* argv)
{
	std::vector<torfile_list> tor_list;

	if (argc != 2)
	{
		std::cerr << "usage: dump_torrent torrent-file\n";
		exit(-1) ;
	}
#if BOOST_VERSION < 103400
	boost::filesystem::path::default_name_check(boost::filesystem::no_check);
#endif

#ifndef BOOST_NO_EXCEPTIONS
	try
	{
#endif

		int size = file_size(argv);
		if (size > 10 * 1000000)
		{
			std::cerr << "file too big (" << size << "), aborting\n";
			exit(-1);
		}
		std::vector<char> buf(size);
		std::ifstream(argv, std::ios_base::binary).read(&buf[0], size);
		lazy_entry e;
		int ret = lazy_bdecode(&buf[0], &buf[0] + buf.size(), e);

		if (ret != 0)
		{
			std::cerr << "invalid bencoding: " << ret << std::endl;
			exit(-1);
		}

//			std::cout << "\n\n----- raw info -----\n\n";
//			std::cout << e << std::endl;

		torrent_info t(e);

		// print info about torrent
//			std::cout << "\n\n----- torrent file info -----\n\n";
//			std::cout << "nodes:\n";
		typedef std::vector<std::pair<std::string, int> > node_vec;
		node_vec const& nodes = t.nodes();
		for (node_vec::const_iterator i = nodes.begin(), end(nodes.end());
			i != end; ++i)
		{
			std::cout << i->first << ":" << i->second << "\n";
		}
//			std::cout << "trackers:\n";
//			for (std::vector<announce_entry>::const_iterator i = t.trackers().begin();
//				i != t.trackers().end(); ++i)
//			{
//				std::cout << i->tier << ": " << i->url << "\n";
//			}

//			std::cout << "number of pieces: " << t.num_pieces() << "\n";
//			std::cout << "piece length: " << t.piece_length() << "\n";
//			std::cout << "info hash: " << t.info_hash() << "\n";
//			std::cout << "comment: " << t.comment() << "\n";
//			std::cout << "created by: " << t.creator() << "\n";
//			std::cout << "magnet link: " << make_magnet_uri(t) << "\n";
//			std::cout << "Name: " << t.name() << "\n";
//			std::cout << "files:\n";
		int index = 0;
		torfile_list *list = new torfile_list;
		std::string temp;
//			std::vector<torfile_list> tor_list;
		for (torrent_info::file_iterator i = t.begin_files();
			i != t.end_files(); ++i, ++index)
		{
			int first = t.map_file(index, 0, 1).piece;
			int last = t.map_file(index, i->size - 1, 1).piece;
			list->size = i->size;
			temp = i->path.string();
			list->name = temp.substr(temp.find('/')+1,temp.size());
			list->node_s = first;
			list->node_f = last;
			list->mtime = i->mtime;
			list->symlink_attribute = i->symlink_attribute;
			if(list->symlink_attribute)
				list->symlink_path = i->symlink_path.string();
			tor_list.push_back(*list);
//				std::cout << "  " << std::setw(11) << i->size
//					<< " " << i->path.string() << "[ " << first << ", "
//					<< last << " ]\n";
		}

#ifndef BOOST_NO_EXCEPTIONS
	}
	catch (std::exception& e)
	{
  		std::cout << e.what() << "\n";
	}
#endif

	return tor_list;
}

std::vector<torfile_list> getList(int a, char* b)
{
	tor mytor;
	std::vector<torfile_list> list;
	list = mytor.gettor(a,b);
	for (int i=0; i< list.size(); i++)
	{
		std::cout<<list[i].name<<std::endl;
	}
	return list;
}

char** fileList (const char *a )
{
	tor mytor;
	char **file;
	char *temp = new char[strlen(a)+1];
	strcpy(temp,a);
	strcat(temp,".torrent");
//	printf("%s\n",a);
//	printf("before new char* [10]\n");
//	file = new char* [10];
//	printf("after new char* \n");
	std::vector<torfile_list> list;
	list = mytor.gettor(2,temp);
	file = new char* [list.size()+1];
//	printf("before loop\n");
	int i;
	for(i =0 ;i< list.size(); i++)
	{	
//		printf("Inside loop with i = %d\n",i);
		file[i] = new char[list[i].name.size()+1];
		strcpy(file[i],list[i].name.c_str());
	}
	file[i] = new char[2];
	strcpy(file[i],"\n");
//	strcpy(file[i],"\0");
//	printf("loop finished\n");
	delete[] temp;
	return file;
}

int check_file( const char *file )
{
	std::string fileName(file);
	int first = fileName.find('/');
	int index_t;
	index_t = check(fileName.substr(0,first), fileName.substr(first+1,fileName.size()-first));
	if (index_t < 0)
		return 0;
	return 1;
}

//Returns the index numer of file present in the torrent. i.e. returns negative number if not found else return index of file in the torrent.
int check ( const std::string torrent_name, const std::string path_tmp)
{
	tor mytor;
	std::string torrent_n(torrent_name);
	std::vector<torfile_list> list;
	torrent_n.append(".torrent");
//	int index = path_tmp.find(".torrent");
	std::string path;
//	if (index < path_tmp.size())
//		path = path_tmp.substr(0,index);
//	else
//		path = path_tmp;
	char *temp = (char *)torrent_n.c_str();
	list = mytor.gettor(2,temp);
	
	std::cout<<"File Name :"<<path_tmp<<std::endl;	
	for(int i =0; i< list.size(); i++)
	{
//		std::cout<<"List ["<<i<<"] = "<<list[i].name<<std::endl;

		if(list[i].name == path_tmp)
		{
			return i;
		}
	}
	return -1;
}

int getTorrent(const char* path)
{
	using namespace libtorrent;
#if BOOST_VERSION < 103400
	namespace fs = boost::filesystem;

	fs::path::default_name_check(fs::no_check); 
#endif

#ifndef BOOST_NO_EXCEPTIONS
	try
#endif
	{
		session s;
		torrent_handle handle;
		torrent_status status;
		tor mytor;
		std::string torrent_name(path);
		std::string torrent_file_name;
		std::vector<torfile_list> list;
		int index_t = torrent_name.find("/");
		torrent_file_name = torrent_name.substr(index_t+1,torrent_name.size()-index_t);
		torrent_name = torrent_name.substr(0,index_t);

		//Determining the index of file to be downloaded in the torrent.
		std::cout << "1. Torrent Name :"<<torrent_name<<"\t Torrent File :"<<torrent_file_name<<std::endl;
		index_t = check(torrent_name,torrent_file_name);
		
		//Appending .torrent to last of torrent_name;
		torrent_name.append(".torrent");

		char *temp = (char *)torrent_name.c_str();
		list = mytor.gettor(2,temp);
		std::vector<int> priorities_files;
		for(int i = 0; i<list.size(); i++)
			priorities_files.push_back(0);
		priorities_files[index_t]=1;
		
		if(list[index_t].symlink_attribute)
		{
			printf("Index is %d\n",index_t);
			std::string sym_path(path);
			sym_path += "/" + list[index_t].symlink_path;
			int index_sym = sym_path.find("../");
			while (index_sym != -1)
			{
				std::cout << "Total link is "<<sym_path<<std::endl;
				int indx = sym_path.rfind("/",index_sym-2);
				indx = sym_path.rfind("/",indx-1);
				if (indx == -1)
				{
					
					printf("wrong symlink path\n");
					break;
				}
				sym_path = sym_path.substr(0,indx+1) + sym_path.substr(index_sym+3);
				index_sym = sym_path.find("../");
				std::cout<<"Next resolution is "<<sym_path<<"\n";
			}
			getTorrent(sym_path.c_str());
			
		}
		
		std::cout << "2. Torrent Name :"<<torrent_name<<"\t Torrent File :"<<torrent_file_name<<"\t Index No :"<<index_t<<std::endl;	

		s.listen_on(std::make_pair(6881, 6889));
		add_torrent_params p;
		p.save_path = "./";
		p.ti = new torrent_info(torrent_name.c_str());
		handle = s.add_torrent(p);

		handle.prioritize_files(priorities_files);

		printf("Download started\n");
		
		
/*		while (1)
		{
			std::auto_ptr<alert> a;
			a = s.pop_alert();
			libtorrent::alert* al = a.get();
		
			if (torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>(al))
			{
				std::cout<<"Alert\n"<<std::flush;
			}
		}
*/	//return 1;
		while(1)
		{
			status = handle.status();

			std::cout<<status.state<<"\t"<<std::flush;
			if(status.state == torrent_status::finished)
			{
				std::cout<< "Download Finished\n"<<std::flush;
				break;
//				sleep(2);
			}
			sleep(5);
		}
		
		//Download finished, setting mtime
		
		struct utimbuf timebuf;
		timebuf.actime = list[index_t].mtime;
		timebuf.modtime = list[index_t].mtime;
		
		if( utime(path,&timebuf) < 0 )
		{
			std::cout<<"mtime not set\t Error code is "<<errno<<std::endl;
			if (errno == ENOENT)
				std::cout<<"filename does not exist\n Fileanme is "<<path<<std::endl;
			else if (errno == EACCES)
				std::cout<<"file permission denied\n";
		}
		else
			std::cout<<"mtime set"<<std::endl;

		// wait for the user to end
		//char a1;
		//std::cin.unsetf(std::ios_base::skipws);
		//std::cin >> index_t;
	}
#ifndef BOOST_NO_EXCEPTIONS
	catch (std::exception& e)
	{
  		std::cout << e.what() << "\n";
	}
#endif
	return 0;
}

int tor_list_C (const char* a, torfile_list_C* list_C[])
{
	printf("Inside tor_list_C function \n");
	printf("File is %s\n",a);
	tor mytor;
	char *temp = new char[strlen(a)+8];
	strcpy(temp,a);
	strcat(temp,".torrent");
	std::vector<torfile_list> list;
	list = mytor.gettor(2,temp);
	int i;
	printf("Before loop in tor_list_C function \n");
	printf("List size is %d\n",list.size());
	for(i =0 ;i< list.size(); i++)
	{
		list_C[i] = new torfile_list_C;
		list_C[i]->name = new char[strlen(list[i].name.c_str())+1];
		strcpy(list_C[i]->name,list[i].name.c_str());
		list_C[i]->size = list[i].size;
		list_C[i]->node_s = list[i].node_s;
		list_C[i]->node_f = list[i].node_f;
		list_C[i]->mtime = list[i].mtime;
		if(list[i].symlink_attribute)
		{
			list_C[i]->symlink_attribute = 1;
			list_C[i]->symlink_path = new char[strlen(list[i].symlink_path.c_str())+1];
			strcpy(list_C[i]->symlink_path,list[i].symlink_path.c_str());
		}
		else
			list_C[i]->symlink_attribute = 0;
	}
	printf("After loop in tor_list_C function, return value is %d \n",i);
	return i;
}