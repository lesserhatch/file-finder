#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <filesystem>
#include <memory>
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
    workers.push_back(new SubStringWorker(search_substrs[i]));
  }

  // Recursively iterate through directory and enqueue filenames
  for (auto const& dir_entry : fs::recursive_directory_iterator(root_dir)) {
    std::shared_ptr<FileObject> fileobj = std::make_shared<FileObject>(dir_entry);

    for (auto w : workers) {
      w->enqueue(fileobj);
    }
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