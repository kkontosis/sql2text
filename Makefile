# parameters

# installation dirs, for binaries and configuration files
INSTPATH=/usr/local/
ETCPATH=/

CFLAGS=-Os -fPIC
RETRANSE_FLAGS=-I ../retranse
RETRANSE_LIBS=-L../retranse -lretranse -lpcre++ -lpcre
CPPDB_FLAGS=-I ../cppdb
CPPDB_LIBS=-L ../cppdb/lib -lcppdb
CPPDB_STATIC=-L ../cppdb/lib -lcppdb -lpthread -lmysqlclient -lpq -lodbc
#CPPDB_STATIC=-L ../cppdb/lib -lcppdb -lodbc
FLAGS=$(RETRANSE_FLAGS) $(CPPDB_FLAGS)
LIBS=$(RETRANSE_LIBS) $(CPPDB_STATIC)

# all

.PHONY: all

all: sql2text libsql2text.a libsql2text.so


DEPENDENCIES=lib/sql2text/shared/libcppdb_mysql.so lib/sql2text/shared/libcppdb_postgresql.so lib/sql2text/shared/libcppdb_sqlite3.so libretranse.so

libretranse.so:
	cp ../retranse/libretranse.so .
	
lib/sql2text/shared/libcppdb_mysql.so:
	cp ../cppdb/lib/libcppdb_mysql.* lib/sql2text/shared

lib/sql2text/shared/libcppdb_postgresql.so: 
	cp ../cppdb/lib/libcppdb_postgresql.* lib/sql2text/shared

lib/sql2text/shared/libcppdb_sqlite3.so: 
	cp ../cppdb/lib/libcppdb_sqlite3.* lib/sql2text/shared

#main targets

sql2text: main.o libsql2text.a $(DEPENDENCIES)
	$(CXX) $(CFLAGS) $(FLAGS) -o sql2text main.o -L. -lsql2text $(LIBS)

main.o: main.cpp sql2text.hpp bindir.hpp
	$(CXX) -c $(CFLAGS) $(FLAGS) main.cpp

sql2text.o: sql2text.cpp sql2text.hpp explode.hpp
	$(CXX) -c $(CFLAGS) $(FLAGS) sql2text.cpp

.cpp.o: 
	$(CXX) $(FLAGS) -c $<

libsql2text.a: sql2text.o $(DEPENDENCIES)
	ar r libsql2text.a sql2text.o

libsql2text.so: sql2text.o $(DEPENDENCIES)
	$(CXX) $(CFLAGS) -shared -o libsql2text.so sql2text.o $(RETRANSE_LIBS) $(CPPDB_LIBS)

.PHONY: clean, cleanall

clean:
	rm -f *.o

cleanall: clean
	rm -f *.so *.a lib/sql2text/shared/* sql2text


.PHONY: install, uninstall

install: all
	cp -r lib $(INSTPATH)
	cp -r etc $(ETCPATH)
	cp sql2text $(INSTPATH)bin/
	cp libsql2text.* $(INSTPATH)lib/
	cp sql2text.hpp $(INSTPATH)include/


uninstall:
	rm -f $(INSTPATH)lib/libsql2text.*
	rm -f $(INSTPATH)bin/sql2text
	rm -f $(INSTPATH)include/sql2text.hpp
	rm -rf $(ETCPATH)etc/sql2text
	rm -rf $(INSTPATH)lib/sql2text


