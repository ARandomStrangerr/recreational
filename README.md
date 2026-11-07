# C Service Orchestrator

A TCP message broker in C with named service registration, hash-table lookup, length-prefixed messages, and module-to-module forwarding.

**Milestone reached — 7 November 2026:** the blocking prototype forwards a message from one registered module to another through the central broker. The local demo registers `test_module_1` and `test_module_2`, looks up the destination by name using `uthash`, and delivers the payload to module 2.

This personal project explores POSIX sockets, memory ownership, message framing, event notification, and service coordination from first principles. It draws on a Java orchestration system I built for an earlier job. This repository is a separate C implementation.

## Working milestone

The current manual demo exercises the complete forwarding path:

1. The broker starts its listener, acceptor, handshake thread, and request-handler thread.
2. Two modules connect over separate TCP connections and register their names.
3. Module 1 sends a destination name, a payload length, and the payload.
4. The broker uses `HASH_FIND_STR` to retrieve the destination's registered socket.
5. Module 2 receives and prints the forwarded payload.

The receiver prints:

```text
hello 1234 from module 1
```

The demo runs the broker and both modules as pthreads in one process, with TCP connections through `127.0.0.1:8080`. It demonstrates one-way forwarding between two named modules. Bidirectional request/response correlation and multi-service workflows are planned extensions.

## Implemented components

| Component | Current behavior |
| --- | --- |
| Socket wrapper | Opaque client/server types, server creation, bind/listen/accept, outgoing connections, blocking reads/writes, and socket cleanup functions. |
| Connection acceptance | Accepts TCP clients and registers their sockets with the unverified `kqueue`. |
| Service handshake | Receives the module name and parameter-description bytes, then registers the module's name and socket. |
| Service registry | Uses `uthash` to associate module names with socket entries. |
| Event notification | Uses separate unverified and verified `kqueue` queues to notify the handshake and request-handler threads of readable sockets. |
| Message framing | Reads destination and payload fields using length prefixes. |
| Forwarding | Resolves the destination by name and sends the payload length and payload to that module. |
| Demo | Includes a basic TCP greeting exchange followed by the two-module broker demonstration. |

The socket wrapper contains IPv6 paths. The current demo exercises IPv4 TCP.

## Build and run

Requirements:

- macOS or a BSD system providing `kqueue` and `<sys/event.h>`.
- A C compiler such as Clang or GCC.
- POSIX sockets and pthreads.
- `uthash.h` available at `structure/uthash.h`.
- Port `8080` available.

From the repository root:

```sh
cc -std=c11 -Wall -Wextra -pthread \
  communication/socket.c \
  orchestrator/socket_management.c \
  main.c \
  -o socket-demo

./socket-demo
```

Compile all three source files because `main.c` now exercises the orchestrator as well as the socket wrapper. The active registry is implemented inside `socket_management.c`.

This milestone was confirmed through the author's manual local run. The demo currently uses sleeps for startup and registration timing, and `main()` returns exit status `1` even after the expected payload is printed. It is a development harness, with deterministic startup, shutdown, and automated assertions still to be added. The server binds to all IPv4 interfaces while the demo clients use loopback.

The current event backend uses `kqueue`. Linux support requires a compatible backend or a future `epoll` implementation.

## Current message format

### Registration

A connecting module sends:

1. A `soc_management_handshake_header` containing `module_name_length` and `module_params_length`.
2. Exactly that many module-name bytes.
3. Exactly that many parameter-description bytes.

The receiver appends local string terminators. Parameter descriptions are received during registration; parameter-driven routing remains future work.

### Module-to-broker request

The working demo uses this sequence:

| Field | Representation |
| --- | --- |
| Destination-name length | Native C `size_t` |
| Destination name | The stated number of bytes, excluding a terminating NUL |
| Payload length | Native C `size_t` |
| Payload | The stated number of bytes |

### Broker-to-module delivery

The broker sends the payload length as a native `size_t`, followed by the payload bytes. The receiving demo appends `'\0'` before printing its text payload.

The implementation sends native C representations directly. A portable wire format with fixed-width fields, defined byte order, and versioning is future work. The declared `soc_management_header` with correlation and size fields is an earlier protocol proposal; the current demonstration uses the framing above.

## Current limitations

- **Blocking transfers.** Socket reads and writes loop over partial transfers until the requested byte count is reached. A client that stalls mid-message, or a destination that stops reading, can hold up the thread handling it.
- **Validation and failure handling.** Length limits, allocation checks, read/write-result checks, and missing-destination handling need to be completed before arbitrary input is supported.
- **Registration timing and synchronization.** The demo uses a delay before sending to the second module. Registration acknowledgements and synchronization of registry access across threads remain to be implemented.
- **Connection lifecycle.** Disconnect handling, registry removal, string ownership, and orderly thread shutdown need further work. The current handshake EOF path still frees a socket before passing it to the queue-removal helper.
- **Scope of validation.** The milestone is a manual local forwarding demonstration. Throughput, multi-machine operation, and resilience under concurrent or malformed traffic have not been measured.
- **Planned capabilities.** TLS, authentication, replica selection, round-robin balancing, retries, delivery guarantees, and automatic response routing remain future work.

## Next milestones

1. Add basic logging for registration, lookup, forwarding, disconnects, and system-call failures.
2. Complete input validation, error handling, and connection cleanup.
3. Add persistent per-connection receive state and nonblocking reads.
4. Queue outgoing data and resume partial writes on write-readiness events.
5. Enforce per-client handshake deadlines and replace sleep-based coordination with explicit readiness.
6. Add repeatable tests covering partial messages, invalid lengths, unavailable destinations, disconnects, and slow clients.
7. Define portable framing, correlation IDs, and request/response behavior.

## Source layout

| File | Purpose |
| --- | --- |
| [`communication/socket.h`](communication/socket.h) | Public socket API and opaque types. |
| [`communication/socket.c`](communication/socket.c) | POSIX socket operations and blocking transfer loops. |
| [`orchestrator/socket_management.h`](orchestrator/socket_management.h) | Management API and protocol declarations. |
| [`orchestrator/socket_management.c`](orchestrator/socket_management.c) | Acceptor, handshake, `kqueue` watches, service registry, and forwarding handler. |
| [`structure/uthash.h`](structure/uthash.h) | Hash-table macros used for module-name lookup. |
| [`main.c`](main.c) | Threaded socket and module-to-module demonstrations. |

