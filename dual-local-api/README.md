# dual-local-api

This examples shows the setup of 2 bindings in zephyr:
tuto0 and tuto1.

After being initialized, tuto0 sends a request to tuto1. Tuto1 receives the
request, print it and responds to it. Tuto0 receives the reply and print it.

Then after 1 second, the opposite is done, tuto1 sends a request to tuto0.
Tuto0 receives the request, print it and responds to it. Tuto1 receives the reply
and print it.

Because operation are exactly the same, only one verb request callback and one reply
callback are coded, used by both APIs.

The expected output looks like:

```
*** Booting Zephyr OS build v4.0.0-65-ge397b1b65790 ***
[00:00:00.003,000] <inf> API ROOT added
[00:00:00.027,000] <inf> Adding APIs ...
[00:00:00.027,000] <inf> API tuto0 added
[00:00:00.027,000] <inf> [API tuto0] hello binding comes to live with uid tuto0
[00:00:00.027,000] <inf> API tuto1 added
[00:00:00.027,000] <inf> [API tuto1] hello binding comes to live with uid tuto1
[00:00:00.027,000] <inf> Starting APIs ...
[00:00:00.027,000] <inf> API ROOT starting...
[00:00:00.027,000] <inf> API ROOT started
[00:00:00.027,000] <inf> API tuto0 starting...
[00:00:00.028,000] <inf> [API tuto0] hello binding starting
[00:00:00.028,000] <inf> API tuto0 started
[00:00:00.028,000] <inf> API tuto1 starting...
[00:00:00.028,000] <inf> [API tuto1] hello binding starting
[00:00:00.028,000] <inf> API tuto1 started
[00:00:00.100,000] <inf> API tuto0 call API tuto1 hello verb with arg "tuto-0"
[00:00:00.100,000] <inf> Sleeping 1 sec....
[00:00:00.100,000] <dbg> [REQ/API tuto1] Hi debugger!
[00:00:00.101,000] <inf> [REQ/API tuto1] Hi observer! I'm tuto1 and I received hello from tuto-0
[00:00:00.101,000] <inf> [API tuto0] reply received closure=*call-1*
[00:00:00.101,000] <inf> [API tuto0] got cool status of 0
[00:00:00.101,000] <inf> [API tuto0] got data 0 as a string: Hello tuto-0!
[00:00:00.101,000] <inf> [API tuto0] leaving reply processing
[00:00:01.101,000] <inf> Wake up !
[00:00:01.101,000] <inf> API tuto1 call API tuto0 hello verb with arg "tuto-1"
[00:00:01.101,000] <inf> end of start
[00:00:01.116,000] <dbg> [REQ/API tuto0] Hi debugger!
[00:00:01.116,000] <inf> [REQ/API tuto0] Hi observer! I'm tuto0 and I received hello from tuto-1
[00:00:01.116,000] <inf> [API tuto1] reply received closure=*call-2*
[00:00:01.116,000] <inf> [API tuto1] got cool status of 0
[00:00:01.116,000] <inf> [API tuto1] got data 0 as a string: Hello tuto-1!
[00:00:01.116,000] <inf> [API tuto1] leaving reply processing
[00:00:01.598,000] <inf> ethdrv: Link up
[00:00:01.598,000] <inf> ethdrv: 100Mbps
[00:00:01.598,000] <inf> ethdrv: full duplex
```

