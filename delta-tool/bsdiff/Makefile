CFLAGS		+= -O0 -Wall -pedantic -g -ggdb -Wextra -std=gnu99 -Wmissing-prototypes -Wstrict-prototypes -ldl -DLZ4_MEMORY_USAGE=10 -Werror=implicit-function-declaration -Werror=return-type
#
bsdiff:	bsdiff.c ../source/varint.c lz4.o
	${CC} ${CFLAGS} -D BSDIFF_EXECUTABLE $^ -o $@ -I../include

bspatch: bspatch_main.c ../source/bspatch.c ../include/bspatch.h ../include/bspatch_private.h lz4.o ../source/varint.c
	${CC} ${CFLAGS} $^ -o $@ -I../include -DBS_PATCH_COMPILE_TIME_MEMORY_ALLOC=1024

bspatch_dynamic_alloc: bspatch_main.c ../source/bspatch.c ../include/bspatch.h ../include/bspatch_private.h lz4.o ../source/varint.c
	${CC} ${CFLAGS} $^ -o $@ -I../include -DBS_PATCH_COMPILE_TIME_MEMORY_ALLOC=0

bspatch_compile_time_memory1024: bspatch_main.c ../source/bspatch.c ../include/bspatch.h ../include/bspatch_private.h lz4.o ../source/varint.c
	${CC} ${CFLAGS} $^ -o $@ -I../include -DBS_PATCH_COMPILE_TIME_MEMORY_ALLOC=1024

bspatch_compile_time_memory131072: bspatch_main.c ../source/bspatch.c ../include/bspatch.h ../include/bspatch_private.h lz4.o ../source/varint.c
	${CC} ${CFLAGS} $^ -o $@ -I../include -DBS_PATCH_COMPILE_TIME_MEMORY_ALLOC=131072


