CC = g++

#compiler flags
CFLAGS = -Wall

#inlcude path and linker
LINKER_FLAGS = -I/usr/local/include -I/usr/local/include/sphinxbase -I/usr/local/include/pocketsphinx -L/usr/local/lib -lpocketsphinx -lsphinxbase -lsphinxad

#the excutible name
TARGET = heyListen

all: $(TARGET)

$(TARGET): src/main.cpp
	$(CC) $(CFLAGS) $(LINKER_FLAGS) -o $(TARGET) src/main.cpp

install: $(TARGET)


clean:
	$(RM) $(TARGET) ~* hubo-us/~* src/~* 

