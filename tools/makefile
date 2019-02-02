BUILDDIR=./build
SOURCEDIR=./src

CC=gcc
CFLAGS=-Wno-dangling-else

CPPC=g++
CPPFLAGS=-Wno-dangling-else -std=c++11

ifeq ($(OS),Windows_NT)
    CPPFLAGS+=
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -lpthread -ldl
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -lpthread -ldl
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += 
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += 
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += 
    endif
endif

TARGETS=build-database


first: build-database


$(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CPPC) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


build-database: $(addprefix $(BUILDDIR)/, build-database.o ngp-games-row.o ws-games-row.o ngp-cart-row.o sqlite/sqlite3.o)
	$(CPPC) $(CPPFLAGS) $^ -o $@

clean:
	@find $(BUILDDIR) -name "*.o" -type f -delete
	@rm -f $(TARGETS)
