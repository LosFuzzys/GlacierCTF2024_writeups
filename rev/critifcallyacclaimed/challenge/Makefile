.PHONY: dist clean server all

all: challenge 

export RUST_LOG=info

challenge:
	cd server && cargo build --features ipv4
	cp server/target/debug/critically_acclaimed_server challenge
	cd server && cargo clean && cargo build --features silent --features ipv4
	cp server/target/debug/critically_acclaimed_server challenge-dist

clean:
	${RM} challenge
	cd server && cargo clean
