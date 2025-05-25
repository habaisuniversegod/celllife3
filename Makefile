CC = g++
SFML_DEPS = -lsfml-graphics -lsfml-window -lsfml-system
EXEC_NAME = celllife

#FLAGS =  -fsanitize=undefined,address -g -O1

FLAGS = -O3 -march=native -flto -funroll-loops -fomit-frame-pointer -fno-stack-protector -ffast-math -fno-math-errno -fno-signed-zeros -fno-trapping-math -fassociative-math

OBJS = cell.o field.o main.o

all: $(EXEC_NAME)

$(EXEC_NAME): $(OBJS)
	$(CC) $(FLAGS) -o $@ $^ $(SFML_DEPS)

%.o: %.cpp %.h
	$(CC) $(FLAGS) -c $< -o $@

main.o: main.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC_NAME)