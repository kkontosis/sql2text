/*  libsql2text, a library for converting database tables to/from text files
 *  Copyright (C) 2013, Kimon Kontosis
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  version 2.1 along with this program (see LICENSE.LESSER); if not, see 
 *  <http://www.gnu.org/licenses/>.
 *
*/

#include "sql2text.hpp"
#include "bindir.hpp"
#include <fstream>
#include <unistd.h>
#include <limits.h>

////////////////////////////////////////////////////////////////////////////////////////

static inline bool file_exists(const char*s)
{
	std::ifstream my_file(s);
	return (my_file.good());
}

////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char**argv)
{
	using namespace sql2text;
	using namespace sql2text::dbtxtio;

	if(argc<3) { 
		std::cout 	<<     "Usage: " << argv[0] << " <connection-string> <command> [arguments]"<<std::endl; 
		std::cout 	<<     "List of available commands:" << std::endl;
		std::cout 	<<     "ls				List databases" << std::endl;
		std::cout 	<<     "ls <dbname>			List tables in database <dbname>" << std::endl;
		std::cout 	<<     "ls <dbname> <table>		List table <table>'s contents in database <db>" << std::endl;
		std::cout 	<<     "lsh <dbname> <table>		List table's <table> header in database <db>" << std::endl;
		std::cout 	<<     "mv <dbname> <table> <new>	Rename table <table> to <new> in database <db>" << std::endl;
		std::cout 	<<     "mv <dbname> <table> <old> <new>	Update line <old> to <new> in <table> in <db>" << std::endl;
		std::cout 	<<     "rm <dbname> 			Remove database <db>" << std::endl;
		std::cout 	<<     "rm <dbname> <table> 		Remove table <table> from database <db>" << std::endl;
		std::cout 	<<     "rm <dbname> <table> <x>		Remove 1 line <x> from <table> in database <db>" << std::endl;
		std::cout 	<<     "rmd <dbname> <table> 		Remove data from table <table> in database <db>" << std::endl;
		std::cout 	<<     "mk <dbname>	 		Create database <db>" << std::endl;
		std::cout 	<<     "mk <dbname> <table> <head>	Create <table> in <db> from description <head>" << std::endl;
		std::cout 	<<     "add <dbname> <table> <new>	Insert line <x> into <table> in database <db>" << std::endl;
		std::cout 	<<     "" << std::endl;
		std::cout 	<<     "def <col>			Find column desc <col> definition in sql" << std::endl;
		std::cout 	<<     "deco <row>			Find actual values from single plain text row" << std::endl;
		//std::cout << "lscol <db> <table>	List tables <table> columns in database <db>" << std::endl;
		std::cout	<<	std::endl;
		std::cout	<<	"the <connection-string> of the database consists of pairs of the type:" << std::endl;
		std::cout	<<	"	key=value" << std::endl;
		std::cout	<<	"separated by semicolon (;) following the database type name and a colon (:)" << std::endl;
		std::cout	<<	"for example:" << std::endl;
		std::cout	<<	"	mysql:user=username;password=hackme" << std::endl;
		std::cout	<<	"see also the documentation of `cppcms` for connection string information" << std::endl;
		std::cout	<<	std::endl;

		return 0; 
	}

	std::string bindir = get_bin_dir(argv[0]);
	const char* default_cfg = "/etc/sql2text/config.ret";
	std::string cfg_file = default_cfg;
	if(!file_exists(default_cfg)) {
		cfg_file = bindir + default_cfg;
		if(!file_exists(cfg_file.c_str())) {
			std::cerr << "cannot find configuration file " << default_cfg << std::endl;
			return 1;
		}
	}
	
	std::string cfg_dir = get_file_dir(cfg_file.c_str());

	// save current directory
	char cur_dir[PATH_MAX];
	char *curdir=getcwd(cur_dir, PATH_MAX);
	if(chdir(cfg_dir.c_str())) 
		{ std::cerr << "error: cannot change directory to " << cfg_dir << std::endl; return 1; }

	retranse::node* nc = retranse::compile("config.ret");
	// restore current directory
	if(chdir(curdir))
		{ std::cerr << "error: cannot change directory to " << curdir << std::endl; return 1; }

	if(!nc) {
		std::cerr << "error in configuration file" << cfg_file << std::endl;
		return 1;
	}
	try {
		// init
		cppdb::connection_info ci(argv[1]);
		std::string modules_path = "/usr/lib:/usr/local/lib:/usr/lib/sql2text/shared:/usr/local/lib/sql2text/shared";
		if(bindir.size()) { modules_path += ":"; modules_path += bindir + "/lib/sql2text/shared"; }

		if(!ci.has("@modules_path")) ci.properties["@modules_path"] = modules_path;

		cppdb::session sql(ci);
		
		handle x(ci, sql, nc);
		x.check();

		// con
		
		//cppdb::statement stat;

		//std::cout << "connected. database name = " << x.dbname << std::endl;	

		if(std::string(argv[2])=="ls") {

			if(argc == 3) {
				std::cout << x.ls_root() << std::endl;
			}
			else
			if(argc == 4) {
				std::cout << x.ls_db(argv[3]) << std::endl;
			}
			if(argc == 5) {	

				std::cout << x.ls_tabh(argv[3], argv[4]) << std::endl;

				x.cat_tab(argv[3], argv[4], std::cout);
			}
			//else

	

		} else
		if(std::string(argv[2])=="lsh") {

			if(argc == 5) {		
				std::cout << x.ls_tabh(argv[3], argv[4]) << std::endl;

			}
			//else

	

		} else
		if(std::string(argv[2])=="def") {
			if(argc == 4) {
				std::cout << x.def_col(argv[3]) << std::endl;
			}
	

		} else
		if(std::string(argv[2])=="mv") {
			if(argc == 6) {
				std::cout << x.mv_tab(argv[3], argv[4], argv[5]) << std::endl;
			} else
			if(argc == 7) {			
				tbl info;
				x.info_tab(info, argv[3], argv[4]);				
				std::cout << x.mv_tab_row(info, argv[3], argv[4], argv[5], argv[6]) << std::endl;
			}
	

		} else
		if(std::string(argv[2])=="rm") {
			if(argc == 4) {
				std::cout << x.rm_db(argv[3]) << std::endl;
			} else
			if(argc == 5) {
				std::cout << x.rm_tab(argv[3], argv[4]) << std::endl;
			}
			if(argc == 6) {
				tbl info;
				x.info_tab(info, argv[3], argv[4]);				
				std::cout << x.rm_tab_row(info, argv[3], argv[4], argv[5]) << std::endl;
			}	

		} else
		if(std::string(argv[2])=="rmd") {
			if(argc == 5) {
				std::cout << x.rm_data(argv[3], argv[4]) << std::endl;
			}
	

		} else
		if(std::string(argv[2])=="mk") {
			if(argc == 4) {
				std::cout << x.mk_db(argv[3]) << std::endl;
			} else
			if(argc == 6) {
				std::cout << x.mk_tab(argv[3], argv[4], argv[5]) << std::endl;
			}

		} else
		if(std::string(argv[2])=="add") {
			if(argc == 6) {
				tbl info;
				x.info_tab(info, argv[3], argv[4]);				
				std::cout << x.add_tab_row(info, argv[3], argv[4], argv[5]) << std::endl;
			}

		}
		else
		if(std::string(argv[2])=="deco") {
			if(argc == 4) {
				std::vector<std::pair<bool, std::string> > r = decompose(argv[3]);
				std::cout << r << std::endl;
			}

		}

	}
	catch(retranse::rtex const &x) {
		std::cerr << "CONFIGURATION ERROR: " << x.what() << std::endl;
		return 1;
	}
	catch(std::exception const &e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}


