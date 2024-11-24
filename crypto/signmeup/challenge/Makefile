.PHONY: dist clean

challenge:
	cargo build --release
	cp target/release/signmeup ./challenge

clean:
	${RM} challenge target/ compile_commands.json
