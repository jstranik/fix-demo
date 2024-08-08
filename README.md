# Professional Example of Communication Between Server and Client

## Overview
This repository contains a server and a client that communicate using the FIX (Financial Information eXchange) protocol. The server accepts single orders and replies with an execution report, while the client sends repeated execution messages and prints any execution reports received.

## Running the Code
To run the server, use the following command:

```
nix run .#fix-server
```

To run the client, use the following command:

```
nix run .#fix-client
```


### Remote startup
You can also run the binaries remotely as: 

``` 
nix run github:jstranik/fix-demo#fix-server 
nix run github:jstranik/fix-demo#fix-client
```
