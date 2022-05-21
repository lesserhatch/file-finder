#include <stdio.h>

#include "SubStringWorker.h"

#define DBG_printf printf

SubStringWorker::SubStringWorker(char* substr) : mSubstr(substr) {
  mThread = std::thread(worker, this);
}

void SubStringWorker::worker(SubStringWorker* w) {
  DBG_printf("<%s> starting worker\n", w->mSubstr);

  while(true) {
    DBG_printf("<%s> waiting for item in queue\n", w->mSubstr);
    std::unique_lock lk(w->mMutex);
    w->mCondVar.wait(lk, [w] {
      return w->mKill || !w->mQueue.empty();
    });

    if (w->mKill) {
      DBG_printf("<%s> was killed\n", w->mSubstr);
      return;
    }

    std::shared_ptr<FileObject> fileobj = w->mQueue.front();
    w->mQueue.pop();
    lk.unlock();

    DBG_printf("<%s> filename = %s\n", w->mSubstr, fileobj.get()->filename.c_str());
  }
}

void SubStringWorker::enqueue(std::shared_ptr<FileObject> fileobj) {
  DBG_printf("<main> waiting on queuefileobj mutex <%s>\n", mSubstr);
  {
    std::lock_guard lk(mMutex);
    mQueue.push(fileobj);
  }
  mCondVar.notify_one();
  DBG_printf("<main> added %s to <%s>\n", fileobj.get()->filename.c_str(), mSubstr);
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
  DBG_printf("<main> send kill signal to <%s>\n", mSubstr);
}