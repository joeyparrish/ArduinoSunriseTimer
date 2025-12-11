.PHONY: demo

all: demo

demo:
	g++ -o demo -x c++ -I src examples/Demo/Demo.ino src/SunriseTimer.cpp
