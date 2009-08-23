#include <iostream>

using namespace std;

libtorrent::torrent_info getTorrentInfo (string torrentFilePath)
{
	using namespace libtorrent;
	using namespace boost::filesystem;
    
	int size = file_size(torrentFilePath.c_str());
	if (size > 10 * 1000000)
	{
		std::cerr << "file too big (" << size << "), aborting\n";
		return 1;
	}
	std::vector<char> buf(size);
	std::ifstream(torrentFilePath.c_str(), std::ios_base::binary).read(&buf[0], size);
	lazy_entry e;
	int ret = lazy_bdecode(&buf[0], &buf[0] + buf.size(), e);

	if (ret != 0)
	{
		std::cerr << "invalid bencoding: " << ret << std::endl;
		return 1;
	}

	error_code ec;
	torrent_info t(e, ec);
	if (ec)
	{
		std::cout << ec.message() << std::endl;
		return 1;
	}

	// print info about torrent
	std::cout << "\n\n----- torrent file info -----\n\n";
	std::cout << "nodes:\n";
	typedef std::vector<std::pair<std::string, int> > node_vec;
	node_vec const& nodes = t.nodes();
	for (node_vec::const_iterator i = nodes.begin(), end(nodes.end());
		i != end; ++i)
	{
		std::cout << i->first << ":" << i->second << "\n";
	}
	std::cout << "trackers:\n";
	for (std::vector<announce_entry>::const_iterator i = t.trackers().begin();
		i != t.trackers().end(); ++i)
	{
		std::cout << i->tier << ": " << i->url << "\n";
	}

	std::cout << "number of pieces: " << t.num_pieces() << "\n";
	std::cout << "piece length: " << t.piece_length() << "\n";
	char ih[41];
	to_hex((char const*)&t.info_hash()[0], 20, ih);
	std::cout << "info hash: " << ih << "\n";
	std::cout << "comment: " << t.comment() << "\n";
	std::cout << "created by: " << t.creator() << "\n";
	std::cout << "magnet link: " << make_magnet_uri(t) << "\n";
	std::cout << "name: " << t.name() << "\n";
	std::cout << "files:\n";
	int index = 0;
	for (torrent_info::file_iterator i = t.begin_files();
		i != t.end_files(); ++i, ++index)
	{
		int first = t.map_file(index, 0, 1).piece;
		int last = t.map_file(index, i->size - 1, 1).piece;
		std::cout << "  " << std::setw(11) << i->size
			<< " "
			<< (i->pad_file?'p':'-')
			<< (i->executable_attribute?'x':'-')
			<< (i->hidden_attribute?'h':'-')
			<< (i->symlink_attribute?'l':'-')
			<< " "
			<< "[ " << std::setw(3) << first << ", " << std::setw(3) << last << " ]\t"
			<< i->path.string() ;
		if (i->symlink_attribute)
			std::cout << " -> " << i->symlink_path.string();
		std::cout << std::endl;
	}
	return t;
}