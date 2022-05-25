#include <mutex>
#include <string>
#include <thread>
#include <queue>

#include "FileObject.h"
#include "MatchContainer.h"

#if !defined(SUBSTRINGWORKER_H)
#define SUBSTRINGWORKER_H

class SubStringWorker {
  public:
    std::string mMatch;
    std::thread mThread;
    std::mutex mMutex;
    std::queue<std::shared_ptr<FileObject>> mQueue;
    std::condition_variable mCondVar;
    bool mKill;
    bool mThreadStarted;
    MatchContainer* mMatchContainer;

    SubStringWorker();

    static void worker(SubStringWorker* w);
    void enqueue(std::shared_ptr<FileObject> fileobj);
    void join();
    void kill();
    bool setMatch(char* const match);
    bool setMatchContainer(MatchContainer* mc);
    bool start();
};

#endif