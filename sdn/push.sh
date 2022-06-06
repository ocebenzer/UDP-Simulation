REPOSITORY="ocebenzer/udpsdn"

docker build -t $REPOSITORY .
docker push $REPOSITORY
