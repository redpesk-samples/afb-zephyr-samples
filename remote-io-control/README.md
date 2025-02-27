# remote-io-control

This example shows implementation of an api exposed to public
that controls a led and a button.

The API is named `zephyr`. It has 5 verbs:

- `led` for getting or setting led state
- `button` for getting button state
- `edge` for getting or setting the edge of the button transition
   that raise an event
- `state` for getting the state
- `events` for subscribing to events for led and/or button

Here some example of commands

| command                 | explanation |
|-------------------------|-------------|
| `zephyr led`            | get state of the led |
| `zephyr led on`         | set led on |
| `zephyr led off`        | set led off |
| `zephyr button`         | get state of the button |
| `zephyr edge`           | get expected edge |
| `zephyr edge falling`   | set expecting falling edge |
| `zephyr edge rising`    | set expecting rising edge |
| `zephyr edge any`       | set expecting any edge |
| `zephyr state`          | get current state |
| `zephyr events +led`    | add expecting led events |
| `zephyr events -led`    | remove expecting led events |
| `zephyr events +button` | add expecting button events |
| `zephyr events -button` | remove expecting button events |
| `zephyr events +all`    | add expecting any events |
| `zephyr events -all`    | remove expecting any events |

Be cautious because falling/rising edges are counter intuitives because
the signal level is high when the button is down.

This example requires a board with a led, a button and an ethernet link.

## Example of compiling

For the board B-U585I-IOT02A with the PHYTEC hat link board ETH:

```
west build -b b_u585i_iot02a --shield link_board_eth -p always
west flash -r openocd
```

## Running the demo

At least 3 terminals are to be used:

- one for showing output of the board
- one containing the binder linked to the board and acting as a proxy
- one for the client for sending interactive commands.

### Output of the board

The logging messages of the board can be seen by using terminal
emulator.

With picocom:

```
picocom -b 115200 /dev/ttyACM0
```

With minicom:

```
minicom -b 115200 /dev/ttyACM0
```

The board will prompt the below messages at boot:

```
*** Booting Zephyr OS build v4.0.0-65-ge397b1b65790 ***
[00:00:00.052,000] <inf> net_dhcpv4_client_sample: Run dhcpv4 client
[00:00:00.052,000] <inf> net_dhcpv4_client_sample: Start on ethernet@40028000: index=1
[00:00:03.153,000] <inf> phy_mii: PHY (0) Link speed 100 Mb, full duplex
[00:00:15.902,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:05:02.645,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:05:33.937,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:05:33.940,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:05:33.940,000] <inf> net_dhcpv4: Received: 10.18.127.29
[00:05:33.940,000] <inf> net_dhcpv4_client_sample:    Address[1]: 10.18.127.29
[00:05:33.940,000] <inf> net_dhcpv4_client_sample:     Subnet[1]: 255.255.0.0
[00:05:33.940,000] <inf> net_dhcpv4_client_sample:     Router[1]: 10.18.253.252
[00:05:33.940,000] <inf> net_dhcpv4_client_sample: Lease time[1]: 1800 seconds
[00:05:33.940,000] <inf> Set up button at gpio@58020800 pin 13
[00:05:33.940,000] <inf> API ROOT added
[00:05:33.940,000] <inf> Entering start
[00:05:33.940,000] <inf> Adding API ...
[00:05:33.940,000] <inf> API zephyr added
[00:05:33.940,000] <inf> Adding RCP server ...
[00:05:33.941,000] <inf> end of start
[00:05:33.941,000] <inf> API ROOT starting...
[00:05:33.941,000] <inf> API ROOT started
[00:05:33.941,000] <inf> API zephyr starting...
[00:05:33.941,000] <inf> API zephyr started
...
```

Once getting its DHCP address, the board will prompt it.
This must be used for connecting to it with the binder.


### Running the binder as a proxy

The binder must be run using the below command:

```
afb-binder -vvv --traceevt=common --tracereq=common --rpc-client=tcp:ADDRESS:1234/zephyr
```

Where ADDRESS is the IP address of the board.

The options `--tracereq` and `--traceevt` are for showing things happening in the binder.

For binder options refer to [binder's man page][1]

### Running the interactive client

This is done as usual:

```
afb-client localhost:1234/api
```


[1]: https://docs.redpesk.bzh/docs/en/master/redpesk-os/afb-binder/afb-binder.1.html


