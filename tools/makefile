first: build-database

sqlite3.o:
	gcc -c ../src/sqlite/sqlite3.c -o $@

.cpp.o:
	g++ -Wno-dangling-else -std=c++11 -c $< -o $@

build-database: build-database.o ngp-games-row.o ws-games-row.o sqlite3.o
	g++ -std=c++11 $^ -o $@

clean:
	@rm -rf *.o
	@rm -f build-database
