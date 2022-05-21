#include <filesystem>
#include <map>
#include <mutex>
#include <vector>

#if !defined(MATCHCONTAINER_H)
#define MATCHCONTAINER_H

class MatchContainer {
  std::mutex mMutex;
  std::vector<std::string> mMatches;

  public:
    MatchContainer() = default;

    void addMatch(std::string filepath);
    void dumpMatches();
};

#endif