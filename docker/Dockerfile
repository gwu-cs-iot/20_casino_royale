FROM arm32v7/debian:buster
# raspbian is running debian, give this a whirl
# Have to enforce arm32v7/armhf architecture, wiringpi incompatiable with arm64
# Thankfully RPi4 is cross-compatiable with arm32v7 and arm64

RUN apt update
RUN apt-get install -y wget

# opencv installation
RUN wget -O opencv-4.2.0.tar.gz https://github.com/opencv/opencv/archive/refs/tags/4.2.0.tar.gz
RUN tar -xf opencv-4.2.0.tar.gz
WORKDIR /opencv-4.2.0
RUN apt-get install -y build-essential
RUN apt-get install -y cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
# RUN apt-get install -y python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
RUN mkdir build
WORKDIR /opencv-4.2.0/build
RUN cmake ../
RUN make -j4
# ^ 4 threads
RUN make install

# zbar installation
RUN apt-get install -y libzbar-dev libzbar0

# wiringpi installation
RUN wget https://project-downloads.drogon.net/wiringpi-latest.deb
RUN dpkg -i wiringpi-latest.deb

# libcurl
RUN apt-get install -y libcurl4-openssl-dev

WORKDIR /
RUN mkdir 20_casino_royale
WORKDIR 20_casino_royale
COPY . .

WORKDIR TwoPlusTwoHandEvaluator
RUN make
RUN mkdir /home/pi && mkdir /home/pi/20_casino_royale && mkdir /home/pi/20_casino_royale/TwoPlusTwoHandEvaluator
RUN cp HandRanks.dat /home/pi/20_casino_royale/TwoPlusTwoHandEvaluator/
WORKDIR ../
RUN make build

#CMD /bin/sh
CMD /20_casino_royale/casino-royale
