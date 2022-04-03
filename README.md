 
### Instructions

* Create a network using command:
`docker network create --attachable --driver=bridge --subnet=192.168.0.0/16 --ip-range=192.168.10.0/24 udp_network`

* Build & Run Server image using:
`./Server/build.sh && ./Client/run.sh`

* Build & Run Client image using:
`./Client/build.sh && ./Client/run.sh`
