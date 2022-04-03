docker run \
    --network udp_network \
    --ip 192.168.10.10 \
    --hostname udpserver \
    --name udpserver \
    --rm -it server
