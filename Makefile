export CPM_SOURCE_CACHE=${HOME}/.cache/CPM
export CPM_USE_NAMED_CACHE_DIRECTORIES=true
CMAKE_FLAGS = -DUSE_CCACHE=YES \
              -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build-release: CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"
build-debug: CMAKE_FLAGS+=-DUSE_SANITIZER='Address;Undefined' -DGREETER_TEST_COVERAGE=1

build-%:
	cmake $(CMAKE_FLAGS) -S. -B build-$*

watch-%:
	find src include test -type f | entr -r -s 'make check-$*'

check-%: build-%
	cmake --build build-$* --target GreeterTests --target test

run-%: build-%
	cmake --build build-$* --target GreeterExec
	build-$*/Greeter --version

install-%: build-%
	cmake --build build-$* --target install

build: build-release
install: install-release

clean:
	rm -rf build*

# FIXME: very questionable if I need cmake for formatting or docs
check-format: build-debug
	cmake --build build-debug --target check-format

fix-format: build-debug
	cmake --build build-debug --target fix-format

doc:
	cmake -DGREETER_LIB=OFF -DGREETER_INSTALL=OFF \
              -DGREETER_DOCS=ON -S. -B build-docs
	cmake --build build-docs --target GreeterDocs

compile_commands.json: build-debug
	ln -sf build-debug/compile_commands.json compile_commands.json

.PHONY: clean all doc check-format fix-format
.SECONDARY: build-release build-debug
