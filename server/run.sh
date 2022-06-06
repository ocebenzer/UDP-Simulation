PORT="61000"
LOGFILE_PATH="./log.csv"
SERVER_ID="server"

make
./server.out $PORT $LOGFILE_PATH $SERVER_ID

# docker run --rm -it --net=host server
