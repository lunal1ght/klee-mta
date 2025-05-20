//===-- MutexManager.cpp ----------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/Thread/MutexManager.h"
#include "klee/Encode/Transfer.h"

using namespace ::std;

namespace klee {

MutexManager::MutexManager() : nextMutexId(1) {
  llvm::errs() << "MutexManager CONSTRUCTED (original): this=" << this << "\n"; // Отладка
}
/*
MutexManager::MutexManager(const MutexManager& other)
    : nextMutexId(other.nextMutexId) { // nextMutexId тоже должен копироваться
    llvm::errs() << "MutexManager COPIED: new " << this << " from old " << &other << "\n"; // Отладка
    for (const auto& pair : other.mutexMap) {
        if (pair.second) { // Проверка на nullptr
            mutexMap[pair.first] = new Mutex(*(pair.second)); // Создаем НОВУЮ копию Mutex
        } else {
            mutexMap[pair.first] = nullptr; // Копируем nullptr, если он был
        }
    }
    llvm::errs() << "MutexManager COPIED: new map size " << mutexMap.size() << "\n"; // Отладка
    // blockedThreadPool копируем аккуратно или очищаем
    blockedThreadPool.clear(); // Для простоты пока очищаем
}*/

//недоработанная логика мьютексманагера

MutexManager::~MutexManager() { clear(); }

/*
MutexManager::~MutexManager() {
  llvm::errs() << "MutexManager DESTROYED: this=" << this << ", map size before clear: " << mutexMap.size() << "\n"; // Отладка
  for (auto& pair : mutexMap) {
      delete pair.second; // Удаляем объекты Mutex
  }
  mutexMap.clear();
  // blockedThreadPool уже должен быть очищен или его элементы не должны удаляться здесь,
  // если они указывают на мьютексы из mutexMap.
  // Если blockedThreadPool хранит УКАЗАТЕЛИ на те же мьютексы, что и mutexMap,
  // то их НЕ НУЖНО удалять дважды. Просто очистить map:
  blockedThreadPool.clear();
}
*/

//недоработанная логика мьютексманагера

bool MutexManager::lock(string mutexName, unsigned threadId, bool &isBlocked, string &errorMsg) {
  Mutex *mutex = getMutex(mutexName);
  if (!mutex) {
    errorMsg = "mutex " + mutexName + " undefined";
    return false;
  } else {
    return lock(mutex, threadId, isBlocked, errorMsg);
  }
}

bool MutexManager::lock(Mutex *mutex, unsigned threadId, bool &isBlocked, string &errorMsg) {
  if (mutex->isMutexLocked()) {
    // mutex->addToBlockedList(thread);
    blockedThreadPool.insert(make_pair(threadId, mutex));
    isBlocked = true;
  } else {
    mutex->lock(threadId);
    isBlocked = false;
    map<unsigned, Mutex *>::iterator ti = blockedThreadPool.find(threadId);
    if (ti != blockedThreadPool.end()) {
      blockedThreadPool.erase(ti);
      // mutex->removeFromBlockedList(thread);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool MutexManager::destroyMutex(const std::string &mutexName, std::string &errorMsg) {
  std::map<std::string, Mutex*>::iterator mit = mutexMap.find(mutexName);
  if (mit == mutexMap.end()) {
    errorMsg = "Mutex " + mutexName + " not found (already destroyed or never initialized).";
    // По стандарту POSIX, уничтожение неинициализированного мьютекса - неопределенное поведение.
    // KLEE может сообщить об ошибке или просто вернуть ошибку.
    return false; // Или true, если считаем это допустимым (хотя стандарт не рекомендует)
  }

  Mutex *mutex = mit->second;
  if (mutex->isCurrentlyLocked()) { // Проверяем, не заблокирован ли мьютекс
    errorMsg = "Mutex " + mutexName + " is currently locked, cannot destroy.";
    // По стандарту POSIX, уничтожение заблокированного мьютекса - неопределенное поведение.
    return false; // Возвращаем ошибку (например, EBUSY)
  }

  // Если все хорошо, удаляем мьютекс из карты и сам объект мьютекса
  delete mutex;
  mutexMap.erase(mit);
  errorMsg = "Mutex " + mutexName + " destroyed successfully.";
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

bool MutexManager::unlock(string mutexName, string &errorMsg) {
  Mutex *mutex = getMutex(mutexName);
  if (!mutex) {
    errorMsg = "mutex " + mutexName + " undefined";
    return false;
  } else {
    return unlock(mutex, errorMsg);
  }
}

bool MutexManager::unlock(Mutex *mutex, string &errorMsg) {
  if (!mutex->isMutexLocked()) {
    // errorMsg = "mutex " + mutex->name + " has not been locked";
    // return false;
    // this is not an error. Calling unlock without any lock is ok.
    cerr << "warning: mutex " + mutex->name + " there isn't any lock before this unlock\n";
    return true;
  } else {
    mutex->unlock();
    return true;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool MutexManager::addMutex(const std::string& mutexName, std::string &errorMsg) {
  // Используем getMutex, чтобы проверить, существует ли он
  Mutex *existingMutex = getMutex(mutexName); // getMutex должен искать в this->mutexMap
  if (existingMutex) {
    errorMsg = "Mutex '" + mutexName + "' already exists (re-initialization attempt).";
    return false; // Мьютекс уже есть, НЕ добавляем снова
  } else {
    Mutex *mutex = new Mutex(nextMutexId++, mutexName);
    mutexMap.insert(std::make_pair(mutexName, mutex)); // Добавляем в this->mutexMap
    errorMsg = "Mutex '" + mutexName + "' created successfully.";
    return true;
  }
}

Mutex* MutexManager::getMutex(const std::string &mutexName) {
  std::map<std::string, Mutex*>::iterator it = mutexMap.find(mutexName); // Используем mutexMap
  if (it == mutexMap.end()) {
    return NULL;
  } else {
    return it->second;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////

void MutexManager::clear() {
  for (map<string, Mutex *>::iterator mi = mutexMap.begin(), me = mutexMap.end(); mi != me; mi++) {
    delete mi->second;
  }
  mutexMap.clear();
  blockedThreadPool.clear();
  nextMutexId = 1;
}

void MutexManager::print(ostream &out) {
  out << "mutex map\n";
  for (map<string, Mutex *>::iterator mi = mutexMap.begin(), me = mutexMap.end(); mi != me; mi++) {
    out << mi->first << endl;
  }
}

unsigned MutexManager::getNextMutexId() { return nextMutexId; }

void MutexManager::addBlockedThread(unsigned threadId, std::string mutexName) {
  Mutex *mutex = getMutex(mutexName);
  assert(mutex);
  blockedThreadPool.insert(make_pair(threadId, mutex));
}

bool MutexManager::tryToLockForBlockedThread(unsigned threadId, bool &isBlocked, string &errorMsg) {
  map<unsigned, Mutex *>::iterator mi = blockedThreadPool.find(threadId);
  if (mi == blockedThreadPool.end()) {
    errorMsg = "thread " + Transfer::uint64toString(threadId) + " does not blocked for mutex";
    return false;
  } else {
    Mutex *mutex = mi->second;
    return lock(mutex, threadId, isBlocked, errorMsg);
  }
}

} // namespace klee
