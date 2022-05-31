 
# How to run the simulation

## Cluster Initialization

* Create a kubernetes cluster
    * [mininet](http://mininet.org/download/) might be a good start

* Create a Persistent Volume && Persistent Volume Claim:
    ```
    kubectl apply -f pv.yaml
    kubectl apply -f pvc.yaml
    ```
    * This makes sure that our storage is independent from our Server pods

## Pod Initialization
* Create a Server pod:
    ```
    kubectl apply -f server.yaml
    ```

* Wait until pod is created:
    ```
    kubectl get status -o wide
    ```

* When pod is created, set the pod IP address as the `SERVER_IP` value in `client.yaml` file

* Create a Client pod:
    ```
    kubectl apply -f client.yaml
    ```

* Wait until pod is created:
    ```
    kubectl get status -o wide
    ```

## See Results
* Enter the storage pod:
    ```
    kubectl exec --stdin --tty server -- /bin/sh
    ```

* Display the .csv file:
    ```
    cat /storage/server.csv
    ```

### Well Done
