all:
	g++ -std=c++11 `pkg-config --cflags opencv` density.cpp jpeg_decode.cpp rgb2gray.cpp decodeGray.cpp -o decodeGray -march=nehalem -ljpeg `pkg-config --libs opencv`
compile:
#add -mfpu=neon for RaspberryPi
#change I/... when change folder
