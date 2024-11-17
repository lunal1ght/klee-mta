//===-- ThreadList.cpp ------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Thread/ThreadList.h"

#include <cassert>
#include <iostream>

using namespace ::std;
namespace klee {

ThreadList::ThreadList() : threadNum(0) {
  allThreads.resize(20, nullptr);
  index.reserve(20);
}

ThreadList::~ThreadList() = default;

ThreadList::iterator ThreadList::begin() { return iterator(this); }

ThreadList::iterator ThreadList::end() { return iterator(this, threadNum); }

ThreadList::iterator ThreadList::begin() const { return iterator(const_cast<ThreadList*>(this)); }

ThreadList::iterator ThreadList::end() const { return iterator(const_cast<ThreadList*>(this), threadNum); }

void ThreadList::addThread(Thread* thread) {
    if (allThreads.size() <= thread->threadId) {
        allThreads.resize(thread->threadId * 2, nullptr);
    }
    assert(!allThreads[thread->threadId] && "Thread with this ID already exists");
    allThreads[thread->threadId] = thread;
    index.push_back(thread->threadId);
    threadNum++;
}


std::map<unsigned, Thread*> ThreadList::getAllUnfinishedThreads() {
    std::map<unsigned, Thread*> result;
    for (iterator it = begin(); it != end(); it++) { //  Постфиксный инкремент
        Thread* thread = *it;
        if (thread && !thread->isTerminated()) {
            result.emplace(thread->threadId, thread);
        }
    }
    return result;
}

Thread* ThreadList::findThreadById(unsigned threadId) {
    if (threadId < allThreads.size() && allThreads[threadId]) {
        return allThreads[threadId];
    } else {
        return nullptr; 
    }
}

int ThreadList::getThreadNum() { return threadNum; }

Thread* ThreadList::getLastThread() {
    if (!index.empty()) {
        return allThreads[index.back()];
    } else {
        return nullptr;  //  Или  бросьте  исключение
    }
}

// --- ThreadList::iterator ---

ThreadList::iterator::iterator(ThreadList* threadList, unsigned pos) : threadList(threadList), currentPos(pos) {}

ThreadList::iterator& ThreadList::iterator::operator++() {
    ++currentPos;
    return *this;
}//итератор префа

ThreadList::iterator ThreadList::iterator::operator++(int) {
    iterator temp(*this);  // Создаем копию текущего состояния итератора
    ++currentPos;          // Инкрементируем текущий итератор
    return temp;          // Возвращаем копию до инкремента
}

Thread*& ThreadList::iterator::operator*() {
    return threadList->allThreads[threadList->index[currentPos]];
}

bool ThreadList::iterator::operator==(const iterator& another) const {
  return threadList == another.threadList && currentPos == another.currentPos;

}

bool ThreadList::iterator::operator!=(const iterator& another) const {
    return !(*this == another);

}

} // namespace klee
