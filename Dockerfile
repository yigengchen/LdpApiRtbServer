FROM gcc:4.8
RUN mkdir -p /src/myapp
COPY . /src/myapp
WORKDIR /src/myapp
RUN make clean && make
EXPOSE 8088
CMD ./start.sh
