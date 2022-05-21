#include <iostream>

#include "MatchContainer.h"

void MatchContainer::addMatch(std::string filepath) {
  std::lock_guard lk(mMutex);
  mMatches.push_back(filepath);
}

void MatchContainer::dumpMatches() {
  std::lock_guard lk(mMutex);

  for (auto const& match : mMatches) {
    std::cout << match << std::endl;
  }

  mMatches.clear();
}