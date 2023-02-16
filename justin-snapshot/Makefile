#
# 'make'        build all executable files
# 'make server' build only server executable file
# 'make client' build only client executable file
# 'make test'   build only test executable file
# 'make clean'  removes all .o and executable files
#

# Basic Variables
CC           := gcc
CFLAGS       := -Wall
#CFLAGS       := -g -lpthread -O2 -Werror -Wall -Wextra -Wpedantic -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Wformat-security -Wnull-dereference -Wstack-protector -Wtrampolines -Walloca -Wvla -Warray-bounds=2 -Wimplicit-fallthrough=3 -Wshift-overflow=2 -Wcast-qual -Wstringop-overflow=4 -Wconversion -Wint-conversion -Wlogical-op -Wduplicated-cond -Wduplicated-branches -Wformat-signedness -Wshadow -Wstrict-overflow=4 -Wundef -Wstrict-prototypes -Wswitch-default -Wswitch-enum -Wstack-usage=1000000 -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fstack-clash-protection -fPIE -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code -fsanitize=thread #-Wtraditional-conversion -Wcast-align=strict

BUILD_DIR    := build
OBJ_DIR      := $(BUILD_DIR)/obj
SRC_DIR      := src
DRV_DIR      := drv

# General Requirements
SRCS         := $(wildcard $(SRC_DIR)/*.c)
DRVS         := $(wildcard $(DRV_DIR)/*.c)
SRC_OBJS     := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DRV_OBJS     := $(DRVS:$(DRV_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS         := $(SRC_OBJS:.o=%.d) $(DRV_OBJS:.o=%.d)

# Executables
SERVER_EXE   := $(BUILD_DIR)/server
CLIENT_EXE   := $(BUILD_DIR)/client
TEST_EXE     := $(BUILD_DIR)/test

#.PHONY: clean

# Entry Points
all: test client server
server: | dirs $(SERVER_EXE)
client: | dirs $(CLIENT_EXE)
test: | dirs $(TEST_EXE)

# Called for all Executables
dirs:
	mkdir -p $(BUILD_DIR) $(OBJ_DIR)

# Compiling each Executable
$(CLIENT_EXE): $(SRC_OBJS) $(DRV_OBJS)
	$(CC) -o $@ $(DRV_DIR)/client.c $(SRC_OBJS) $(CFLAGS)
$(SERVER_EXE): $(SRC_OBJS) $(DRV_OBJS)
	$(CC) -o $@ $(DRV_DIR)/server.c $(SRC_OBJS) $(CFLAGS)
$(TEST_EXE): $(SRC_OBJS) $(DRV_OBJS)
	$(CC) -o $@ $(DRV_DIR)/test.c $(SRC_OBJS) $(CFLAGS)

# Compiling any needed OBJ files from SRC or DRV
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)
$(OBJ_DIR)/%.o: $(DRV_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

# Extra Cleaning Entry Point
clean:
	rm -fr $(BUILD_DIR)

-include $(DEPS)
