# llext-module

This example shows the load of 2 LLEXT bindings.

LLEXT are the [Linkable Loadable Extensions][1] and are for Zephr OS
equivalent to dynamic libraries of rich operating systems.

Zephyr loads the LLEXT modules in RAM and solve in it the required
address. It also allows the loader to discover address exported by
the LLEXT.

This features allows direct implementation of redpesk bindings with
their strict isolation of the framework without change in sources.

The sample is divided in 4 main parts, each being in a separate
subdirectory:

- sdk: this part is used for generation of the SDK
- ext1: The llext 1, compiled against the SDK
- ext2: The llext 2, compiled against the SDK
- app: the application, it loads the extension and publish it on network

A fith directory is created during build for holding the SDK when compiling
extensions.

The schema below summarizes the process.

```
    sdk
     |
     | produce
     V
  afb-sdk
     |
     | used for compiling
    / \
   /   \
  /     \
  V     V
 ext1  ext2
   \   /
    \ /
     V
     | included in
     V
    app
```

That build process is encoded in the file `build-all.sh`.
This script accept several options as follows:
```bash
Usage: ./build-all.sh [options]
        -h, --help: display help
        -b, --board <board_string>: target board                       [default: b_u585i_iot02a --shield link_board_eth
        -z, --zephyr-path <path>: path to the zephyr kernel            [default: ~/zephyrproject/zephyr]
        -s, --sdk-path <path>: path to the zephyr SDK                  [default: ~/zephyr-sdk-0.16.8]
```

Then, flash the app as you would usually do for your target board:
```bash
# example for b_u585i_iot02a with the shield link_board_eth
cd app/build
west flash -r openocd
```

The main application, app, exports using RPC protocol,
the API ext1 of the LLEXT ext1 on the port 1111 and the
API ext2 of LLEXT ext2 on port 1112.

The API ext1 has only one verb, hello, that replies hello
to requests.

The API ext2 has only one verb, hello, that replies hello
to requests. But it can do it either directly when DIRECT=1
or by invoking API ext1 when DIRECT=0. 

A typical output of the board, using picocom is:

```
*** Booting Zephyr OS build v4.0.0-65-ge397b1b65790 ***
[00:00:00.003,000] <inf> net_dhcpv4_client_sample: Run dhcpv4 client
[00:00:00.003,000] <inf> net_dhcpv4_client_sample: Start on enc424j600@0: index=1
[00:00:01.591,000] <inf> ethdrv: Link up
[00:00:01.591,000] <inf> ethdrv: 100Mbps
[00:00:01.591,000] <inf> ethdrv: full duplex
[00:00:04.596,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:00:04.626,000] <inf> net_dhcpv4_client_sample: DHCP Option 42: 10.18.253.1
[00:00:04.626,000] <inf> net_dhcpv4: Received: 10.18.127.29
[00:00:04.627,000] <inf> net_dhcpv4_client_sample:    Address[1]: 10.18.127.29
[00:00:04.627,000] <inf> net_dhcpv4_client_sample:     Subnet[1]: 255.255.0.0
[00:00:04.627,000] <inf> net_dhcpv4_client_sample:     Router[1]: 10.18.253.252
[00:00:04.627,000] <inf> net_dhcpv4_client_sample: Lease time[1]: 1800 seconds
[00:00:04.627,000] <inf> API ROOT added
[00:00:04.627,000] <inf> Entering start
[00:00:04.627,000] <inf> binding [ext1] looks like an AFB binding V4
[00:00:04.627,000] <inf> API ext1 added
[00:00:04.628,000] <inf> [API ext1] ext1 in pre-init
[00:00:04.628,000] <inf> binding [ext2] looks like an AFB binding V4
[00:00:04.628,000] <inf> API ext2 added
[00:00:04.628,000] <inf> [API ext2] ext2 in pre-init
[00:00:04.628,000] <inf> Adding RCP server ...
[00:00:04.629,000] <inf> end of start
[00:00:04.629,000] <inf> API ROOT starting...
[00:00:04.629,000] <inf> API ROOT started
[00:00:04.629,000] <inf> API ext1 starting...
[00:00:04.629,000] <inf> [API ext1] ext1 in init
[00:00:04.629,000] <inf> API ext1 started
[00:00:04.629,000] <inf> API ext2 starting...
[00:00:04.629,000] <inf> [API ext2] ext2 in init
```

Then take the IP of the board for launching `afb-binder`.
Because `afb-client` has no direct support of RPC protocol,
`afb-binder` should be launched for relaying:

```
afb-binder -p 7777 --rpc-client tcp:10.18.127.29:1111/1 --rpc-client tcp:10.18.127.29:1112/2
```

