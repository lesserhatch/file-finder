#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "FileObject.h"
#include "MatchContainer.h"
#include "SubStringWorker.h"

namespace fs = std::filesystem;

bool exitRequested = false;

void print_usage() {
  std::cout << "file-finder <dir> <substring1>[<substring2> [<substring3>]...]" << std::endl;
}

void console_parser(MatchContainer* mc) {
  std::string command;

  while (command != "exit") {
    if (command == "dump") {
      mc->dumpMatches();
    }
    getline(std::cin, command);
  }

  if (command == "exit") {
    exitRequested = true;
  }
}

void periodic_dumper(MatchContainer* mc) {
  while (!exitRequested) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mc->dumpMatches();
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    print_usage();
    exit(0);
  }

  // Create match container
  auto mc = MatchContainer();

  // Start console and periodic dumper
  auto console = std::thread(console_parser, &mc);
  auto dumper = std::thread(periodic_dumper, &mc);

  // Get the root directory from args
  char* const root_dir = argv[1];

  if (!fs::is_directory(fs::status(root_dir))) {
    std::cout << "Error: " << root_dir << " is not a directory!" << std::endl;
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
    worker->setMatchContainer(&mc);
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

  // Run until exit is requested or queue is empty
  while (!(exitRequested || dirQueue.empty())) {
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

  // Kill workers
  for (auto w : workers) {
    w->kill();
    w->join();
    delete w;
  }

  workers.clear();

  // Dump any remaining matches
  mc.dumpMatches();

  exit(0);
}