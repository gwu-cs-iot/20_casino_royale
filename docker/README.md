# Dockerized Version

This version will automatically install all the prerequisities (minus WiringPi on host machine) and run the casino-royale program when starting the container.

**NOTE:** Building this container will take a while... opencv building from source >1 hour on Raspberry Pis, be sure to never clear your docker cache.

**NOTE 2:** Privileged mode is enabled since the container will need access to the host's devices (camera + /dev/gpiomem (for buzzer)). WiringPi is still required on the host machine!

*The containerized version is provided as-is and is experimental.* Might be worthwhile to add an additional container to host the webserver in the future.

## Usage

- Install WiringPi on host machine (version 2.52+ for Pi4)
	- `sudo apt-get install wiringpi` or `wget https://project-downloads.drogon.net/wiringpi-latest.deb && dpkg -i wiringpi-latest.deb`
- Install docker on host machine: [https://docs.docker.com/engine/install/](https://docs.docker.com/engine/install/)
- Start docker container: `docker build -t casino-royale . && docker run --privileged -it casino-royale`
	- Can also be started with docker-compose: `docker compose up`
