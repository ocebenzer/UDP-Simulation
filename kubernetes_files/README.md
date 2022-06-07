
# How to run the simulation

## Cluster Initialization

* Create a kubernetes cluster
    * [minikube](https://minikube.sigs.k8s.io/docs/start/) might be a good start:
        ```
        minikube config set memory 1536M
        minikube config set cpus 2
        minikube config set disk-size 20GB
        ```
        ```
        minikube start --nodes 3
        alias kubectl="minikube kubectl --"
        ```

* Create an NFS Server:
    * Create the NFS Server Node:
        ```
        kubectl apply -f NFS.yaml
        ```
    * Find the NFS Server IP:
        ```
        kubectl get pod -o wide
        ```
    * Set the nfs.server to newly created NFS Server IP in `PV.yaml`
    * Create the Persistent Volume && Persistent Volume Claim:
        ```
        kubectl apply -f PV.yaml
        ```


## Pod Initialization
* Create BBU pods:
    ```
    kubectl apply -f BBU1.yaml -f BBU2.yaml -f BBU3.yaml -f BBU4.yaml
    ```

* Create the SDN pod:
    ```
    kubectl apply -f SDN.yaml
    ```

* Wait until pods are created:
    ```
    kubectl get pod -o wide
    ```

* When pods are created, set the SDN pod IP address as the `SERVER_IP` value in `RRU*.yaml` files

* Enter the tti of SDN:
    ```
    kubectl attach sdn -it
    ```
    * type `help` for SDN usage
    * example SDN port forwarding:
        ```
        add 61001 10.244.1.2:61000
        add 61002 10.244.1.3:61000
        add 61003 10.244.1.4:61000
        add 61004 10.244.2.2:61000
        ```
    * example SDN port forwarding update
        ```
        update 2 10.244.2.3:61000
        ```

* Create RRU pods:
    ```
    kubectl apply -f RRU1.yaml -f RRU2.yaml -f RRU3.yaml -f RRU4.yaml
    ```

* Wait until pod is created:
    ```
    kubectl get status -o wide
    ```

## Debug
* You ssh into nodes using:
    ```
    minikube ssh
    minikube ssh -n ${NODE_NAME}
    ```
    or
    ```
    ssh -i $(minikube ssh-key) docker@$(minikube ip)
    ```
* You can also sh into running pods using:
    ```
    kubectl exec --stdin --tty ${POD_NAME} -- /bin/sh
    ```

## Results
* Results will be in the nfs server node:
    ```
    kubectl exec --stdin --tty nfs -- /bin/sh
    # cd /exports
    # ls
    ```
* Get the .csv files:
    ```
    kubectl cp nfs:/exports ./logs
    ```


### Well Done
* Don't forget to delete minikube cluster:
    ```
    minikube delete
    ```