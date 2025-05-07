# call-extern-api

This example does the things below:

- initialize DHCP and wait until it is completed
- initialize GPIO and preallocate data
- initilize the client connection to the external server API
- start a timer triggered every 500 ms
- when the timer triggers:
  * change the state of the led
  * make a request to the external API
  * if reply is expected, print the reply of the request

This example should be configured. This is done in files `main.c`
lines 15 to 45. The values to setup are:

- SERVER\_IP: IP of the server as a string
- SERVER\_PORT: port of the service as a string
- LED\_NODE: indication of the led, depends on the board
- IGNORE\_RESPONSE: boolean indicating if response is ignored or not
- MTLS: boolean indicating if, when doing TLS, the certificate
  verification should be mutual or not

This example also needs the server API to run. This can be done using the
binder and the binding tutorial hello4. Here are the step to follow.

1. Install afb-binder on the server (see [Setup your build host][1])
2. Get the tutorial of afb-binding by cloning
   https://github.com/redpesk-core/afb-binding in some directory `DIR`
3. Compile the tutorial hello4: `make -C DIR/tutorials/v4 hello4.so`
4. Run the binder using the command:
   `afb-binder -b DIR/tutorials/v4/hello4.so -v --rpc-server=tcp:*:4444/hello --tracereq=common`

A typical output of zephyr logs is:

```
*** Booting Zephyr OS build v4.0.0-65-ge397b1b65790 ***
[00:00:00.003,000] <inf> ENTERING MAIN
[00:00:00.003,000] <inf> net_dhcpv4_client_sample: Run dhcpv4 client
[00:00:00.003,000] <inf> net_dhcpv4_client_sample: Start on enc424j600@0: index=1
[00:00:01.550,000] <inf> ethdrv: Link up
[00:00:01.551,000] <inf> ethdrv: 100Mbps
[00:00:01.551,000] <inf> ethdrv: full duplex
[00:00:09.883,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:00:10.085,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:00:10.086,000] <inf> net_dhcpv4: Received: 10.18.127.29
[00:00:10.086,000] <inf> net_dhcpv4_client_sample:    Address[1]: 10.18.127.29
[00:00:10.086,000] <inf> net_dhcpv4_client_sample:     Subnet[1]: 255.255.0.0
[00:00:10.086,000] <inf> net_dhcpv4_client_sample:     Router[1]: 10.18.253.252
[00:00:10.086,000] <inf> net_dhcpv4_client_sample: Lease time[1]: 1800 seconds
[00:00:10.086,000] <inf> STARTING AFB
[00:00:10.086,000] <inf> API ROOT added
[00:00:10.086,000] <inf> STARTING START
[00:00:10.091,000] <inf> API extern added
[00:00:10.093,000] <inf> API ROOT starting...
[00:00:10.093,000] <inf> API ROOT started
[00:00:10.093,000] <inf> API extern starting...
[00:00:10.093,000] <inf> API extern started
[00:00:10.603,000] <inf> replied 0, [false,1]
[00:00:11.102,000] <inf> replied 0, [true,2]
[00:00:11.602,000] <inf> replied 0, [false,3]
[00:00:12.102,000] <inf> replied 0, [true,4]
[00:00:12.601,000] <inf> replied 0, [false,5]
[00:00:13.101,000] <inf> replied 0, [true,6]
[00:00:13.602,000] <inf> replied 0, [false,7]
[00:00:14.102,000] <inf> replied 0, [true,8]
[00:00:14.603,000] <inf> replied 0, [false,9]
[00:00:15.102,000] <inf> replied 0, [true,10]
...
```

And a typical output of the server is:

```
   NOTICE: Browser URL= http://localhost:1234
   NOTICE: Serving rootdir=. uploaddir=.
   NOTICE: [API hello] hello binding comes to live
   NOTICE: [API hello] hello binding is /home/jobol/redpesk/redpesk-core/afb-binding/tutorials/v4/hello4.so
   NOTICE: [API hello] hello binding's uid is (null)
   NOTICE: [API hello] hello binding's config is { "binding-path": "\/home\/jobol\/redpesk\/redpesk-core\/afb-binding\/tutorials\/v4\/hello4.so", "path": ".\/hello4.so" }
   NOTICE: API hello starting...
   NOTICE: [API hello] hello binding starting
   NOTICE: [API hello] aliasing [API hello] to [API fakename]
   NOTICE: API monitor starting...
   NOTICE: Listening interface *:1234
HOOK: [req-000001:hello/ping] BEGIN false, 1
HOOK: [req-000001:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000001:hello/ping] reply[success: 0] [false,1], (null), ping count=1
HOOK: [req-000001:hello/ping] END
HOOK: [req-000002:hello/ping] BEGIN true, 2
HOOK: [req-000002:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000002:hello/ping] reply[success: 0] [true,2], (null), ping count=2
HOOK: [req-000002:hello/ping] END
HOOK: [req-000003:hello/ping] BEGIN false, 3
HOOK: [req-000003:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000003:hello/ping] reply[success: 0] [false,3], (null), ping count=3
HOOK: [req-000003:hello/ping] END
HOOK: [req-000004:hello/ping] BEGIN true, 4
HOOK: [req-000004:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000004:hello/ping] reply[success: 0] [true,4], (null), ping count=4
HOOK: [req-000004:hello/ping] END
HOOK: [req-000005:hello/ping] BEGIN false, 5
HOOK: [req-000005:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000005:hello/ping] reply[success: 0] [false,5], (null), ping count=5
HOOK: [req-000005:hello/ping] END
HOOK: [req-000006:hello/ping] BEGIN true, 6
HOOK: [req-000006:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000006:hello/ping] reply[success: 0] [true,6], (null), ping count=6
HOOK: [req-000006:hello/ping] END
HOOK: [req-000007:hello/ping] BEGIN false, 7
HOOK: [req-000007:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000007:hello/ping] reply[success: 0] [false,7], (null), ping count=7
HOOK: [req-000007:hello/ping] END
HOOK: [req-000008:hello/ping] BEGIN true, 8
HOOK: [req-000008:hello/ping] CRED uid=99=99 gid=99 pid=0 label=NoLabel id=NoLabel
HOOK: [req-000008:hello/ping] reply[success: 0] [true,8], (null), ping count=8
HOOK: [req-000008:hello/ping] END
...
```

## Adding TLS



-DEXTRA_CONF_FILE=overlay-tls.conf

[1]: https://docs.redpesk.bzh/docs/en/master/getting_started/host-configuration/docs/1-Setup-your-build-host.html
