Run 'make httpserver' to compile

Progam usage: httpserver [-N] HostName/IP [Port]

*HostName required

Port is set to 80 by default

-N specifies number of threads, default is 4

Server only supports GET and PUT requests
All other requests will return an error


```
$ make httpserver
```
To start the server simply run `httpserver`

Here is an example using localhost/127.0.0.1 and port 8888. 

The server will make 4 threads since no -N option was provided to specify how many threads to create

```
  $ ./httpserver 127.0.0.1 8888
  
```

In a terminal use something like `wget` or `curl` to send a GET or POST request to the specified Hostname and Port

```
  $ wget http://127.0.0.1:8888/request_file
```
  
