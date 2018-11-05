# Installation

## libwebsockets bug in 2.4.1

This is fixed on master, download the code and compile with:

```sh
mkdir build
build
OPENSSL_ROOT_DIR=/usr/local/Cellar/openssl/1.0.2n/ cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/opt/libwebsockets/ -DLWS_IPV6=ON -DLWS_WITH_HTTP2=ON -DLWS_WITH_LIBEV=ON -DLWS_WITH_LIBEVENT=ON -DLWS_WITH_LIBUV=ON -DLWS_WITH_PLUGINS=ON -DLWS_WITHOUT_TESTAPPS=ON -DLWS_UNIX_SOCK=ON -DCMAKE_BUILD_TYPE=DEBUG 
```
