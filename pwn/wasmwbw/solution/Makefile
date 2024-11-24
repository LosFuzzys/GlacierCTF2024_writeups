.PHONY: zig wat all

zig: payload.zig
	zig build-exe payload.zig -target wasm64-freestanding-none -fno-entry -fno-lto --export=entry -O ReleaseSafe
	base64 payload.wasm -w 0 > payload.wasm.base64

wat: payload.wat
	wat2wasm --enable-memory64 payload.wat > payload.wasm
	base64 payload.wasm -w 0 > payload.wasm.base64

all: zig
