#include "evaluate.h"
#include "gtest/gtest.h"

#include <ctime>
#include <memory>
#include <stdint.h>

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"

namespace incremental_atpg {
  using std::to_string;
  using std::string;
  using std::map;
  using std::make_pair;
class EvaluateTest : public testing::Test {
 protected:
  EvaluateTest() { 
    log4cxx::BasicConfigurator::resetConfiguration();
    log4cxx::BasicConfigurator::configure();
}

  virtual void SetUp() {
    evaluate.reset(new Evaluate("tmp/WriteRulesToFileTest.out", "out"));
  }
  unique_ptr<Evaluate> evaluate;
};

  //    void Compare(uint64_t at_num_rules, uint64_t for_time);
TEST_F(EvaluateTest, Compare) {
  evaluate->Compare(1000, 20);
  //  evaluate->ShowRulesPerSet(sets);
}
}  // namespace incremental_atpg
