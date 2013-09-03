#include "online_set_cover.h"
#include "gtest/gtest.h"

#include <ctime>
#include <memory>
#include <stdlib.h> // srand, rand
#include <stdint.h>
#include <set>
#include <sstream>

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "set_cover.h"

namespace incremental_atpg {
  using std::to_string;
  using std::string;
  using std::map;
  using std::make_pair;
  using std::set;
  using std::ostringstream;
  using std::clock_t;


  class OnlineSetCoverTest : public testing::Test {
  protected:
    OnlineSetCoverTest() { 
      log4cxx::BasicConfigurator::resetConfiguration();
      log4cxx::BasicConfigurator::configure();
    }

    string GetString(uint64_t num) {
      return static_cast<ostringstream*> (&(ostringstream() << num) )->str();
    }
    virtual void SetUp() {
      sc_.reset(new OnlineSetCover);
    }
    virtual void TearDown() {
    }
    std::unique_ptr<OnlineSetCover> sc_;
    const uint64_t num_rules_ = 5000;
    const uint64_t num_sets_ = 10000;
    const uint64_t sets_per_rule_ = 500;
  };

  TEST_F(OnlineSetCoverTest, UpdateCoverManyGreedy) {
    srand(10);
    vector< vector<string> > rules;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      if (rule % 100 == 0) {
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Making rule #" << rule
		     << " of " << num_rules_);
      }
      vector<string> sets; set<uint64_t> added;
      for (uint64_t set_num = 0; set_num < sets_per_rule_; set_num++) {
	uint64_t set_uint64 = rand()%num_sets_;
	if (added.insert(set_uint64).second) {
	  sets.push_back(GetString(set_uint64));
	}
      }
      rules.push_back(sets);
      //EXPECT_TRUE(sc_->SanityCheck());
    }


    clock_t begin = clock();
    clock_t last = begin;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      if (rule % 100 == 0) {
	clock_t now = clock();
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Adding rule #" << rule
		     << " " << double(now - begin) / CLOCKS_PER_SEC
		     << " seconds after start"
		     << " and " << double(now - last) / CLOCKS_PER_SEC
		     << " seconds after last check.");
	EXPECT_TRUE(sc_->SanityCheck());
	last = now;
	sc_->ShowStats();
      }
      sc_->AddRule(rules[rule]);
      sc_->GreedyUpdateCover();
      //EXPECT_TRUE(sc_->SanityCheck());
    }
  }

  TEST_F(OnlineSetCoverTest, UpdateCoverMany) {
    srand(10);
    vector< vector<string> > rules;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      if (rule % 100 == 0) {
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Making rule #" << rule
		     << " of " << num_rules_);
      }
      vector<string> sets; set<uint64_t> added;
      for (uint64_t set_num = 0; set_num < sets_per_rule_; set_num++) {
	uint64_t set_uint64 = rand()%num_sets_;
	if (added.insert(set_uint64).second) {
	  sets.push_back(GetString(set_uint64));
	}
      }
      rules.push_back(sets);
      //EXPECT_TRUE(sc_->SanityCheck());
    }


    clock_t begin = clock();
    clock_t last = begin;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      if (rule % 100 == 0) {
	clock_t now = clock();
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Adding rule #" << rule
		     << " " << double(now - begin) / CLOCKS_PER_SEC
		     << " seconds after start"
		     << " and " << double(now - last) / CLOCKS_PER_SEC
		     << " seconds after last check.");
	EXPECT_TRUE(sc_->SanityCheck());
	last = now;
	sc_->ShowStats();
      }
      sc_->AddRule(rules[rule]);
      sc_->UpdateCover();
      //EXPECT_TRUE(sc_->SanityCheck());
    }
  }

  /*
  TEST_F(OnlineSetCoverTest, UpdateCover) {
    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"dog\"}");
    sc_->AddRule({"dog"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(1, sc_->cover_->size());

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"dog\", \"cat\"}");
    sc_->AddRule({"dog", "cat"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(1, sc_->cover_->size());

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"rain\"}");
    sc_->AddRule({"rain"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("dog", sc_->rule_processing_infos_->at(0).first_covered_by);
    EXPECT_EQ("dog", sc_->rule_processing_infos_->at(1).first_covered_by);
    EXPECT_EQ("rain", sc_->rule_processing_infos_->at(2).first_covered_by);

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"rain\"}");
    sc_->AddRule({"rain"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("rain", sc_->cover_->front());

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"rain\"}");
    sc_->AddRule({"rain"});
    sc_->UpdateCover();
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("rain", sc_->cover_->front());
    EXPECT_EQ("dog", sc_->cover_->back());
  }
  */
}  // namespace incremental_atpg
