make full:
	$(MAKE) -C TwoPlusTwoHandEvaluator/
	g++ -O2 -o casino-royale casino-royale.cpp -I/usr/local/include/opencv4 -lopencv_core -lopencv_videoio -lopencv_objdetect -lopencv_imgproc -lzbar -lwiringPi -lcurl
 

make quick:
	g++ -o casino-royale casino-royale.cpp -I/usr/local/include/opencv4 -lopencv_core -lopencv_videoio -lopencv_objdetect -lopencv_imgproc -lzbar -lwiringPi -lcurl

make build:
	g++ -O2 -o casino-royale casino-royale.cpp -I/usr/local/include/opencv4 -lopencv_core -lopencv_videoio -lopencv_objdetect -lopencv_imgproc -lzbar -lwiringPi -lcurl

clean:
	rm casino-royale
	rm TwoPlusTwoHandEvaluator/HandRanks.dat
