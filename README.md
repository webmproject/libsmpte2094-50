# libsmpte2094_50

This is not an officially supported Google product. This project is not
eligible for the [Google Open Source Software Vulnerability Rewards
Program](https://bughunters.google.com/open-source-security).

This project includes utilities to use the SMPTE 2094-50 specification.

## Build instructions

Just clone and use CMake to build. You need
[Cargo](https://doc.rust-lang.org/cargo/) on your path.

```sh
git clone https://github.com/webmproject/libsmpte2094-50.git
cmake -S libsmpte2094-50 -B libsmpte2094-50/build
cmake --build libsmpte2094-50/build --config Release --parallel
```

## Formatting

The `rust_lib` directory contains a `rustfmt.toml` file configured with unstable nightly options (such as `wrap_comments` and `style_edition`). To format the Rust library from the repository root, you must run `rustfmt` via a nightly toolchain or with unstable features enabled:

```sh
# Using a nightly toolchain
cargo +nightly fmt --manifest-path rust_lib/Cargo.toml

# Or by explicitly enabling unstable features
cargo fmt --manifest-path rust_lib/Cargo.toml -- --unstable-features
```

## Release instructions

Before doing a gitag and release, you need to upgrade the Cargo.toml:

```sh
cargo upgrade --incompatible
cargo publish
```
