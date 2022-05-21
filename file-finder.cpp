#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>

#define DBG_printf printf

char* root_dir;
char** search_substrs;

void print_usage() {
  printf("file-finder <dir> <substring1>[<substring2> [<substring3>]...]\n");
}

void print_substr(int i) {
  printf("%s\n", search_substrs[i]);
}

using namespace std;

class SubStringWorker {
  public:
    char* mSubstr;
    thread mThread;
    mutex mMutex;
    queue<char*> mQueue;
    condition_variable cv;
    bool mKill = false;

    SubStringWorker(char* substr) :
      mSubstr(substr) {
        mThread = thread(worker, this);
      }

    static void worker(SubStringWorker* w) {
      DBG_printf("<%s> starting worker\n", w->mSubstr);

      while(true) {
        DBG_printf("<%s> waiting for item in queue\n", w->mSubstr);
        unique_lock lk(w->mMutex);
        w->cv.wait(lk, [w] {
          return w->mKill || !w->mQueue.empty();
        });

        if (w->mKill) {
          DBG_printf("<%s> was killed\n", w->mSubstr);
          return;
        }

        char* filename = w->mQueue.front();
        w->mQueue.pop();

        lk.unlock();

        DBG_printf("<%s> filename = %s\n", w->mSubstr, filename);
      }
    }

    void enqueue(char* filename) {
      DBG_printf("<main> waiting on queue mutex <%s>\n", mSubstr);
      {
        lock_guard lk(mMutex);
        mQueue.push(filename);
      }
      cv.notify_one();
      DBG_printf("<main> added %s to <%s>\n", filename, mSubstr);
    }

    void join() {
      mThread.join();
    }

    void kill() {
      {
        lock_guard lk(mMutex);
        mKill = true;
      }
      cv.notify_one();
      DBG_printf("<main> send kill signal to <%s>\n", mSubstr);
    }
};

int main(int argc, char** argv) {

  if (argc < 3) {
    print_usage();
    exit(0);
  }

  // get the root directory from args
  root_dir = argv[1];

  // get search substrings from args
  const int search_substrs_count = argc - 2;
  search_substrs = &argv[2];

  vector<SubStringWorker*> workers;

  for (int i = 0; i < search_substrs_count; ++i) {
    workers.push_back(new SubStringWorker(search_substrs[i]));
  }

  char* filenames[] = {
    "word.exe",
    "calc.exe",
    "sudo",
    "taxes.docx",
    "private_notes.txt"
  };

  for (auto w : workers) {
    for (auto filename : filenames) {
      w->enqueue(filename);
    }
  }

  std::this_thread::sleep_for(std::chrono::seconds(1s));

  for (auto w : workers) {
    w->kill();
  }

  for (auto w : workers) {
    w->join();
  }
}