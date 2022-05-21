#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <queue>

#include "FileObject.h"

#if !defined(SUBSTRINGWORKER_H)
#define SUBSTRINGWORKER_H

class SubStringWorker {
  public:
    char* mSubstr;
    std::thread mThread;
    std::mutex mMutex;
    std::queue<std::shared_ptr<FileObject>> mQueue;
    std::condition_variable mCondVar;
    bool mKill;

    SubStringWorker(char* substr);

    static void worker(SubStringWorker* w);
    void enqueue(std::shared_ptr<FileObject> fileobj);
    void join();
    void kill();
};

#endif