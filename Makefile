CXX=g++
FLAGS=-Werror -Wall -Wextra -pedantic -O3 -std=c++2a
LIBS=-pthread -lncurses

all: client_drive server_drive

client_drive: Client/client_driver.cc Client/broadcast.o Client/display.o Client/handle_server.o Client/client_base.o 
	$(CXX) $(FLAGS) -o $@ $^ $(LIBS)

server_drive: Server/server_driver.cc Server/server_state.o 
	$(CXX) $(FLAGS) -o $@ $^ $(LIBS)

%.o: %.cc
	$(CXX) $(FLAGS) -c -o $@ $^

clean:
	cd Client; rm *.o; cd ..
	cd Server; rm *.o; cd ..
	rm client_drive
	rm server_drive