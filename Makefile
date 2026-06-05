.PHONY: build clean run-fsuae test

build:
	./scripts/build.sh

test:
	./scripts/test-parser.sh

run-fsuae:
	./scripts/run-fsuae.sh

clean:
	rm -rf build dist
