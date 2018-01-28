FLAGS=-lncurses -std=c++11
snaker: game.cpp
        g++ -o snaker game.cpp $(FLAGS)
