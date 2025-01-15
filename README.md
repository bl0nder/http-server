# HTTP Server in C
Trying to make a webserver in C.

Compile: 

```bash
gcc http_server.c -o http_server
```

Run: 

```bash
./http_server [port]
```

Go to `127.0.0.1:port/test` for a simple webpage being served. Any other route (including `127.0.0.1:port/`) will return a `404: Page not found`.
