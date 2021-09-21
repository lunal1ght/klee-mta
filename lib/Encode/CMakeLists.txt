#===------------------------------------------------------------------------===#
#
#                     The KLEE Symbolic Virtual Machine
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===------------------------------------------------------------------------===#
klee_add_component(kleeEncode
  BitcodeListener.cpp
  BarrierInfo.cpp
  DTAM.cpp
  DTAMPoint.cpp
  Encode.cpp
  Event.cpp
  FilterSymbolicExpr.cpp
  KQuery2Z3.cpp
  ListenerService.cpp
  Prefix.cpp
  PSOListener.cpp
  RuntimeDataManager.cpp
  SymbolicListener.cpp
  TaintListener.cpp
  Trace.cpp
  Transfer.cpp
)

set(LLVM_COMPONENTS
  support
)
klee_get_llvm_libs(LLVM_LIBS ${LLVM_COMPONENTS})
target_link_libraries(kleeEncode PUBLIC ${LLVM_LIBS})