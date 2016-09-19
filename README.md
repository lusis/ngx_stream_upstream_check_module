# ngx_stream_upstream_health_check
This module is a port of [http_upstream_health_check](https://github.com/yaoweibin/nginx_upstream_check_module) module to support using health checks in nginx streams
Ideally this should be a part of that module but it was easier to work with an isolated patch

## Building
Currently this has only been tested with nginx 1.11+. The stream module is not compiled in by default.

```
cd nginx-1.11.2
patch -p0 < /location/to/this/repo/clone/patch-1.11.x.patch
./configure --with-stream --add-module=/location/to/this/repo/clone
make
```

## Usage
The usage is almost exactly the same as documented in the http upstream check module. 
The biggest difference is that upstreams are defined inside a `stream` section.

This module supports the original status page queries but that should live in an http location block.

## TODO
The SSL support really needs updating but so does the original module's SSL handshake support. Ideally the logic in the [older tcp module support](https://github.com/yaoweibin/nginx_tcp_proxy_module) (which was needed before nginx opened up `stream`), would be used.
Testing is sadly impossible right now as the [Test::Nginx](https://github.com/openresty/test-nginx) library doesn't yet support defining streams. I've opened an issue for this and plan on working on a PR.

## Modified version of original readme is below

```
Name
    nginx_stream_upstream_check_module - support upstream health check with
    Nginx in stream definitions

Synopsis
    stream {

        upstream cluster {

            # simple round-robin
            server 192.168.0.1:80;
            server 192.168.0.2:80;

            check interval=5000 rise=1 fall=3 timeout=4000;

            #check interval=3000 rise=2 fall=5 timeout=1000 type=ssl_hello;

            #check interval=3000 rise=2 fall=5 timeout=1000 type=http;
            #check_http_send "HEAD / HTTP/1.0\r\n\r\n";
            #check_http_expect_alive http_2xx http_3xx;
        }

        server {
            listen 8080;
            proxy_pass http://cluster;
        }
    }

    http {
        server {
	    listen 80;
	    server_name localhost;

            location /stream_status {
                stream_check_status;
	    }

        }

    }

Description
    Add the support of health check with the upstream servers.

Directives
  check
    syntax: *check interval=milliseconds [fall=count] [rise=count]
    [timeout=milliseconds] [default_down=true|false]
    [type=tcp|http|ssl_hello|mysql|ajp|fastcgi]*

    default: *none, if parameters omitted, default parameters are
    interval=30000 fall=5 rise=2 timeout=1000 default_down=true type=tcp*

    context: *upstream*

    description: Add the health check for the upstream servers.

    The parameters' meanings are:

    *   *interval*: the check request's interval time.

    *   *fall*(fall_count): After fall_count check failures, the server is
        marked down.

    *   *rise*(rise_count): After rise_count check success, the server is
        marked up.

    *   *timeout*: the check request's timeout.

    *   *default_down*: set initial state of backend server, default is
        down.

    *   *port*: specify the check port in the backend servers. It can be
        different with the original servers port. Default the port is 0 and
        it means the same as the original backend server.

    *   *type*: the check protocol type:

        1.  *tcp* is a simple tcp socket connect and peek one byte.

        2.  *ssl_hello* sends a client ssl hello packet and receives the
            server ssl hello packet.

        3.  *http* sends a http request packet, receives and parses the http
            response to diagnose if the upstream server is alive.

        4.  *mysql* connects to the mysql server, receives the greeting
            response to diagnose if the upstream server is alive.

        5.  *ajp* sends a AJP Cping packet, receives and parses the AJP
            Cpong response to diagnose if the upstream server is alive.

        6.  *fastcgi* send a fastcgi request, receives and parses the
            fastcgi response to diagnose if the upstream server is alive.

  check_http_send
    syntax: *check_http_send http_packet*

    default: *"GET / HTTP/1.0\r\n\r\n"*

    context: *upstream*

    description: If you set the check type is http, then the check function
    will sends this http packet to check the upstream server.

  check_http_expect_alive
    syntax: *check_http_expect_alive [ http_2xx | http_3xx | http_4xx |
    http_5xx ]*

    default: *http_2xx | http_3xx*

    context: *upstream*

    description: These status codes indicate the upstream server's http
    response is ok, the backend is alive.

  check_keepalive_requests
    syntax: *check_keepalive_requests num*

    default: *check_keepalive_requests 1*

    context: *upstream*

    description: The directive specifies the number of requests sent on a
    connection, the default vaule 1 indicates that nginx will certainly
    close the connection after a request.

  check_fastcgi_param
    Syntax: *check_fastcgi_params parameter value*

    default: see below

    context: *upstream*

    description: If you set the check type is fastcgi, then the check
    function will sends this fastcgi headers to check the upstream server.
    The default directive looks like:

          check_fastcgi_param "REQUEST_METHOD" "GET";
          check_fastcgi_param "REQUEST_URI" "/";
          check_fastcgi_param "SCRIPT_FILENAME" "index.php";

  check_shm_size
    syntax: *check_shm_size size*

    default: *1M*

    context: *http*

    description: Default size is one megabytes. If you check thousands of
    servers, the shared memory for health check may be not enough, you can
    enlarge it with this directive.

  check_status
    syntax: *check_status [html|csv|json]*

    default: *none*

    context: *location*

    description: Display the health checking servers' status by HTTP. This
    directive should be set in the http block.

    You can specify the default display format. The formats can be `html`,
    `csv` or `json`. The default type is `html`. It also supports to specify
    the format by the request argument. Suppose your `check_status` location
    is '/status', the argument of `format` can change the display page's
    format. You can do like this:

        /status?format=html
        /status?format=csv
        /status?format=json

    At present, you can fetch the list of servers with the same status by
    the argument of `status`. For example:

        /status?format=html&status=down
        /status?format=csv&status=up

    Below it's the sample html page:

        <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
        <title>Nginx http upstream check status</title>
            <h1>Nginx http upstream check status</h1>
            <h2>Check upstream server number: 1, generation: 3</h2>
                    <th>Index</th>
                    <th>Upstream</th>
                    <th>Name</th>
                    <th>Status</th>
                    <th>Rise counts</th>
                    <th>Fall counts</th>
                    <th>Check type</th>
                    <th>Check port</th>
                    <td>0</td>
                    <td>backend</td>
                    <td>106.187.48.116:80</td>
                    <td>up</td>
                    <td>39</td>
                    <td>0</td>
                    <td>http</td>
                    <td>80</td>

    Below it's the sample of csv page:

        0,backend,106.187.48.116:80,up,46,0,http,80

    Below it's the sample of json page:

        {"servers": {
          "total": 1,
          "generation": 3,
          "server": [
           {"index": 0, "upstream": "backend", "name": "106.187.48.116:80", "status": "up", "rise": 58, "fall": 0, "type": "http", "port": 80}
          ]
         }}

Installation
    Download the latest version of the release tarball of this module from
    github (<http://github.com/yaoweibin/nginx_upstream_check_module>)

    Grab the nginx source code from nginx.org (<http://nginx.org/>), for
    example, the version 1.0.14 (see nginx compatibility), and then build
    the source with this module:

        $ wget 'http://nginx.org/download/nginx-1.0.14.tar.gz'
        $ tar -xzvf nginx-1.0.14.tar.gz
        $ cd nginx-1.0.14/
        $ patch -p1 < /path/to/nginx_http_upstream_check_module/check.patch

        $ ./configure --add-module=/path/to/nginx_http_upstream_check_module

        $ make
        $ make install

Compatibility
    *   I've only tested the patch against 1.11.x versions

Notes
TODO
Known Issues
Changelogs
  v0.1
    *   first release

Authors

    John E Vincent *lusis.org+github.com@gmail.com*

    Weibin Yao(姚伟斌) *yaoweibin at gmail dot com*

    Matthieu Tourne

Copyright & License

    The health check part is borrowed the design of Jack Lindamood's
    healthcheck module healthcheck_nginx_upstreams
    (<http://github.com/cep21/healthcheck_nginx_upstreams>);

    This module is licensed under the BSD license.

    Copyright (C) 2016 by John E Vincent <lusis.org+github.com@gmail.com>

    Copyright (C) 2014 by Weibin Yao <yaoweibin@gmail.com>

    Copyright (C) 2010-2014 Alibaba Group Holding Limited

    Copyright (C) 2014 by LiangBin Li

    Copyright (C) 2014 by Zhuo Yuan

    Copyright (C) 2012 by Matthieu Tourne

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    *   Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
