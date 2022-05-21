#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "FileObject.h"
#include "SubStringWorker.h"

namespace fs = std::filesystem;

void print_usage() {
  printf("file-finder <dir> <substring1>[<substring2> [<substring3>]...]\n");
}

int main(int argc, char** argv) {
  if (argc < 3) {
    print_usage();
    exit(0);
  }

  // Get the root directory from args
  char* const root_dir = argv[1];

  if (!fs::is_directory(fs::status(root_dir))) {
    printf("Error: %s is not a directory!\n", root_dir);
    print_usage();
    exit(0);
  }

  // Get search substrings from args
  const int search_substrs_count = argc - 2;
  char* const* const search_substrs = &argv[2];

  // Create workers
  std::vector<SubStringWorker*> workers;

  for (int i = 0; i < search_substrs_count; ++i) {
    auto worker = new SubStringWorker();
    worker->setMatch(search_substrs[i]);
    worker->start();
    workers.push_back(worker);
  }

  // Recursively iterate through directory and enqueue filenames
  //
  // Ideally this would "just work" with recursive_directory_iterator.
  // recursive_directory_iterator would eventually reach directory it
  // could not read and would stop iterating at that point (at least
  // in my environment).
  //
  // This implements a basic breadth first search with non-recursive
  // directory_iterator and a queue.

  auto ec = std::error_code();

  std::queue<fs::path> dirQueue;
  dirQueue.push(root_dir);

  while (!dirQueue.empty()) {
    auto dir = dirQueue.front();

    for (auto const& dir_entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
      if (ec) {
        continue;
      }

      if (fs::is_directory(dir_entry.path())) {
        dirQueue.push(dir_entry.path());
        continue;
      }

      std::shared_ptr<FileObject> fileobj = std::make_shared<FileObject>(dir_entry);

      for (auto w : workers) {
        w->enqueue(fileobj);
      }
    }

    dirQueue.pop();
  }

  // Sleep to allow threads to finish remaining work
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Kill workers
  for (auto w : workers) {
    w->kill();
  }

  // Wait for threads to exit
  for (auto w : workers) {
    w->join();
  }
}