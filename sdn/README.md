
todo
```
socat - udp-datagram:172.17.0.3:61000,bind=:61000
socat - udp-listen:61000 | socat - udp-sendto:172.17.0.3:61000
```
