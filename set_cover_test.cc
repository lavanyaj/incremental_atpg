#include "set_cover.h"
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
class SetCoverTest : public testing::Test {
 protected:
  SetCoverTest() { 
    log4cxx::BasicConfigurator::resetConfiguration();
    log4cxx::BasicConfigurator::configure();
}

  virtual void SetUp() {
    sc_.reset(new SetCover);
 }
  std::unique_ptr<SetCover> sc_;
};

TEST_F(SetCoverTest, SetUp) {
  map<string, SetInfo>* set_infos
    = new map<string, SetInfo>();
  SetInfo cat;
  cat.AddRule(0);
  set_infos->insert(make_pair("cat", cat));
  SetInfo bob;
  bob.AddRule(0);
  bob.AddRule(1);
  set_infos->insert(make_pair("bob", bob));

  vector<RuleInfo>* rule_infos = new vector<RuleInfo>();
  vector<string> rule0;
  rule0.push_back("cat");
  rule0.push_back("bob");
  RuleInfo info0(rule0);
  vector<string> rule1;
  rule1.push_back("bob");
  RuleInfo info1(rule1);

  rule_infos->push_back(info0);
  rule_infos->push_back(info1);

  sc_.reset(new SetCover(set_infos, rule_infos));
  /*
  uint64_t expected_index = max_uint_/ 2;
  om_->InsertAtEnd("India", Value("New Delhi"));
  EXPECT_EQ(expected_index, om_->indices["India"]);
  EXPECT_TRUE(om_->values[expected_index].Has("New Delhi"));
  EXPECT_EQ("India", om_->ids[expected_index]);
  EXPECT_EQ(1UL, om_->values.size());
  */
}
  TEST_F(SetCoverTest, ResetProcessingInfo) {
    sc_->AddRule({"cat", "dog"});
    sc_->AddRule({"cat"});
    {
      sc_->ResetProcessingInfo();
      EXPECT_EQ(2, sc_->rule_processing_infos_->size());
      for (auto const& rp : *sc_->rule_processing_infos_.get()) {
	EXPECT_TRUE(rp.first_covered_by.empty());
      }
      EXPECT_TRUE(sc_->set_processing_infos_->empty());
    }
    
    {
      sc_->cover_->push_back("cat");
      sc_->ResetProcessingInfo();
      EXPECT_EQ(2, sc_->rule_processing_infos_->size());
      for (auto const& rp : *sc_->rule_processing_infos_.get()) {
	EXPECT_EQ("cat", rp.first_covered_by);
      }
      EXPECT_EQ(1, sc_->set_processing_infos_->size());
      EXPECT_TRUE(sc_->set_processing_infos_->find("cat") !=
		  sc_->set_processing_infos_->end());
      SetProcessingInfo sp = sc_->set_processing_infos_->at("cat");
      EXPECT_EQ(2, sp.num_uncovered);
      EXPECT_EQ(2, sp.covers_rules.size());
      EXPECT_TRUE(sp.covers_rules.find(0) != sp.covers_rules.end());
      EXPECT_TRUE(sp.covers_rules.find(1) != sp.covers_rules.end());
    }
    
  }

}  // namespace incremental_atpg
