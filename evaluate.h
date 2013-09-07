#ifndef INCREMENTAL_ATPG_EVALUATE_H_
#define INCREMENTAL_ATPG_EVALUATE_H_

#include <vector>
#include <string>
#include <memory>
#include <list>
#include <set>
#include <map>
#include <stdint.h>
#include <utility>
#include <log4cxx/logger.h>

#include "gtest/gtest_prod.h"
#include "set_cover.h"
#include "lazy_set_cover.h"
#include "greedy_set_cover.h"
#include "online_set_cover.h"
#include "util.h"

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::map;
  using std::make_pair;
  using std::pair;
  using std::set;
  using std::list;

  class Evaluate : public LazySetCover {
  public:
    log4cxx::LoggerPtr evaluate_logger;
   
  Evaluate()
    : gr_(nullptr),
      on_(nullptr) {
      evaluate_logger = Logger::getLogger("Evaluate");
      evaluate_logger->setLevel(log4cxx::Level::getWarn());
    }

    // Takes ownership of @set_infos..
  Evaluate(const string& input_file, const string& output_file) 
    : gr_(nullptr),
      on_(nullptr),
      input_file_(input_file),
      output_file_(output_file) {
      evaluate_logger = Logger::getLogger("Evaluate");
      evaluate_logger->setLevel(log4cxx::Level::getWarn());
      util.ReadRulesFromFile(input_file, &sets_);
    }

    void Compare(uint64_t at_num_rules, uint64_t for_time);

  protected:
    unique_ptr<GreedySetCover> gr_;
    unique_ptr<OnlineSetCover> on_;
    Util util;
    string input_file_;
    string output_file_;
    vector<vector<string> > sets_;
  private:
    friend class EvaluateTest;
    FRIEND_TEST(EvaluateTest, UpdateCover);
  };
}  // namespace incremental_atpg
#endif  // INCREMENTAL_ATPG_EVALUATE_H_
