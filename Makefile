.PHONY: demo zip install

all: demo

demo:
	g++ -o demo -x c++ -I src examples/Demo/Demo.ino src/SunTimer.cpp

zip:
	git archive --format=zip --output=ArduinoSunTimer.zip --prefix=ArduinoSunTimer/ main

install: zip
	arduino-cli config init || true
	arduino-cli config set library.enable_unsafe_install true
	arduino-cli lib install --zip-path ArduinoSunTimer.zip
