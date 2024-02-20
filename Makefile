build-release: CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"
build-debug: CMAKE_FLAGS+=-DUSE_SANITIZER='Address;Undefined' -DGREETER_COVERAGE=1

build: build-release
install: install-release

configure-%: phony
	cmake $(CMAKE_FLAGS) -S. -B build-$*

build-%: configure-% phony
	cmake --build build-$*

watch-%: phony
	find src include test -type f | entr -r -s 'make check-$*'

check-%: configure-% phony
	cmake --build build-$* --target GreeterTests --target test

run-%: configure-% phony
	cmake --build build-$* --target GreeterExec
	build-$*/Greeter --version

install-%: configure-% phony
	cmake --build build-$* --target install

clean: phony
	rm -rf build*

# FIXME: very questionable if I need cmake for formatting or docs
check-format: build-debug
	cmake --build build-debug --target check-format

fix-format: build-debug
	cmake --build build-debug --target fix-format

doc: phony
	cmake -DGREETER_LIB=OFF -DGREETER_INSTALL=OFF \
              -DGREETER_DOCS=ON -S. -B build-docs
	cmake --build build-docs --target GreeterDocs

compile_commands.json: build-debug
	ln -sf build-debug/compile_commands.json compile_commands.json

.PHONY: phony

