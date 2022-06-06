REPOSITORY="ocebenzer/udpclient"

docker build -t $REPOSITORY .
docker push $REPOSITORY
