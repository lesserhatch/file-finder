#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "SubStringWorker.h"

void print_usage() {
  printf("file-finder <dir> <substring1>[<substring2> [<substring3>]...]\n");
}

int main(int argc, char** argv) {
  if (argc < 3) {
    print_usage();
    exit(0);
  }

  // get the root directory from args
  char* const root_dir = argv[1];

  // get search substrings from args
  const int search_substrs_count = argc - 2;
  char* const* const search_substrs = &argv[2];

  std::vector<SubStringWorker*> workers;

  for (int i = 0; i < search_substrs_count; ++i) {
    workers.push_back(new SubStringWorker(search_substrs[i]));
  }

  char const* const filenames[] = {
    "word.exe",
    "calc.exe",
    "sudo",
    "taxes.docx",
    "private_notes.txt"
  };

  for (auto f : filenames) {
    std::shared_ptr<FileObject> filename = std::make_shared<FileObject>(std::string(f));

    for (auto w : workers) {
      w->enqueue(filename);
    }
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));

  for (auto w : workers) {
    w->kill();
  }

  for (auto w : workers) {
    w->join();
  }
}