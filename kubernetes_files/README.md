
# How to run the simulation

## Cluster Initialization

* Create a kubernetes cluster
    * [minikube](https://minikube.sigs.k8s.io/docs/start/) might be a good start
    * ```
      minikube config set memory 2048
      minikube config set cpus 2
      minikube config set disk-size 20GB
      ```
    * ```
      minikube start --nodes 3 -p cloudran
      alias kubectl="minikube -p cloudran kubectl --"
      ```

* Create a Persistent Volume && Persistent Volume Claim:
    ```
    kubectl apply -f pv.yaml -f pvc.yaml
    ```
    * This makes sure that our storage is independent from our Server pods
    
    * Make sure to create specified local path on node with persistent volume

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

* Create RRU pods:
    ```
    kubectl apply -f RRU1.yaml -f RRU2.yaml -f RRU3.yaml -f RRU4.yaml
    ```

* Wait until pod is created:
    ```
    kubectl get status -o wide
    ```

## See Results
* You ssh into nodes using:
    ```
    minikube -p cloudran ssh
    minikube -p cloudran ssh -n ${NODE_NAME}
    ```
* You can also sh into running pods using:
    ```
    kubectl exec --stdin --tty ${POD_NAME} -- /bin/sh
    ```
* Get the .csv files:
    ```
    scp -i $(minikube -p cloudran ssh-key) \
        docker@$(minikube -p cloudran ip):/mnt/data/*.csv \
        .
    ```

### Well Done
* Don't forget to delete minikube cluster:
    ```
    minikube delete -p cloudran
    ```