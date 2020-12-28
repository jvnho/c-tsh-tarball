FROM ubuntu:latest
RUN apt-get update && apt-get -y install build-essential && apt-get -y install gcc
RUN mkdir /home/projet/
COPY tsh /home/projet/tsh/
