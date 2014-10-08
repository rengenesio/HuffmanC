CC = gcc
OPTIMIZATION = -O3
CFLAGS = -Wall -Wpadded -std=gnu99 $(OPTIMIZATION)
RM = rm -rf

all: normal optimized thread

clean: clean_normal clean_optimized clean_thread

clean_all: clean_normal_all clean_optimized_all clean_thread_all


########## NORMAL ##########
PATH_SRC_NORMAL = src/normal
PATH_OBJ_NORMAL = src/normal
TARGET_NORMAL = release/normal/huffmanC$(OPTIMIZATION)

OBJ_NORMAL = $(PATH_OBJ_NORMAL)/main.o \
	     $(PATH_OBJ_NORMAL)/decoder.o \
	     $(PATH_OBJ_NORMAL)/encoder.o \
	     $(PATH_OBJ_NORMAL)/huffman.o

normal: $(TARGET_NORMAL)

optimized: $(TARGET_OPTIMIZED)

$(TARGET_NORMAL): $(OBJ_NORMAL)
	$(CC) $(OBJ_NORMAL) -o $@
	     
$(PATH_OBJ_NORMAL)/main.o: $(PATH_SRC_NORMAL)/main.c $(PATH_OBJ_NORMAL)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_NORMAL)/main.c -o $@

$(PATH_OBJ_NORMAL)/encoder.o: $(PATH_SRC_NORMAL)/encoder.c $(PATH_OBJ_NORMAL)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_NORMAL)/encoder.c -o $@

$(PATH_OBJ_NORMAL)/decoder.o: $(PATH_SRC_NORMAL)/decoder.c $(PATH_OBJ_NORMAL)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_NORMAL)/decoder.c -o $@

$(PATH_OBJ_NORMAL)/huffman.o: $(PATH_SRC_NORMAL)/huffman.c $(PATH_SRC_NORMAL)/huffman.h
	$(CC) -c $(CFLAGS) $(PATH_SRC_NORMAL)/huffman.c -o $@

clean_normal:
	$(RM) $(OBJ_NORMAL)

clean_all_normal: clean_normal
	$(RM) $(TARGET_NORMAL)

	
########## OPTIMIZED ##########
PATH_SRC_OPTIMIZED = src/optimized
PATH_OBJ_OPTIMIZED = src/optimized
TARGET_OPTIMIZED = release/optimized/huffmanC$(OPTIMIZATION)

OBJ_OPTIMIZED = $(PATH_OBJ_OPTIMIZED)/main.o \
	     $(PATH_OBJ_OPTIMIZED)/decoder.o \
	     $(PATH_OBJ_OPTIMIZED)/encoder.o \
	     $(PATH_OBJ_OPTIMIZED)/huffman.o

optimized: $(TARGET_OPTIMIZED)

$(TARGET_OPTIMIZED): $(OBJ_OPTIMIZED)
	$(CC) $(OBJ_OPTIMIZED) -o $@
	     
$(PATH_OBJ_OPTIMIZED)/main.o: $(PATH_SRC_OPTIMIZED)/main.c $(PATH_OBJ_OPTIMIZED)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_OPTIMIZED)/main.c -o $@

$(PATH_OBJ_OPTIMIZED)/encoder.o: $(PATH_SRC_OPTIMIZED)/encoder.c $(PATH_OBJ_OPTIMIZED)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_OPTIMIZED)/encoder.c -o $@

$(PATH_OBJ_OPTIMIZED)/decoder.o: $(PATH_SRC_OPTIMIZED)/decoder.c $(PATH_OBJ_OPTIMIZED)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_OPTIMIZED)/decoder.c -o $@

$(PATH_OBJ_OPTIMIZED)/huffman.o: $(PATH_SRC_OPTIMIZED)/huffman.c $(PATH_SRC_OPTIMIZED)/huffman.h
	$(CC) -c $(CFLAGS) $(PATH_SRC_OPTIMIZED)/huffman.c -o $@

clean_optimized:
	$(RM) $(OBJ_OPTIMIZED)

clean_all_optimized: clean_optimized
	$(RM) $(TARGET_OPTIMIZED)


########## THREAD ##########
PATH_SRC_THREAD = src/thread
PATH_OBJ_THREAD = src/thread
TARGET_THREAD = release/thread/huffmanC$(OPTIMIZATION)
NTHREAD = 2 # same value in src/thread/huffman.h

OBJ_THREAD = $(PATH_OBJ_THREAD)/main.o \
	     $(PATH_OBJ_THREAD)/decoder.o \
	     $(PATH_OBJ_THREAD)/encoder.o \
	     $(PATH_OBJ_THREAD)/huffman.o

thread: $(TARGET_THREAD)_$(NTHREAD)

$(TARGET_THREAD)_$(NTHREAD): $(OBJ_THREAD)
	$(CC) $(OBJ_THREAD) -o $@
	     
$(PATH_OBJ_THREAD)/main.o: $(PATH_SRC_THREAD)/main.c $(PATH_OBJ_THREAD)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_THREAD)/main.c -o $@ -lpthread

$(PATH_OBJ_THREAD)/encoder.o: $(PATH_SRC_THREAD)/encoder.c $(PATH_OBJ_THREAD)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_THREAD)/encoder.c -o $@ -lpthread

$(PATH_OBJ_THREAD)/decoder.o: $(PATH_SRC_THREAD)/decoder.c $(PATH_OBJ_THREAD)/huffman.o
	$(CC) -c $(CFLAGS) $(PATH_SRC_THREAD)/decoder.c -o $@ -lpthread

$(PATH_OBJ_THREAD)/huffman.o: $(PATH_SRC_THREAD)/huffman.c $(PATH_SRC_THREAD)/huffman.h
	$(CC) -c $(CFLAGS) $(PATH_SRC_THREAD)/huffman.c -o $@ -lpthread

clean_thread:
	$(RM) $(OBJ_THREAD)*

clean_all_thread: clean_thread
	$(RM) $(TARGET_THREAD)*