It declares 2 API: simply named 1 and 2. It can then be invoked
using `afb-client`:

```
> afb-client localhost:7777/api
2 hello truc machin chouette
ON-REPLY 1:2/hello: OK
{"jtype":"afb-reply","request":{"status":"success","code":0},"response":"Hello truc machin chouette!"}
1 hello c'est moi
ON-REPLY 2:1/hello: OK
{"jtype":"afb-reply","request":{"status":"success","code":0},"response":"Hello c'est moi!"}
```

And on the log output of the board:

```
[00:06:30.515,000] <dbg> [REQ/API ext2] Hi debugger!
[00:06:30.515,000] <inf> [REQ/API ext2] Hi observer! I'm ext2 and I received hello from truc machin chouette
[00:06:30.515,000] <inf> [REQ/API ext2] Calling ext 1
[00:06:30.515,000] <dbg> [REQ/API ext1] Hi debugger!
[00:06:30.515,000] <inf> [REQ/API ext1] Hi observer! I'm ext1 and I received hello from truc machin chouette
[00:06:30.516,000] <inf> [REQ/API ext1] replying "Hello truc machin chouette!"
[00:06:30.516,000] <inf> [REQ/API ext2] reply of ext 1
[00:06:56.349,000] <dbg> [REQ/API ext1] Hi debugger!
[00:06:56.349,000] <inf> [REQ/API ext1] Hi observer! I'm ext1 and I received hello from c'est moi
[00:06:56.349,000] <inf> [REQ/API ext1] replying "Hello c'est moi!"
```

## On writing LLEXT extension for redpesk µbinder framework

This [repository][2] shows examples of integration with the µbinder
that are in the same address space and environment than the µbinder.
It is so tightly coupled that these implementations are calling directly
the internal functions of the µbinder.

With LLEXT, it changes radically because it uses the same interface
that the binding coded as dynamic libraries: the address space is really
separated and the interaction is restricted to the precise interface
defined by *afb-binding*.

It has the following implications: standard include and, at the moment,
explicit export of some symbols.

### Standard include

As when writing bindings for linux, the code of binding should just include
the below lines:

```
#define AFB_BINDING_VERSION 4
#include <afb/afb-binding.h>
```

### Exporting symbols

Under linux, when creating a dynamic library, all the public symbols
are exported. And if required, the framwork provides version script for
the linker that only exports the symbols needed by the framework.

That is not the case with Zephyr. So nowadays, the code of the LLEXT binding
must add declarations for exporting the symbols to the part of the µbinder
that loads the LLEXT.

The code below shows the mandatory lines to export it:

```
#include <zephyr/llext/symbol.h>
LL_EXTENSION_SYMBOL(afbBindingRoot);
LL_EXTENSION_SYMBOL(afbBindingV4r1_itfptr);
LL_EXTENSION_SYMBOL(afbBindingV4_itf_revision);
```

It is also need to add one of the below lines or the two.

```
LL_EXTENSION_SYMBOL(afbBindingExport);
LL_EXTENSION_SYMBOL(afbBindingEntry);
```

Framework help page about globals ([see][3]) explains the
symbols **afbBindingRoot**, **afbBindingExport** and **afbBindingEntry**.

Symbols **afbBindingV4r1_itfptr** and **afbBindingV4_itf_revision**
are internally used for exporting the framework interface and checking
its compatibility.

## Notes and remarks

The example works on `b_u585i_iot02a` with the shield `link_board_eth`.

Some more experiments have to be done for exporting system symbols to extensions.
During our work, for simplifying things, we used the function *append* instead
of *snprintf* in order to not depend on any library. **feedback welcome on how
to export system symbols to LLEXT when creating a SDK**.

On that board, *b\_u585i\_iot02a*, there is no activation of USERSPACE
because west/zephyr complains that **ARCH\_HAS\_USERSPACE=n** and
**SRAM\_REGION\_PERMISSIONS=n**. In my humble opinion, this is
a Zephyr OS error because the MCU STM32U585 has these features.

However using the board **nucleo_h755zi_q** that supports *USERSPACE*,
the integration of LLEXT in userspace requires more work on how to
manage memory. Fact is that in that case, the memory allocator has no
mamory to provide in userspace. So a setting is missing, a setting
that may depend on integration choices.


[1]: https://docs.zephyrproject.org/latest/services/llext/index.html
[2]: https://github.com/redpesk-samples/afb-zephyr-samples
[3]: https://docs.redpesk.bzh/docs/en/master/developer-guides/reference-v4/types-and-globals.html
