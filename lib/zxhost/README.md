# ZXHost target runtime and mailbox

`ZXHost` is the small C runtime/API linked into a program running on the ZX.
It publishes a request/result mailbox and a bounded text log in target memory.
It is not the Python controller or the emulator transport.

`protocol.json` is the source of truth for the header address, layout, states,
version and log range. Generate the C constants before linking a target:

```sh
python3 tools/zesarux/zxhost/generate_header.py \
  lib/zxhost/protocol.json build/zxhost_config.h
```

Add `lib/zxhost/include` and the generated-header directory to the target
include path, then link `lib/zxhost/src/zxhost.c`. The target calls
`zxhost_init()` before reading `zxhost_get_request()`. Initialization preserves
the request bytes (command, arguments, sequence and cancel) published by the
controller. The Python request writer clears cancellation for a new request.
The target may poll
`zxhost_cancelled()` cooperatively, write result bytes with
`zxhost_set_results()`, append diagnostic text with `zxhost_log_text()`, and
finish with one of `ZXHOST_DONE`, `ZXHOST_ABORTED` or `ZXHOST_ERROR`.

Commands and argument meanings belong to the target program. ZXHost provides
the byte transport contract, not a shared command registry. The sequence byte
identifies the request and is written last by the Python protocol codec; the
target preserves it in the completion header.

## Python roles

`tools/zesarux/zxhost/protocol.py` validates and encodes the mailbox descriptor.
`tools/zesarux/zxhost/manifest.py` provides typed launch, block and test-group
records
and serializes the program manifest at the JSON boundary. `tools/zesarux/zxhost/basic.py`
can build BASIC POKEs for the startup request from the same descriptor fields.
`tools/zesarux/zxhost/session.py` provides `ZXHostSession`, a host-side session
with a ZXHost target. It loads the blocks listed in a program manifest, sends an
initial command, polls for the matching sequence, reads results and can request
cooperative cancellation. `tools/zesarux/runner.py` remains the emulator
transport used internally by that session. A package that remaps the mailbox
after launch may include `runtime_memory` in its manifest; the session checks
that the mailbox and log fit the declared CPU window and bootstrap-to-runtime
transfer range.

```python
from tools.zesarux.zxhost import ZXHostSession

with ZXHostSession("build/manifest.json", "/path/to/zesarux") as session:
    session.start(command=1, args=(0, 0))
    result = session.wait_for_completion(timeout=15)
```

`start()` prepares the request and leaves the emulated CPU in CPU-step mode.
The caller may inject keys or call `request_cancel()` before or while advancing
the target, then wait for completion. Close the session through the context
manager or `close()`.
Call `detach()` only when the emulator should remain open.
