## SDN

### configure sdn
create the sdn
```
kubectl apply -f sdn.yaml
```
ssh into pod
```
kubectl exec --stdin --tty sdn -- /bin/sh
```
configure tunnel
```
socat - udp4-listen:61000,fork | tee >(socat - udp-sendto:172.17.0.3:61000)
```


### todo:
```
kubectl exec
kubectl attach
samplicate 61000 172.17.0.3/61000
SSH -L Wrapper + Daemon
socat - udp4-listen:61000 | tee >(socat - udp-sendto:172.17.0.3:61000)
socat - udp4-listen:61000,fork | tee >(socat - udp-sendto:172.17.0.3:61000) >(socat - udp-sendto:172.17.0.4:6100)
```
