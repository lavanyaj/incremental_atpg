#ifndef INCREMENTAL_ATPG_ONLINE_SET_COVER_H_
#define INCREMENTAL_ATPG_ONLINE_SET_COVER_H_
#include <vector>
#include <string>
#include <memory>
#include <set>
#include <map>
#include <stdint.h>
#include <utility>
#include <log4cxx/logger.h>

#include "gtest/gtest_prod.h"
#include "set_cover.h"
#include "lazy_set_cover.h"
#include "greedy_set_cover.h"

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::map;
  using std::make_pair;
  using std::pair;
  using std::set;

  class OnlineSetCover : public LazySetCover {
  public:
    log4cxx::LoggerPtr online_set_cover_logger;
   
  OnlineSetCover()
    : gr_(nullptr),
      adds_(0),
      updates_(0),
      best_greedy_fraction_(1.0),
      greedy_updates_(0) {
      online_set_cover_logger = Logger::getLogger("OnlineSetCover");
      online_set_cover_logger->setLevel(log4cxx::Level::getWarn());
    }

    // Takes ownership of @set_infos..
  OnlineSetCover(map<string, SetInfo>* set_infos,
		 vector<RuleInfo>* rule_infos)
    : LazySetCover(set_infos, rule_infos),
      gr_(nullptr),
      adds_(0),
      updates_(0),
      best_greedy_fraction_(1.0),
      greedy_updates_(0)  {
      online_set_cover_logger = Logger::getLogger("OnlineSetCover");
      online_set_cover_logger->setLevel(log4cxx::Level::getWarn());
    }

    void AddRule(const vector<string>& sets);
    void GreedyUpdateCover();
    void UpdateCover();
    void ShowStats();
    bool SanityCheck();

  protected:
    unique_ptr<GreedySetCover> gr_;
    bool NoNullPtrs();
    bool GetMin(double* min);
    bool GetSum(double* sum);
    bool GoodEnough();
    uint64_t adds_;
    uint64_t updates_;
    double best_greedy_fraction_;
    uint64_t greedy_updates_;
  private:
    friend class OnlineSetCoverTest;
    FRIEND_TEST(OnlineSetCoverTest, UpdateCover);
  };
}  // namespace incremental_atpg
#endif  // INCREMENTAL_ATPG_ONLINE_SET_COVER_H_
