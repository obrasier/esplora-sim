CC=clang
CXX=clang++
INC=-I../inc/json -I../inc -I./json -I../sketch
TARGET=esplora_sim
CPPFLAGS= -fsanitize=address -std=c++11
LIBS= -lpthread
SRC=Main.cpp Arduino.cpp Hardware.cpp String.cpp Serial.cpp
OBJS= $(SRC:.cpp=.o)
JSRC = json/buffer.c json/lexer.c json/logging.c json/json.c
JOBJS= $(JSRC:.c=.o)



all: $(TARGET)

$(TARGET): $(JOBJS) $(OBJS) 
	$(CXX) $(CPPFLAGS) $(INC) -o $(TARGET) $(JOBJS) $(OBJS) $(LIBS)


.c.o:
	$(CC) $(INC) -c $< -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $(INC) -c $< -o $@

clean:
	rm *.o $(TARGET) 

