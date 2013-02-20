/*  libsql2text, a library for converting database tables to/from text files
 *  Copyright (C) 2012, Kimon Kontosis
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
#include "explode.hpp"

////////////////////////////////////////////////////////////////////////////////////////

#define s_t handle

namespace sql2text {

std::string dencode(const std::string& s, bool& isnull, bool& invalid)
{
	isnull = 0;
	invalid = 0;
	std::string r;
	if(s=="\\N" || (s.size()>2 && s[0]=='\\' && s[1]=='N')) 
		{ isnull = 1; return r; }
	for(int i = 0; i < s.size(); i++) {
		if(s[i]!='\\') { r.push_back(s[i]); continue; }
		i++;
		if(i >= s.size()) { invalid = true; return r; }
		switch(s[i]) {
			case 't':
			r.push_back('\t');
			break;
			case 'r':
			r.push_back('\r');
			break;
			case 'n':
			r.push_back('\n');
			break;
			case '{':
			{
				size_t j = s.find('}', i);
				if(j==std::string::npos) { invalid=true; return r; }
				std::string x = s.substr(i+1, j-i-1);
				std::istringstream is(x);
				int cc;
				is >> cc;
				r.push_back(cc);
				i=j+1;
			}
			default:
			r.push_back(s[i]);
		}
	}
	return r;
}

std::string encode(const std::string& s, bool isnull)
{
	if(isnull) return "\\N";
	std::string r;
	for(int i = 0; i < s.size(); i++) {
		if(s[i] == '\t') { r.push_back('\\'); r.push_back('t'); continue; }
		if(s[i] == '\r') { r.push_back('\\'); r.push_back('r'); continue; }
		if(s[i] == '\n') { r.push_back('\\'); r.push_back('n'); continue; }
		if(s[i] < '\n') {
			r.push_back('\\'); r.push_back('{');
			std::ostringstream os;
			os << int(s[i]);
			r+=os.str();
			r.push_back('}'); 
		}
		r.push_back(s[i]);
	}
	return r;
}

std::vector<std::pair<bool, std::string> > decompose(const std::string& s)
{
	size_t l = 0;
	bool n, v;
	std::vector<std::pair<bool, std::string> > rx;
	std::string z;
	for(size_t i = 0; i < s.size()+1; i++) if(i==s.size() || s[i]=='\r' || s[i]=='\n' || s[i]=='\t') {
		z = s.substr(l, i-l);
		l = i+1;
		z = dencode(z, n, v);
		if(v) return std::vector<std::pair<bool, std::string> >();
		rx.push_back(std::pair<bool, std::string>(!n, z));
	}
	return rx;
}

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////

s_t::s_t(cppdb::connection_info& ci, cppdb::session& sql, retranse::node* nc) 
	: ci(ci), sql(sql), nc(nc)
{
	engine = sql.engine();
	std::vector<std::string> tst = explode(":",retranse::run(nc, "dbname", sql.engine()));
	for(size_t i=0;i<tst.size();i++) if(ci.has(tst[i])) dbname = ci.properties[tst[i]];
}

std::vector<std::string> s_t::rt_response_query()
{
	cppdb::statement s = sql << v["1"];
	
	std::string x = "2";
	int n = 2;
	while(v[x].size()) {
		s << v[x];
		std::ostringstream z;
		z << ++n;
		x = z.str();
	}
	cppdb::result res = s;

	std::vector<std::string> r;
	while(res.next()) {
		std::string s2;
		res >> s2;
		r.push_back(s2);
	}
	return r;
}

bool s_t::rt_exec_query()
{
	cppdb::statement s = sql << v["1"];

	std::string x = "2";
	int n = 2;
	while(v[x].size()) {
		s << v[x];
		std::ostringstream z;
		z << ++n;
		x = z.str();
	}
	s.exec();
	return true;
}

std::vector<std::string> s_t::rt_response_vector()
{
	std::vector<std::string> r;
	std::string x = "1";
	int n = 1;
	while(v[x].size()) {
		r.push_back(v[x]);
		std::ostringstream z;
		z << ++n;
		x = z.str();
	}
	return r;
}

std::vector<std::string> s_t::rt_single_query()
{
	cppdb::statement s = sql << v["1"];
	
	std::string x = "2";
	int n = 2;
	while(v[x].size()) {
		s << v[x];
		std::ostringstream z;
		z << ++n;
		x = z.str();
	}
	cppdb::result res = s;
	std::vector<std::string> r;

	if(!res.next()) return r;
	for(int i = res.cols(); i; i--)
	{
		std::string s2;
		res >> s2;
		r.push_back(s2);
	}
	return r;
}

////////////////////////////////////////////////////////////////////////////////////////

// stage two: main ops

// list all the databases
bool s_t::check()
{
	retranse::run(nc, "check", engine, v);
	if(v["1"].size() == 0) return true;
	throw retranse::rtex(v["1"]);
	return false;
}

// list all the databases
std::vector<std::string> s_t::ls_root()
{
	retranse::run(nc, "ls_root", engine, v);
	return rt_response_query();
}

// list all the tables
std::vector<std::string> s_t::ls_db(const std::string& db)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	retranse::run(nc, "ls_db", a, v);
	return rt_response_query();
}

// list all columns
std::vector<std::string> s_t::ls_tab(const std::string& db, const std::string& tab)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	a.push_back(tab);
	retranse::run(nc, "ls_tab", a, v);
	return rt_response_query();
}

// list column important attributes (engine specific)
std::string s_t::ls_col_prop(const std::string& db, const std::string& tab, const std::string& col)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	a.push_back(tab);
	a.push_back(col);
	retranse::run(nc, "q_ls_col_prop", a, v);
	a = rt_single_query();
	a.insert(a.begin(), engine);
	retranse::run(nc, "a_ls_col_prop", a, v);
	return v["1"];
}

// list column
std::string s_t::ls_col(const std::string& col, const std::string& prop)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(col);
	a.push_back(prop);
	retranse::run(nc, "ls_col", a, v);
	return v["1"];
}

// list table header
std::string s_t::ls_tabh(const std::string& db, const std::string& tab)
{
	std::vector<std::string> z = ls_tab(db, tab);
	std::ostringstream os;
	for(size_t i = 0; i<z.size(); i++) {
		os<< ls_col(z[i], ls_col_prop(db, tab, z[i]));
		if(i<z.size()-1) os <<"\t";
	}
	//os << "\\" << std::endl;
	return os.str();
}

// catalog table contents
void s_t::cat_tab(const std::string& db, const std::string& tab, std::ostream& out)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	a.push_back(tab);
	std::string q = retranse::run(nc, "q_cat_tab", a, v);
	//std::cout << "query = " << q << std::endl;
	cppdb::result res = sql << q;

	while(res.next()) {
		std::string s;
		for(int i = 0 ; i < res.cols() ; i++)
		{
			s = "";
			bool nl=res.fetch(s);
			out << encode(s, !nl);
			if(i<res.cols()-1) out << "\t";
		}
		out << std::endl;
	}
}

// column definition
std::vector<std::string> s_t::def_col(const std::string& col)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(col);
	retranse::run(nc, "def_col", a, v);
	return rt_response_vector();
}

// rename table
bool s_t::mv_tab(const std::string& db, const std::string& tab, const std::string& n)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	a.push_back(tab);
	a.push_back(n);
	retranse::run(nc, "mv_tab", a, v);
	return rt_exec_query();
}

// remove database
bool s_t::rm_db(const std::string& db)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	retranse::run(nc, "rm_db", a, v);
	return rt_exec_query();
}

// remove table
bool s_t::rm_tab(const std::string& db, const std::string& tab)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	a.push_back(tab);
	retranse::run(nc, "rm_tab", a, v);
	return rt_exec_query();
}

// remove 1 row from table
// if table has primary key, it is used. 
// else a database-dependant `rm_tab_1row` implementation is used.
bool s_t::rm_tab_row(const tbl& info, const std::string& db, const std::string& tab, const std::string& row)
{
	std::vector<std::pair<bool, std::string> > ds = decompose(row);
	if(!ds.size()) return false;

	std::string where;
	for(int i=0; i<info.fields; i++)
		if(info.prim[i] || !info.has_prim) where += std::string(where.size()?" AND ":"") 
			+ "`"+info.col[i]+"`" + (ds[i].first?"=?":" IS NULL");

	if(!info.has_prim) {
		std::vector<std::string> a;
		a.push_back(engine);
		a.push_back(db);
		a.push_back(tab);
		a.push_back(where);
		retranse::run(nc, "rm_tab_1row", a, v);
	} else {
		v["1"]=std::string("DELETE FROM `"+db+"`.`"+tab+"` WHERE ( "+where+" )");
	}
	cppdb::statement s = sql << v["1"];
	
	for(int i=0; i<info.fields; i++)
		if(info.prim[i] || !info.has_prim)
			if(ds[i].first) s << ds[i].second;

	s.exec();
		
	return true;
}

// insert 1 row to table
bool s_t::add_tab_row(const tbl& info, const std::string& db, const std::string& tab, const std::string& row)
{
	std::string cols, marks;
	for(int i=0; i<info.fields; i++) {
		cols += std::string(cols.size() ? ", ":"") + "`"+info.col[i]+"`";
		marks += marks.size() ? ", ?" : "?";
	}
	std::string q = "INSERT INTO `"+db+"`.`"+tab+"` ( " + cols + " ) VALUES ( " + marks + " ) "; 
	
	cppdb::statement s = sql << q;
	std::vector<std::pair<bool, std::string> > ds = decompose(row);
	if(!ds.size()) return false;
	
	for(int i=0; i<info.fields; i++)
		if(!ds[i].first) s.bind_null(); else s << ds[i].second;
	s.exec();
		
	return true;
}

// update row on table. table must have primary key
bool s_t::update_tab_row_imp(const tbl& info, const std::string& db, const std::string& tab, const std::string& prev, const std::string& next)
{
	std::vector<std::pair<bool, std::string> >  dsp = decompose(prev);
	std::vector<std::pair<bool, std::string> >  dsn = decompose(next);
	if(!dsp.size() || !dsn.size()) return false;
	std::vector<int> diff(dsn.size(), 0);
	int diffs=0;
	
	std::string where;
	for(int i=0; i<info.fields; i++)
		if(info.prim[i] || !info.has_prim) where += std::string(where.size()?" AND ":"") 
			+ "`"+info.col[i]+"`" + (dsp[i].first?"=?":" IS NULL");
	
	std::string cols;
	for(int i=0; i<info.fields; i++) if(dsn[i]!=dsp[i]) {
		cols += std::string(cols.size() ? ", ":"SET ") + "`"+info.col[i]+"`=?";
		diff[i]=1;
		diffs++;
	}
	if(!diffs) return true;
	
	std::string q = "UPDATE `"+db+"`.`"+tab+"` " + cols + " WHERE ( " + where + " ) "; 

	cppdb::statement s = sql << q;
	
	for(int i=0; i<info.fields; i++) if(diff[i])
		if(!dsn[i].first) s.bind_null(); else s << dsn[i].second;
		
	for(int i=0; i<info.fields; i++)
		if(info.prim[i] || !info.has_prim) 
			if(dsp[i].first) s << dsp[i].second;
	
	s.exec();
		
	return true;
}

// update row in table
// (either by update_tab_row_imp or using rm and add)
bool s_t::mv_tab_row(const tbl& info, const std::string& db, const std::string& tab, const std::string& prev, const std::string& next)
{
	if(info.has_prim)
		return update_tab_row_imp(info, db, tab, prev, next);
	else
		return rm_tab_row(info, db, tab, prev) && add_tab_row(info, db, tab, next);
}

// remove table data
bool s_t::rm_data(const std::string& db, const std::string& tab)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	a.push_back(tab);
	retranse::run(nc, "rm_data", a, v);
	return rt_exec_query();
}

// create database
bool s_t::mk_db(const std::string& db)
{
	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	retranse::run(nc, "mk_db", a, v);
	return rt_exec_query();
}

// create table
bool s_t::mk_tab(const std::string& db, const std::string& tab, const std::string& head)
{
	std::vector<std::pair<bool, std::string> > rx = decompose(head);
	if(!rx.size()) return false;
	std::vector<std::string> rd;

	std::string rs;
	std::string pk;
	
	for(size_t i = 0; i < rx.size(); i++) {
		if(!rx[i].first) return false;
		rd = def_col(rx[i].second);
		rs += std::string(rs.size() ? ", ":"") + rd[0];
		if(rd.size()>1) 
		pk += std::string(pk.size() ? ", ":"") + rd[1];
	}

	std::vector<std::string> a;
	a.push_back(engine);
	a.push_back(db);
	a.push_back(tab);
	a.push_back(rs);
	a.push_back(pk);
		
	retranse::run(nc, "mk_tab", a, v);
	return rt_exec_query();
}

////////////////////////////////////////////////////////////////////////////////////////

// list table header, get info
std::string s_t::info_tab(tbl& info, const std::string& db, const std::string& tab)
{
	std::vector<std::string> z = ls_tab(db, tab);
	std::ostringstream os;
	info.fields = z.size();
	info.has_prim = false;
	info.col = z;
	info.prim.resize(z.size(), 0);
	for(size_t i = 0; i<z.size(); i++) {
		std::string cp = ls_col_prop(db, tab, z[i]);
		info.prim[i] = cp[1]=='1';
		info.has_prim |= info.prim[i];
		os<< ls_col(z[i], cp);
		if(i<z.size()-1) os <<"\t";
	}
	//os << "\\" << std::endl;
	return os.str();	
}


} // namespace sql2text

