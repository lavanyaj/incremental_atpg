#include "set_cover.h"
#include "greedy_set_cover.h"
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


  class GreedySetCoverTest : public testing::Test {
  protected:
    GreedySetCoverTest() { 
      log4cxx::BasicConfigurator::resetConfiguration();
      log4cxx::BasicConfigurator::configure();
    }

    virtual void SetUp() {
      sc_.reset(new GreedySetCover);
    }
    virtual void TearDown() {
    }
    std::unique_ptr<GreedySetCover> sc_;
  };
  /*
    vector<string> rule2;
    rule2.push_back("giraffe");
    rule2.push_back("cat");
    sc_->AddRule(rule2);
  */

  TEST_F(GreedySetCoverTest, UpdateProcessingInfo) {
    sc_->AddRule({"cat", "dog"});
    sc_->AddRule({"cat"});
    sc_->ResetProcessingInfo();
    sc_->cover_->push_back("dog");
    map<string, uint64_t> key_changes;
    uint64_t covered = 0;
    sc_->UpdateProcessingInfo(&covered, &key_changes);
    EXPECT_EQ(1, covered);
    EXPECT_EQ(2, key_changes.size()); // Need to update both "dog" and "cat" in heap. 
    EXPECT_TRUE(key_changes.find("dog") != key_changes.end());
    EXPECT_EQ(1, key_changes.find("dog")->second);
    EXPECT_TRUE(key_changes.find("cat") != key_changes.end());
    EXPECT_EQ(1, key_changes.find("cat")->second);


    EXPECT_EQ(2, sc_->rule_processing_infos_->size());
    RuleProcessingInfo rp0 = sc_->rule_processing_infos_->at(0);
    RuleProcessingInfo rp1 = sc_->rule_processing_infos_->at(1);
    EXPECT_EQ("dog", rp0.first_covered_by);
    EXPECT_TRUE(rp1.first_covered_by.empty());
  }

  TEST_F(GreedySetCoverTest, AddAllSetsToHeap) {
    sc_->AddRule({"cat", "dog"});
    sc_->AddRule({"cat"});

    sc_->AddAllSetsToHeap();    
    EXPECT_EQ(2, sc_->heap_->size());
    EXPECT_EQ(heap_data("cat", 2), sc_->heap_->top());
    sc_->heap_->pop();
    EXPECT_EQ(heap_data("dog", 1), sc_->heap_->top());
  }


  TEST_F(GreedySetCoverTest, UpdateSetsInHeap) {
    sc_->AddRule({"cat", "dog"});
    sc_->AddRule({"cat"});

    
      sc_->AddAllSetsToHeap();    
      {
	map<string, uint64_t> key_changes;
	key_changes["cat"] = 1;
	key_changes["dog"] = 0;
	
	sc_->UpdateSetsInHeap(key_changes);
	EXPECT_EQ(2, sc_->heap_->size());
	EXPECT_EQ(heap_data("dog", 1), sc_->heap_->top());
	sc_->heap_->pop();
	EXPECT_EQ(heap_data("cat", 1), sc_->heap_->top());
	sc_->heap_->pop();
      }
      sc_->AddAllSetsToHeap();    
      {
	map<string, uint64_t> key_changes;
	key_changes["cat"] = 2;
	key_changes["bubblewrap"] = 5;
	sc_->UpdateSetsInHeap(key_changes);
	EXPECT_EQ(2, sc_->heap_->size());
	EXPECT_EQ(heap_data("dog", 1), sc_->heap_->top());
	sc_->heap_->pop();
	EXPECT_EQ(heap_data("cat", 0), sc_->heap_->top());
      }

  }

  TEST_F(GreedySetCoverTest, AddRule) {
    sc_->AddRule({"cat", "dog"});
    sc_->AddRule({"cat"});

    map<string, SetInfo> si = sc_->GetSetInfos();
    EXPECT_EQ(2, si.size());
    EXPECT_TRUE(si.find("cat") != si.end());    
  }

  TEST_F(GreedySetCoverTest, UpdateCover) {
    sc_->AddRule({"cat", "dog"});
    sc_->AddRule({"cat"});
    sc_->UpdateCover();
    EXPECT_EQ(1, sc_->cover_->size());
    EXPECT_EQ("cat", sc_->cover_->back());

    sc_->AddRule({"jam"});
    sc_->UpdateCover();
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("jam", sc_->cover_->back());
    sc_->cover_->pop_back();
    EXPECT_EQ("cat", sc_->cover_->back());
    sc_->cover_->pop_back();
  }
}  // namespace incremental_atpg