bspatch_compile_time_memory131072test: bspatch_compile_time_memory131072
	rm -f test-folder/*
	@echo "******"
	@echo "Test 9"
	@echo "******"
	./bsdiff ./test_data/rootfs-yocto-original-8.tar ./test_data/rootfs-yocto-new-8.tar ./test-folder/raspi.bsdiff 65536
	./bspatch_compile_time_memory131072 ./test_data/rootfs-yocto-original-8.tar ./test-folder/raspi_image.tar ./test-folder/raspi.bsdiff
	diff ./test_data/rootfs-yocto-new-8.tar ./test-folder/raspi_image.tar
	
	@echo "Test 6: app update bspatch frame 65536"
	@echo "******"
	./bsdiff ./test_data/app_update_old.bin ./test_data/app_update_new.bin ./test-folder/app_update_diff_65536.bsdiff 65536
	./bspatch_compile_time_memory131072 ./test_data/app_update_old.bin ./test-folder/app_update_patched_65536.bin ./test-folder/app_update_diff_65536.bsdiff
	diff ./test_data/app_update_new.bin ./test-folder/app_update_patched_65536.bin
	#lz4 ./test-folder/app_update_diff_65536.bsdiff test-folder/app_update_diff_65536.bsdiff.lz4
	#lz4 ./test-folder/app_update_patched_65536.bin test-folder/app_update_patched_65536.bin.lz4
	
lz4.o: ../source/lz4.c ../include/lz4.h
	${CC} -c ../source/lz4.c -I../include -DLZ4_MEMORY_USAGE=10

#varint.o: ../source/varint.c ../include/varint.h
#	${CC} -c ../source/varint.c -I../include

.PHONY: test
test: bsdiff bspatch just-test

.PHONY: test_dynamic_alloc
test_dynamic_alloc: bsdiff bspatch bspatch_dynamic_alloc justdynamictest

#these test now run with default framesize of 512B
.PHONY: just-test
just-test:
	rm -f test-folder/*
	mkdir -p test-folder
	
	
	@echo "*********************************"	
	ls -la test-folder	
	
	@echo "******"
	@echo "Test 1: simple 1"
	@echo "******"
	./bsdiff ./test_data/simple1.txt ./test_data/simple1_new.txt ./test-folder/simple1_diff.bsdiff
	./bspatch ./test_data/simple1.txt ./test-folder/simple1_patched.bin ./test-folder/simple1_diff.bsdiff
	diff ./test_data/simple1_new.txt ./test-folder/simple1_patched.bin
	
	@echo "******"
	@echo "Test 3: string tweak"
	@echo "******"
	./bsdiff ./test_data/string_tweak_old.bin ./test_data/string_tweak_new.bin ./test-folder/string_tweak_diff.bsdiff
	./bspatch ./test_data/string_tweak_old.bin ./test-folder/string_tweak_patched.bin ./test-folder/string_tweak_diff.bsdiff
	diff ./test_data/string_tweak_new.bin ./test-folder/string_tweak_patched.bin
	lz4 ./test-folder/string_tweak_diff.bsdiff test-folder/string_tweak_diff.bsdiff.lz4
	lz4 ./test-folder/string_tweak_patched.bin test-folder/string_tweak_patched.bin.lz4
	
	@echo "******"
	@echo "Test 4: driver add"
	@echo "******"
	./bsdiff ./test_data/driver_add_old.bin ./test_data/driver_add_new.bin ./test-folder/driver_add_diff.bsdiff
	./bspatch ./test_data/driver_add_old.bin ./test-folder/driver_add_patched.bin ./test-folder/driver_add_diff.bsdiff
	diff ./test_data/driver_add_new.bin ./test-folder/driver_add_patched.bin
	lz4 ./test-folder/driver_add_diff.bsdiff test-folder/driver_add_diff.bsdiff.lz4
	lz4 ./test-folder/driver_add_patched.bin test-folder/driver_add_patched.bin.lz4
	
	@echo "******"
	@echo "Test 5: app update bs patch frame 512"
	@echo "******"
	./bsdiff ./test_data/app_update_old.bin ./test_data/app_update_new.bin ./test-folder/app_update_diff.bsdiff
	./bspatch ./test_data/app_update_old.bin ./test-folder/app_update_patched.bin ./test-folder/app_update_diff.bsdiff
	diff ./test_data/app_update_new.bin ./test-folder/app_update_patched.bin
	lz4 ./test-folder/app_update_diff.bsdiff test-folder/app_update_diff.bsdiff.lz4
	lz4 ./test-folder/app_update_patched.bin test-folder/app_update_patched.bin.lz4
	

	
	@echo "Test 7"
	@echo "******"
	./bsdiff ./test_data/old1.bin ./test_data/new1.bin ./test-folder/diff1.bsdiff
	./bspatch ./test_data/old1.bin ./test-folder/patched1.bin ./test-folder/diff1.bsdiff
	diff ./test_data/new1.bin ./test-folder/patched1.bin
	#do check how much smaller it could have been
	lz4 ./test-folder/diff1.bsdiff test-folder/diff1.bsdiff.lz4
	lz4 ./test-folder/patched1.bin test-folder/patched1.bin.lz4
	
	@echo "Test 8"
	@echo "******"
	./bsdiff ./test_data/old2.bin ./test_data/new2.bin ./test-folder/diff2.bsdiff
	./bspatch ./test_data/old2.bin ./test-folder/patched2.bin ./test-folder/diff2.bsdiff
	diff ./test_data/new2.bin ./test-folder/patched2.bin
	#do check how much smaller it could have been
	lz4 ./test-folder/diff2.bsdiff test-folder/diff2.bsdiff.lz4
	lz4 ./test-folder/patched2.bin test-folder/patched2.bin.lz4
	
	#do check how much smaller it could have been
	#lz4 ./test-folder/diff2.bsdiff test-folder/diff2.bsdiff.lz4
	#lz4 ./test-folder/patched2.bin test-folder/patched2.bin.lz4
	
	@echo "*********************************"	
	ls -la test-folder	

.PHONY: clean
clean:
	rm -f bsdiff
	rm -f bspatch
	rm -f test-folder/*
	rm -f *.o

.PHONY: all
all: bsdiff bspatch

.PHONY: lines
lines:
	cloc . --by-file

.PHONY: size
size: all
	nm --print-size --size-sort bspatch
	ldd bspatch
	#size -t `ldd bspatch | awk '/=> \// {print $3;}'`
