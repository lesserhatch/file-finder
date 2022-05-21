#include <stdio.h>

#include "SubStringWorker.h"

#define DBG_printf printf

void SubStringWorker::worker(SubStringWorker* w) {
  // DBG_printf("<%s> starting worker\n", w->mMatch.c_str());

  while(true) {
    // DBG_printf("<%s> waiting for item in queue\n", w->mMatch.c_str());
    std::unique_lock lk(w->mMutex);
    w->mCondVar.wait(lk, [w] {
      return w->mKill || !w->mQueue.empty();
    });

    if (w->mKill) {
      // DBG_printf("<%s> was killed\n", w->mMatch.c_str());
      return;
    }

    std::shared_ptr<FileObject> fileobj = w->mQueue.front();
    w->mQueue.pop();
    lk.unlock();

    std::string filename = fileobj.get()->getFilename();

    if (filename.find(w->mMatch) != std::string::npos) {
      w->mMatchContainer->addMatch(fileobj.get()->getFilepath());
      // DBG_printf("<%s> Match found! Filename = %s\n", w->mMatch.c_str(), fileobj.get()->getFilepath().c_str());
    }
  }
}

void SubStringWorker::enqueue(std::shared_ptr<FileObject> fileobj) {
  // DBG_printf("<main> waiting on queue mutex <%s>\n", mMatch.c_str());
  {
    std::lock_guard lk(mMutex);
    mQueue.push(fileobj);
  }
  mCondVar.notify_one();
  // DBG_printf("<main> added %s to <%s>\n", fileobj.get()->filename.c_str(), mMatch.c_str());
}

void SubStringWorker::join() {
  mThread.join();
}

void SubStringWorker::kill() {
  {
    std::lock_guard lk(mMutex);
    mKill = true;
  }
  mCondVar.notify_one();
  // DBG_printf("<main> send kill signal to <%s>\n", mMatch.c_str());
}

bool SubStringWorker::setMatch(std::string match) {
  if (mThreadStarted) {
    return false;
  }

  mMatch = match;

  return true;
}

bool SubStringWorker::setMatchContainer(MatchContainer* mc) {
  if (mThreadStarted) {
    return false;
  }

  mMatchContainer = mc;

  return true;
}

bool SubStringWorker::start() {
  if (mThreadStarted) {
    return false;
  }

  mThread = std::thread(worker, this);

  return true;
}