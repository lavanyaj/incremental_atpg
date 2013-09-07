#include "util.h"
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
class UtilTest : public testing::Test {
 protected:
  UtilTest() { 
    log4cxx::BasicConfigurator::resetConfiguration();
    log4cxx::BasicConfigurator::configure();
}

  virtual void SetUp() {
    util.reset(new Util);
  }
  unique_ptr<Util> util;
};

TEST_F(UtilTest, MakeRules) {
  vector<vector<string> > sets;
  // num_rules, num_sets, max_rules_per_set, distr. zipf
  // most sets should have 1-5 rules, a few have close to 30
  // all rules have many sets, few sets that are in all rules.
  // TODO(lav): Most sets have too many rules. Flip!?
  //  util->MakeRules(7000, 3000, 20, Util::zipf_1, &sets);
  //  util->ShowRulesPerSet(sets);
}

TEST_F(UtilTest, WriteRulesToFile) {
  vector<vector<string> > sets;
  // num_rules, num_sets, max_rules_per_set, distr. zipf
  // most sets should have 1-5 rules, a few have close to 30
  // all rules have many sets, few sets that are in all rules.
  // TODO(lav): Most sets have too many rules. Flip!?
  util->MakeRules(700000, 300000, 200, Util::zipf_1, &sets);
  util->WriteRulesToFile(sets, "tmp/WriteRulesToFileTest.out");
}

TEST_F(UtilTest, ReadRulesFromFile) {
  vector<vector<string> > sets;
  //  util->ReadRulesFromFile("tmp/WriteRulesToFileTest.out", &sets);
  //  util->ShowSetsPerRule(sets);
}
 
}  // namespace incremental_atpg
