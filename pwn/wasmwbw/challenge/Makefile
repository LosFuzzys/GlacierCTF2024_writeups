.PHONY: build debug all challenge

build: wasm_host/src/main.rs
	cd wasm_host && cargo build --release && cd ..
	cp wasm_host/target/release/wasm_host challenge

debug: wasm_host/src/main.rs
	cd wasm_host && cargo build && cd ..

all: build
challenge: build
