#include "SubStringWorker.h"

SubStringWorker::SubStringWorker() :
  mKill(false),
  mThreadStarted(false),
  mMatchContainer(nullptr) {}

void SubStringWorker::worker(SubStringWorker* w) {
  while(true) {
    std::unique_lock lk(w->mMutex);

    // Wait for mutex and either
    //    a) kill signal
    //    b) queue filled
    w->mCondVar.wait(lk, [w] {
      return w->mKill || !w->mQueue.empty();
    });

    if (w->mKill) {
      // Thread is being killed; early return
      return;
    }

    // Create a local copy of the shared pointer to the file
    // object then remove the original from the queue.
    std::shared_ptr<FileObject> fileobj = w->mQueue.front();
    w->mQueue.pop();

    // Queue operations are complete, release the mutex
    lk.unlock();

    // Search this filename for the match pattern
    std::string filename = fileobj.get()->getFilename();

    if (filename.find(w->mMatch) != std::string::npos) {
      if (w->mMatchContainer != nullptr) {
        // A match was found! Add it to the match container.
        w->mMatchContainer->addMatch(fileobj.get()->getFilepath());
      }
    }
  }
}

void SubStringWorker::enqueue(std::shared_ptr<FileObject> fileobj) {
  {
    std::lock_guard lk(mMutex);
    mQueue.push(fileobj);
  }

  // Notify outside of the lock guard scope otherwise the listener
  // will still see the locked mutex
  mCondVar.notify_one();
}

void SubStringWorker::join() {
  mThread.join();
}

void SubStringWorker::kill() {
  {
    std::lock_guard lk(mMutex);
    mKill = true;
  }

  // Notify outside of the lock guard scope otherwise the listener
  // will still see the locked mutex
  mCondVar.notify_one();
}

bool SubStringWorker::setMatch(char* const match) {
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