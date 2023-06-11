export CPM_SOURCE_CACHE=${HOME}/.cache/CPM
export CPM_USE_NAMED_CACHE_DIRECTORIES=true
CMAKE_FLAGS = -DUSE_CCACHE=YES -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

all: release debug coverage

build-release: CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"
build-debug: CMAKE_FLAGS+=-DUSE_SANITIZER='Address;Undefined'
build-coverage: CMAKE_FLAGS+=-DENABLE_TEST_COVERAGE=1

build-%:
	cmake $(CMAKE_FLAGS) -Scmake/all -B build-$*

compile_commands.json: build-debug
	ln -sf build-debug/compile_commands.json compile_commands.json

watch-%:
	find src include test -type f | entr -r -s 'make check-$*'

check-%: build-%
	cmake --build build-$* --target GreeterTests --target test

check-format: build-debug
	cmake --build build-debug --target check-format

fix-format: build-debug
	cmake --build build-debug --target fix-format

doc: build-debug
	cmake --build build-debug --target GenerateDocs

greeter-%: build-%
	cmake --build build-$* --target GreeterExec
	build-$*/greeter/Greeter --version

clean:
	rm -rf build*

.PHONY: clean all watch doc
.SECONDARY: build-release build-debug build-coverage
