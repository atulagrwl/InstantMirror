/*

Copyright (c) 2008, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <fstream>
#include <sstream>

#include "libtorrent/session.hpp"
#include "libtorrent/hasher.hpp"

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include "test.hpp"
#include "libtorrent/assert.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/create_torrent.hpp"

using boost::filesystem::remove_all;
using boost::filesystem::create_directory;
using namespace libtorrent;
namespace sf = boost::filesystem;

bool tests_failure = false;

void report_failure(char const* err, char const* file, int line)
{
	std::cerr << "\033[31m" << file << ":" << line << " \"" << err << "\"\033[0m\n";
	tests_failure = true;
}

bool print_alerts(libtorrent::session& ses, char const* name
	, bool allow_disconnects, bool allow_no_torrents, bool allow_failed_fastresume
	, bool (*predicate)(libtorrent::alert*))
{
	bool ret = false;
	std::vector<torrent_handle> handles = ses.get_torrents();
	TEST_CHECK(!handles.empty() || allow_no_torrents);
	torrent_handle h;
	if (!handles.empty()) h = handles[0];
	std::auto_ptr<alert> a;
	a = ses.pop_alert();
	while (a.get())
	{
		if (predicate && predicate(a.get())) ret = true;
		if (peer_disconnected_alert* p = dynamic_cast<peer_disconnected_alert*>(a.get()))
		{
			std::cerr << name << "(" << p->ip << "): " << p->message() << "\n";
		}
		else if (a->message() != "block downloading"
			&& a->message() != "block finished"
			&& a->message() != "piece finished")
		{
			std::cerr << name << ": " << a->message() << "\n";
		}
		TEST_CHECK(dynamic_cast<fastresume_rejected_alert*>(a.get()) == 0 || allow_failed_fastresume);

		TEST_CHECK(dynamic_cast<peer_error_alert*>(a.get()) == 0
			|| (!handles.empty() && h.is_seed())
			|| a->message() == "connecting to peer"
			|| a->message() == "closing connection to ourself"
			|| a->message() == "duplicate connection"
			|| a->message() == "duplicate peer-id, connection closed"
			|| (allow_disconnects && a->message() == "Broken pipe")
			|| (allow_disconnects && a->message() == "Connection reset by peer")
			|| (allow_disconnects && a->message() == "End of file."));
		a = ses.pop_alert();
	}
	return ret;
}

void test_sleep(int millisec)
{
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	boost::uint64_t nanosec = (millisec % 1000) * 1000000 + xt.nsec;
	int sec = millisec / 1000;
	if (nanosec > 1000000000)
	{
		nanosec -= 1000000000;
		sec++;
	}
	xt.nsec = nanosec;
	xt.sec += sec;
	boost::thread::sleep(xt);
}

void stop_web_server(int port)
{
	std::stringstream cmd;
	cmd << "kill `cat ./lighty" << port << ".pid` >/dev/null";
	system(cmd.str().c_str());
}

void start_web_server(int port, bool ssl)
{
	stop_web_server(port);

	if (ssl)
	{
		system("echo . > tmp");
		system("echo test province >>tmp");
		system("echo test city >> tmp");
		system("echo test company >> tmp");
		system("echo test department >> tmp");
		system("echo tester >> tmp");
		system("echo test@test.com >> tmp");	
		system("openssl req -new -x509 -keyout server.pem -out server.pem "
			"-days 365 -nodes <tmp");
	}
	
	std::ofstream f("lighty_config");
	f << "server.modules = (\"mod_access\", \"mod_redirect\", \"mod_setenv\")\n"
		"server.document-root = \"" << fs::initial_path<fs::path>().string() << "\"\n"
		"server.range-requests = \"enable\"\n"
		"server.port = " << port << "\n"
		"server.pid-file = \"./lighty" << port << ".pid\"\n"
		"url.redirect = ("
			"\"^/redirect$\" => \"" << (ssl?"https":"http") << "://127.0.0.1:" << port << "/test_file\""
			", \"^/infinite_redirect$\" => \"" << (ssl?"https":"http") << "://127.0.0.1:" << port << "/infinite_redirect\""
			", \"^/relative/redirect$\" => \"../test_file\""
			")\n"
		"$HTTP[\"url\"] == \"/test_file.gz\" {\n"
		"    setenv.add-response-header = ( \"Content-Encoding\" => \"gzip\" )\n"
		"#    mimetype.assign = ()\n"
		"}\n";
	// this requires lighttpd to be built with ssl support.
	// The port distribution for mac is not built with ssl
	// support by default.
	if (ssl)
		f << "ssl.engine = \"enable\"\n"
			"ssl.pemfile = \"server.pem\"\n";
	f.close();
	
	system("lighttpd -f lighty_config 2> lighty.err >lighty.log &");
	test_sleep(1000);
}

void stop_proxy(int port)
{
	std::stringstream cmd;
	cmd << "delegated -P" << port << " -Fkill";
	system(cmd.str().c_str());
}

void start_proxy(int port, int proxy_type)
{
	using namespace libtorrent;

	stop_proxy(port);
	std::stringstream cmd;
	// we need to echo n since dg will ask us to configure it
	cmd << "echo n | delegated -P" << port << " ADMIN=test@test.com "
		"PERMIT=\"*:*:localhost\" REMITTABLE=+,https RELAY=proxy,delegate";
	switch (proxy_type)
	{
		case proxy_settings::socks4:
			cmd << " SERVER=socks4";
			break;
		case proxy_settings::socks5:
			cmd << " SERVER=socks5";
			break;
		case proxy_settings::socks5_pw:
			cmd << " SERVER=socks5 AUTHORIZER=-list{testuser:testpass}";
			break;
		case proxy_settings::http:
			cmd << " SERVER=http";
			break;
		case proxy_settings::http_pw:
			cmd << " SERVER=http AUTHORIZER=-list{testuser:testpass}";
			break;
	}
	system(cmd.str().c_str());
	test_sleep(1000);
}

using namespace libtorrent;

template <class T>
boost::intrusive_ptr<T> clone_ptr(boost::intrusive_ptr<T> const& ptr)
{
	return boost::intrusive_ptr<T>(new T(*ptr));
}

boost::intrusive_ptr<torrent_info> create_torrent(std::ostream* file, int piece_size, int num_pieces)
{
	char const* tracker_url = "http://non-existent-name.com/announce";
	// excercise the path when encountering invalid urls
	char const* invalid_tracker_url = "http:";
	char const* invalid_tracker_protocol = "foo://non/existent-name.com/announce";
	
	using namespace boost::filesystem;

	file_storage fs;
	int total_size = piece_size * num_pieces;
	fs.add_file(path("temporary"), total_size);
	libtorrent::create_torrent t(fs, piece_size);
	t.add_tracker(tracker_url);
	t.add_tracker(invalid_tracker_url);
	t.add_tracker(invalid_tracker_protocol);

	std::vector<char> piece(piece_size);
	for (int i = 0; i < int(piece.size()); ++i)
		piece[i] = (i % 26) + 'A';
	
	// calculate the hash for all pieces
	int num = t.num_pieces();
	sha1_hash ph = hasher(&piece[0], piece.size()).final();
	for (int i = 0; i < num; ++i)
		t.set_hash(i, ph);

	if (file)
	{
		while (total_size > 0)
		{
			file->write(&piece[0], (std::min)(int(piece.size()), total_size));
			total_size -= piece.size();
		}
	}
	
	std::vector<char> tmp;
	std::back_insert_iterator<std::vector<char> > out(tmp);
	bencode(out, t.generate());
	return boost::intrusive_ptr<torrent_info>(new torrent_info(&tmp[0], tmp.size()));
}

boost::tuple<torrent_handle, torrent_handle, torrent_handle>
setup_transfer(session* ses1, session* ses2, session* ses3
	, bool clear_files, bool use_metadata_transfer, bool connect_peers
	, std::string suffix, int piece_size
	, boost::intrusive_ptr<torrent_info>* torrent, bool super_seeding
	, add_torrent_params const* p)
{
	using namespace boost::filesystem;

	assert(ses1);
	assert(ses2);

	session_settings sess_set;
	sess_set.allow_multiple_connections_per_ip = true;
	sess_set.ignore_limits_on_local_network = false;
	ses1->set_settings(sess_set);
	ses2->set_settings(sess_set);
	if (ses3) ses3->set_settings(sess_set);
	ses1->set_alert_mask(~alert::progress_notification);
	ses2->set_alert_mask(~alert::progress_notification);
	if (ses3) ses3->set_alert_mask(~alert::progress_notification);

	std::srand(time(0));
	peer_id pid;
	std::generate(&pid[0], &pid[0] + 20, std::rand);
	ses1->set_peer_id(pid);
	std::generate(&pid[0], &pid[0] + 20, std::rand);
	ses2->set_peer_id(pid);
	assert(ses1->id() != ses2->id());
	if (ses3)
	{
		std::generate(&pid[0], &pid[0] + 20, std::rand);
		ses3->set_peer_id(pid);
		assert(ses3->id() != ses2->id());
	}

	boost::intrusive_ptr<torrent_info> t;
	if (torrent == 0)
	{
		create_directory("./tmp1" + suffix);
		std::ofstream file(("./tmp1" + suffix + "/temporary").c_str());
		t = ::create_torrent(&file, piece_size, 19);
		file.close();
		if (clear_files)
		{
			remove_all("./tmp2" + suffix + "/temporary");
			remove_all("./tmp3" + suffix + "/temporary");
		}
		char ih_hex[41];
		to_hex((char const*)&t->info_hash()[0], 20, ih_hex);
		std::cerr << "generated torrent: " << ih_hex << " ./tmp1" << suffix << "/temporary" << std::endl;
	}
	else
	{
		t = *torrent;
	}

	// they should not use the same save dir, because the
	// file pool will complain if two torrents are trying to
	// use the same files
	sha1_hash info_hash = t->info_hash();
	add_torrent_params param;
	if (p) param = *p;
	param.ti = clone_ptr(t);
	param.save_path = "./tmp1" + suffix;
	torrent_handle tor1 = ses1->add_torrent(param);
	tor1.super_seeding(super_seeding);
	TEST_CHECK(!ses1->get_torrents().empty());
	torrent_handle tor2;
	torrent_handle tor3;

	// the downloader cannot use seed_mode
	param.seed_mode = false;

	if (ses3)
	{
		param.ti = clone_ptr(t);
		param.save_path = "./tmp3" + suffix;
		tor3 = ses3->add_torrent(param);
		TEST_CHECK(!ses3->get_torrents().empty());
	}

  	if (use_metadata_transfer)
	{
		param.ti = 0;
		param.info_hash = t->info_hash();
	}
	else
	{
		param.ti = clone_ptr(t);
	}
	param.save_path = "./tmp2" + suffix;

	tor2 = ses2->add_torrent(param);
	TEST_CHECK(!ses2->get_torrents().empty());

	assert(ses1->get_torrents().size() == 1);
	assert(ses2->get_torrents().size() == 1);

	test_sleep(100);

	if (connect_peers)
	{
		std::cerr << "connecting peer\n";
		error_code ec;
		tor1.connect_peer(tcp::endpoint(address::from_string("127.0.0.1", ec)
			, ses2->listen_port()));

		if (ses3)
		{
			// give the other peers some time to get an initial
			// set of pieces before they start sharing with each-other
			tor3.connect_peer(tcp::endpoint(
				address::from_string("127.0.0.1", ec)
				, ses2->listen_port()));
			tor3.connect_peer(tcp::endpoint(
				address::from_string("127.0.0.1", ec)
				, ses1->listen_port()));
		}
	}

	return boost::make_tuple(tor1, tor2, tor3);
}

