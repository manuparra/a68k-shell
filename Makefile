.PHONY: build clean package run-fsuae

build:
	./scripts/build.sh

package:
	./scripts/package-workbench.sh

run-fsuae:
	./scripts/run-fsuae.sh

clean:
	rm -rf build dist
