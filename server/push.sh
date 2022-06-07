REPOSITORY="ocebenzer/udpserver"

docker build -t $REPOSITORY .
docker push $REPOSITORY
