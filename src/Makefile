CC=clang
CXX=clang++
INC=-I./inc/json -I./inc -I./json -I./sketch
TARGET=esplora_sim
CPPFLAGS= -fsanitize=address -std=c++11 -W -Wall -pedantic -Wno-reserved-id-macro -Wno-keyword-macro
LIBS= -lpthread
SRC=Arduino.cpp Device.cpp WString.cpp Serial.cpp ultoa.cpp Esplora.cpp Main.cpp
OBJS= $(SRC:.cpp=.o)
JSRC = json/buffer.c json/lexer.c json/logging.c json/json.c
JOBJS= $(JSRC:.c=.o)

all: $(TARGET)

$(TARGET): $(JOBJS) $(OBJS) $(SOBJS)
	$(CXX) $(CPPFLAGS) $(INC) -o $(TARGET) $(JOBJS) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $(INC) -c $< -o $@


clean:
	rm *.o $(TARGET) 
	touch ___client_events ___device_updates
	rm ___device_updates ___client_events

