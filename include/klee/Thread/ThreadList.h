//===-- ThreadList.h --------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef THREADLIST_H_
#define THREADLIST_H_

#include "klee/Thread/Thread.h"
#include <map>
#include <vector>

namespace klee {

class ThreadList {

private:
    std::vector<Thread*> allThreads;
    std::vector<unsigned> index;
    int threadNum;


public:
    class iterator {  // iterator теперь вложенный класс

    private:

        friend class ThreadList;  //  ThreadList  имеет  доступ к  private  членам iterator


        ThreadList* threadList;
        unsigned currentPos;


    public:
        iterator(ThreadList* threadList, unsigned pos = 0);
        iterator& operator++();  // Префиксный инкремент
        iterator operator++(int); // Постфиксный инкремент
        Thread*& operator*(); // Возвращаем  указатель  на  Thread*

        bool operator==(const iterator& another) const;
        bool operator!=(const iterator& another) const;


    };


    ThreadList();
    ~ThreadList(); //  Не виртуальный
    iterator begin();
    iterator end();
    iterator begin() const;
    iterator end() const;
    void addThread(Thread* thread);
    std::map<unsigned, Thread*> getAllUnfinishedThreads();
    Thread* findThreadById(unsigned threadId);
    int getThreadNum();
    Thread* getLastThread();

    // Добавляем методы доступа к данным для iterator
    const std::vector<Thread*>& getAllThreads() const { return allThreads; }
    const std::vector<unsigned>& getIndex() const { return index; }
};




} // namespace klee

#endif // THREADLIST_H_
