# Programs

These are programs used to manage the microVMs and user function execution and result forwarding inside. They are run inside the microVMs.

## Flask web server

The Python Flask web server listens on a certain port facing the user in the real world (host machine), and upon any HTTP
GET request, executes the function it was told to execute either during its initialization or via a named POSIX FIFO
later on (by the `microDNS` manager program on the host machine, transferred by the `Vsock "server"` detailed below).

It runs the user function that was bundled as a single executable inside a subprocess, captures its `stdout` and returns
this result as JSON to the HTTP client that made the request or an error response if the subprocess failed in any way.

Upon any events, it informs the Vsock "server" via another named POSIX FIFO. These events can be successful or erroneous
function executions that are to relayed back to the manager host process for monitoring purposes. Other events are the
initial "OK" to say it launched without errors and readily listening on the network as well as "SWITCH OK" to inform on
successful change to the user function it executes.

## Vsock "server"

This program written in C, bypasses network layers and directly communicates with the host from within the microVM by
using Vsock. These are special sockets made to be used for this specific reason.

The host Linux kernel has to be compiled with the option `CONFIG_VHOST_VSOCK=m` while the guest has to have `CONFIG_VIRTIO_VSOCKETS=y` (it already does if you're using the default Firecracker tailored guest kernel config). You can use `ls /dev/vsock` inside the guest to test if support is enabled in the guest kernel. You can use `sudo modinfo vhost_vsock` to check if your host kernel has the necessary module.

This process sends updates to the `microDNS` manager program via the Vsock while also listening on incoming requests
from this same program such as to switch the function that the web server executes upon requests or to simply die.

The function switch message causes this program to inform the web server process of this change via a named POSIX FIFO
and waits for an acknowledgment to relay back to the manager host process.

The die message makes it kill the web server process and exit causing the microVM to die and Firecracker process to
terminate, thus freeing resources.

It also relays heartbeat like events to keep the managing host process informed on the microVM's state. These events are
all those mentioned in the web server part above relayed back as well as potentially the exit status of the web server
either upon expected or unexpected termination.
