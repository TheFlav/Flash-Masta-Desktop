first: build-database

sqlite3.o:
	gcc -c ../src/sqlite/sqlite3.c -o $@

.cpp.o:
	g++ -c $< -o $@

build-database: build-database.o sqlite3.o
	g++ $^ -o $@

clean:
	@rm -rf *.o
	@rm -f build-database
