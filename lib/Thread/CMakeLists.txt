#===------------------------------------------------------------------------===#
#
#                     The KLEE Symbolic Virtual Machine
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===------------------------------------------------------------------------===#
klee_add_component(kleeThread
  Barrier.cpp
  BarrierManager.cpp
  Condition.cpp
  CondManager.cpp
  CondScheduler.cpp
  Mutex.cpp
  MutexManager.cpp
  MutexScheduler.cpp
  StackFrame.cpp
  StackType.cpp
  Thread.cpp
  ThreadList.cpp
  ThreadScheduler.cpp
  WaitParam.cpp
)

set(LLVM_COMPONENTS
  support
)
klee_get_llvm_libs(LLVM_LIBS ${LLVM_COMPONENTS})
target_link_libraries(kleeThread PUBLIC ${LLVM_LIBS})