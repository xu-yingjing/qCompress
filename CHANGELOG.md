# Changelog

## 0.1.0

Initial public release. Extracted from an in-tree application helper that needed to produce `qCompress`-compatible payloads.

- `qcompress_pack_size()` and `qcompress_pack()` keep the in-tree contract: budget-checked destination, nothing allocated, and a rejected call writes nothing.
- New for the standalone release: `README.md`, `doc/api.md`, `examples/qcompress_demo.c` and `build.sh`. The two files under `src/` are unchanged.
- Known gaps, by design: stored deflate blocks only (no compression), no decoder and no streaming interface. Output is compatible with `qUncompress()` but not byte-identical to what Qt produces.
