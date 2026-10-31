# Recreational: a C service orchestrator

A personal project exploring socket programming, message framing, and service orchestration in C.

I enjoy building things from the underlying pieces. This project gives me a reason to work through connections, memory ownership, event notification, and request routing myself. The idea comes from a Java orchestration system I built for an earlier job; this repository is a separate C experiment.

The goal is for services to connect to a central process, register their names and required parameters, and receive requests addressed to them. Routing a request through multiple services is part of the intended design.

**Current status:** the TCP client/server example runs. The orchestrator is still being built, with partial handshake code, a proposed service registry, and placeholders for request handling and routing.

## What is here

| Component | Current state |
| --- | --- |
| Socket wrapper | Server creation, bind/listen/accept, outgoing connections, blocking reads and writes, and socket cleanup. Uses opaque socket types. |
| TCP example | A server and client run in separate pthreads and exchange a fixed greeting over IPv4. |
| Connection management | Code for accepting connections and watching readable sockets with `kqueue`. Separate queues represent connections before and after registration. |
| Service handshake | A partial parser reads a service name and parameter description. Registration is not connected into a working application yet. |
| Service registry | An entry structure and function declarations exist. Storage, lookup, removal, and ownership rules still need implementation. |
| Request routing | A request header is declared. The router and request handler are unfinished. |

The socket wrapper also contains IPv6 paths. The included example exercises IPv4 TCP only.

## Run the socket example

Requirements:

- A C compiler such as Clang or GCC.
- POSIX sockets and pthreads.
- Port `8080` available.

From the repository root:

```sh
cc -std=c11 -Wall -Wextra -pthread \
  communication/socket.c main.c \
  -o socket-demo

./socket-demo
```

This builds the socket wrapper and the example without compiling the unfinished orchestrator. The command was checked on Linux: compilation succeeded with unused-variable/parameter warnings, and the client received the greeting.

The program starts a server thread, waits one second, starts a client that connects to `127.0.0.1:8080`, and prints output containing:

```text
Received from server: Hello from server
```

The example currently returns exit status `1` even after a successful exchange because `main()` ends with `return 1;`. It also uses a sleep for startup coordination, so it is a small demonstration rather than a reliable automated test. The server binds to all IPv4 interfaces, although the example client connects through loopback.

### Full build status

The supplied Makefile includes `orchestrator/socket_management.c`. That file directly uses `<sys/event.h>` and `kqueue`, targeting macOS/BSD APIs. A Linux `epoll` implementation is not present, despite comments in the header mentioning it.

The orchestrator also calls `soc_storage_append`, which currently has a declaration but no implementation. The full build needs that implementation before it can link. Use the command above to run the existing socket example.

## Source layout

| File | Purpose |
| --- | --- |
| [`communication/socket.h`](communication/socket.h) | Public socket API and opaque types. |
| [`communication/socket.c`](communication/socket.c) | POSIX socket operations and blocking byte-transfer loops. |
| [`orchestrator/socket_management.h`](orchestrator/socket_management.h) | Management API and proposed request header. |
| [`orchestrator/socket_management.c`](orchestrator/socket_management.c) | Acceptor, `kqueue` registration, partial handshake, and routing placeholders. |
| [`orchestrator/socket_storage.h`](orchestrator/socket_storage.h) | Proposed registry entry containing a socket, service name, and parameter description. |
| [`main.c`](main.c) | Threaded TCP greeting example. |

## Protocol work in progress

The handshake code currently expects two native C `int` lengths, followed by the service-name bytes and parameter-description bytes. It adds string terminators locally after reading those fields.

The proposed request header contains three `uint32_t` fields: `correlation_id`, `target_service_size`, and `payload_size`. The encoding of multiple target services, payload handling, and response routing still need to be defined and implemented.

This is an experimental format. The handshake currently reads a native struct directly, with no byte-order conversion or protocol versioning. It is not yet a portable wire protocol.

## Current limitations

- `soc_read` and `soc_write` loop over partial transfers, but remain blocking. A readable-socket notification does not guarantee that an entire handshake has arrived.
- Handshake length limits and read-result checks are unfinished. The `kevent` wait timeout does not enforce a deadline for each connection.
- Registry allocation and initialization are missing. The handshake function currently returns immediately while the registry pointer is null.
- Disconnect cleanup needs correction: one path frees a socket before passing it to the queue-removal helper. String ownership and synchronization also need to be settled before registry integration.
- TLS, authentication, load balancing, retries, and delivery guarantees are not implemented. The current example does not demonstrate distributed request processing or throughput scaling.

## Next steps

1. Implement the service registry with explicit ownership of sockets and strings.
2. Complete registration and demonstrate one request and response between a client and a named service.
3. Define request framing, correlation handling, and the representation of multiple target services.
4. Replace blocking handshake reads with incremental connection state, validate lengths, and handle deadlines and disconnects.
5. Add repeatable tests for fragmented messages, invalid lengths, disconnects, and concurrent clients.
