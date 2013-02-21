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

#ifndef SQL2TEXT_EXPLODE_INCLUDED
#define SQL2TEXT_EXPLODE_INCLUDED

#include <vector>
#include <string>
#include <utility>

static std::vector<std::string> explode(const std::string &delimiter, 
	const std::string &src)
{
    using namespace std;
    vector<std::string> result;

    int sl = src.length();
    int dl = delimiter.length();
    if(!dl) return result; // empty vector

    int i=0;
    int k=0;
    while(i < sl) {
        int j=0;
        while ((i+j < sl) && (j < dl) && (src[i+j] == delimiter[j]))
            j++;

        if(j==dl) {
            result.push_back(src.substr(k, i-k));
            i+=dl;
            k=i;
        }
        else {
            i++;
        }
    }
    result.push_back(src.substr(k, i-k));
    return result;
}

#endif // include guard

