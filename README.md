KLEE-mta — A KLEEM-based and KLEE-based Verification Engine for Multithreaded Programs 
========================================================================================

#[![Build Status](https://travis-ci.com/klee/klee.svg?branch=master)](https://travis-ci.com/klee/klee)
#[![Coverage](https://codecov.io/gh/klee/klee/branch/master/graph/badge.svg)](https://codecov.io/gh/klee/klee)

`KLEE-mta` is a verification machine for multithreaded programs built on top of the LLVM compiler infrastructure. Currently, there are five primary components:

  1. The core symbolic virtual machine engine supporting multithread (pthread); this is responsible for executing LLVM bitcode modules with support for symbolic values. This is comprised of the code in lib/.

  2. A constraint encoder supporting transforming a multithread trace to constraint formulas, which symbolically captures all schedules under a fixed input.

  3. A branch explorer. Its target is to find out all input-sensitive branches and schedule-sensitive branches.(In progress)

  4. An assertion verifyer. It can verify all program properties that are expressed as assertions.

  5. A symbolic taint analyzer. It definely investigates whether a variable is tainted under the random thread shceduling.

KLEE-mta is implemented on top of KLEE 2.2 and KLEEM.

If you want to give it an easy try - follow the instructions below (mine one for Ubuntu 22.04):

  1. sudo apt install systemd docker.io docker git (reboot system if you do not have systemd installed before)
  
  2. sudo systemctl restart docker (systemctl status docker for checking availability of docker daemon, systemctl start docker for starting of docker daemon)
  
  3. git clone https://github.com/lunal1ght/klee-mta && cd klee-mta
  
  4. sudo docker build -t klee-mta .
  
  5. sudo docker run -it --rm klee-mta
  
At this point you will be in specially prepared docker container. There is some of my tests, which u can try by using next commands:

  1. cd klee_src/klee-mta-tests
  
  2. clang-9 -emit-llvm -g -c test.c -o test.bc
  
  3. klee test.bc

By now there is six tests.
  
  1. hellworld.c - based "hello world" program, only for build of KLEE testing
  
  2. return.c - main return program, nothing to test, only for build of klee testing
  
  3. sym.c - program with klee_make_symbolic, used for making tests of KLEE symbolic functionality
  
  4. sym_assert.c - program with klee_make_symbolic+klee_assert, used for making tests of KLEE symbolic+asserts functionality
  
  5. pth_assert_sym.c - program with pthread and KLEE functions usage, used for making tests of KLEE functionality with pthread
  
  6. dr_ptr_sym_as.c - program with datarace, found by klee-mta
  
