# Targets for non-devs (who must supply dependencies themselves)
build: build-nodev
build-nodev: CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release
install: install-nodev

# Targets for devs and CI (dependencies fetched via CPM.cmake)
configure-release: CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release -DGREETER_DEV=ON
configure-debug: CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug -DGREETER_DEV=ON -DGREETER_COVERAGE=ON

configure-%: phony
	cmake $(CMAKE_FLAGS) -S. -B build-$*

build-%: configure-% phony
	cmake --build build-$*

watch-%: phony
	find CMakeLists.txt src include test -type f | entr -r -s 'make check-$*'

check-%: build-% phony
	ctest --test-dir build-$* --output-on-failure ${SELECTOR}

coverage-%: CMAKE_FLAGS+=-DGREETER_COVERAGE=ON
coverage-%: check-%
	./build-debug/Greeter
	./build-debug/Greeter -l martian || true
	./build-debug/Greeter -l en
	./build-debug/Greeter -l de
	./build-debug/Greeter -l es
	./build-debug/Greeter -l fr
	./build-debug/Greeter --help
	./build-debug/Greeter --version
	find build-$* -type f -iname "*.gcda" | xargs gcov -s $$PWD -r -t

run-%: configure-% phony
	cmake --build build-$* --target GreeterExec
	build-$*/Greeter --version

clean: phony
	rm -rf build*

doc: phony
	cmake -DGREETER_DOCS=ON -S. -B build-docs
	cmake --build build-docs --target GreeterDocs

compile_commands.json: configure-debug
	ln -sf build-debug/compile_commands.json compile_commands.json

check-format: phony
	find src include test -type f | xargs clang-format --dry-run --Werror

fix-format: phony
	find src include test -type f | xargs clang-format -i

.PHONY: phony
