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

#ifndef SQL2TEXT_HPP_INCLUDED
#define SQL2TEXT_HPP_INCLUDED

#include <cppdb/frontend.h>
#include <cppdb/utils.h>
#include <iostream>
#include <ctime>

#include "retranse.hpp"

////////////////////////////////////////////////////////////////////////////////////////

namespace sql2text {


////////////////////////////////////////////////////////////////////////////////////////

struct tbl
{
	bool has_prim;
	int fields;
	std::vector<std::string> col;
	std::vector<int> prim;
};

struct handle
{
	cppdb::connection_info& ci;
	cppdb::session& sql;
	retranse::node* nc;
	std::string engine, dbname;
	retranse::var_t v;
	handle(cppdb::connection_info& ci, cppdb::session& sql, retranse::node* nc);
	handle(const handle& o) : ci(o.ci), sql(o.sql), nc(o.nc), engine(o.engine), dbname(o.dbname), v(o.v) {}

	// stage one: helpers
	std::vector<std::string> rt_response_query();
	std::vector<std::string> rt_single_query();
	std::vector<std::string> rt_response_vector();
	bool rt_exec_query();

	////////////////////////////////////////////////////////////////////////////////
	// stage two: main ops /////////////////////////////////////////////////////////

	// check configuration for db engine
	bool check();
	// list all the databases
	std::vector<std::string> ls_root();
	// list all the tables
	std::vector<std::string> ls_db(const std::string& db);
	// list all columns
	std::vector<std::string> ls_tab(const std::string& db, const std::string& tab);
	// list column important attributes
	std::string ls_col_prop(const std::string& db, const std::string& tab, const std::string& col);
	// list column
	std::string ls_col(const std::string& col, const std::string& prop);
	// list table header
	std::string ls_tabh(const std::string& db, const std::string& tab);
	// catalog table contents
	void cat_tab(const std::string& db, const std::string& tab, std::ostream& out);
	// column definition
	std::vector<std::string> def_col(const std::string& col);
	// rename table
	bool mv_tab(const std::string& db, const std::string& tab, const std::string& s);
	// remove database
	bool rm_db(const std::string& db);
	// remove table
	bool rm_tab(const std::string& db, const std::string& tab);
	// remove table data
	bool rm_data(const std::string& db, const std::string& tab);
	// create database
	bool mk_db(const std::string& db);
	// create table
	bool mk_tab(const std::string& db, const std::string& tab, const std::string& head);

	//// row ops

	// remove 1 row from table
	bool rm_tab_row(const tbl& info, const std::string& db, const std::string& tab, const std::string& row);
	// insert 1 row to table
	bool add_tab_row(const tbl& info, const std::string& db, const std::string& tab, const std::string& row);
	// update row in table
	bool mv_tab_row(const tbl& info, const std::string& db, const std::string& tab, const std::string& prev, const std::string& next);


	////////////////////////////////////////////////////////////////////////////////
	// stage three: programmatically used ops //////////////////////////////////////

	// list table header, get info
	std::string info_tab(tbl& info, const std::string& db, const std::string& tab);
	// update row on table, using sql update. table must have primary key
	bool update_tab_row_imp(const tbl& info, const std::string& db, const std::string& tab, const std::string& prev, const std::string& next);

};

////////////////////////////////////////////////////////////////////////////////////////

//
namespace dbtxtio
{
	template<class T, class S> inline
	std::ostream& operator <<(std::ostream& o, const std::pair<T, S> & x)
	{
		o << "(" << x.first << ", " << x.second << ")";
		return o;
	}
	template<class T> inline
	std::ostream& operator <<(std::ostream& o, const std::vector<T> & x)
	{
		for(size_t i=0;i<x.size();i++) o << x[i] << "\t";
		return o;
	}
}
//

////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::pair<bool, std::string> > decompose(const std::string& s);


} // namespace sql2text

////////////////////////////////////////////////////////////////////////////////////////

#endif // INCLUDE GUARD


