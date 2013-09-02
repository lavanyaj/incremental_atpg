#include "lazy_set_cover.h"
#include "gtest/gtest.h"

#include <ctime>
#include <memory>
#include <stdint.h>
#include <set>

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

  class LazySetCoverTest : public testing::Test {
  protected:
    LazySetCoverTest() { 
      log4cxx::BasicConfigurator::resetConfiguration();
      log4cxx::BasicConfigurator::configure();
    }

    virtual void SetUp() {
      sc_.reset(new LazySetCover);
    }
    virtual void TearDown() {
    }
    std::unique_ptr<LazySetCover> sc_;
  };


  TEST_F(LazySetCoverTest, UpdateCover) {
        sc_->AddRule({"dog"});
        sc_->UpdateCover();
	EXPECT_EQ(1, sc_->cover_->size());
	sc_->AddRule({"dog", "cat"});
        sc_->UpdateCover();
	EXPECT_EQ(1, sc_->cover_->size());
	sc_->AddRule({"rain"});
        sc_->UpdateCover();
	EXPECT_EQ(2, sc_->cover_->size());
	EXPECT_EQ("dog", sc_->rule_processing_infos_->at(0).first_covered_by);
	EXPECT_EQ("dog", sc_->rule_processing_infos_->at(1).first_covered_by);
	EXPECT_EQ("rain", sc_->rule_processing_infos_->at(2).first_covered_by);
	sc_->AddRule({"rain"});
        sc_->UpdateCover();
	EXPECT_EQ(2, sc_->cover_->size());
	EXPECT_EQ("dog", sc_->cover_->front());
	sc_->AddRule({"rain"});
        sc_->UpdateCover();
	EXPECT_EQ(2, sc_->cover_->size());
	EXPECT_EQ("rain", sc_->cover_->front());
	EXPECT_EQ("dog", sc_->cover_->back());
  }

  TEST_F(LazySetCoverTest, MakeCoverOrderMap) {
    sc_->cover_->push_back("cat");
    sc_->cover_->push_back("jellyfish");
    sc_->cover_->push_back("sand");
    sc_->MakeCoverOrderMap();
    EXPECT_EQ(3, sc_->cover_order_->size());
    EXPECT_TRUE(sc_->cover_order_->find("cat") != sc_->cover_order_->end());
    EXPECT_EQ(0, sc_->cover_order_->at("cat"));
    EXPECT_TRUE(sc_->cover_order_->find("jellyfish") != sc_->cover_order_->end());
    EXPECT_EQ(1, sc_->cover_order_->at("jellyfish"));
    EXPECT_TRUE(sc_->cover_order_->find("sand") != sc_->cover_order_->end());
    EXPECT_EQ(2, sc_->cover_order_->at("sand"));
  }

  TEST_F(LazySetCoverTest, WhereWouldSetGo) {
      {
	sc_->AddRule({"dog"});
	sc_->AddRule({"cat"});

	sc_->cover_->push_back("dog");
	sc_->cover_->push_back("cat");
	
	sc_->ResetProcessingInfo();
	sc_->AddRule({"dog", "rain"});
	uint64_t before_uncovered;
	EXPECT_FALSE(sc_->WhereWouldSetGo("dog", &before_uncovered));
	EXPECT_FALSE(sc_->WhereWouldSetGo("rain", &before_uncovered));
      }

      {
	sc_.reset(new LazySetCover);
	sc_->AddRule({"dog"});
	sc_->AddRule({"cat"});

	sc_->cover_->push_back("cat");
	sc_->cover_->push_back("dog");
	
	sc_->ResetProcessingInfo();
	sc_->AddRule({"dog", "rain"});

	uint64_t before_uncovered;
	// Since we only compare it to "dog". Maybe we should make
	// an exception and compare it to the first set too.
	EXPECT_TRUE(sc_->WhereWouldSetGo("dog", &before_uncovered));
	EXPECT_EQ(2, before_uncovered);
	// I need to fix my heuristic. Do I compare against
	// as many sets in cover as I can - e.g., "dog" has no
	// rule in comon except last one, "cat" has no rule
	// in common but it's the first set in the cover.
	// Also when I'm comparing I should take into account
	// that the other set may also have the last rule.
	EXPECT_FALSE(sc_->WhereWouldSetGo("rain", &before_uncovered));
      }
  }

  TEST_F(LazySetCoverTest, GetUnique) {
    vector<string> sets({"Gary", "Patrick", "Squidward", "Squidward", "Krusty",
	  "Sandy", "Sandy", "Plankton"}); 
    sc_->GetUnique(&sets);
    EXPECT_EQ(6, sets.size());
    EXPECT_EQ("Gary", sets.at(0));
    EXPECT_EQ("Squidward", sets.at(2));
    EXPECT_EQ("Krusty", sets.at(3));
    EXPECT_EQ("Sandy", sets.at(4));
    EXPECT_EQ("Plankton", sets.at(5));
    
 }
  TEST_F(LazySetCoverTest, SortByCoverOrder) {
    sc_->cover_->push_back("cat");
    sc_->cover_->push_back("jellyfish");
    sc_->cover_->push_back("squarepants");
    sc_->cover_->push_back("starward");

    sc_->MakeCoverOrderMap();

    vector<string> sets({"squarepants", "cat", "jellyfish", "starward"});
    sc_->SortByCoverOrder(&sets);
    EXPECT_EQ(4, sets.size());
    EXPECT_EQ("cat", sets.at(0));
    EXPECT_EQ("jellyfish", sets.at(1));
    EXPECT_EQ("squarepants", sets.at(2));
    EXPECT_EQ("starward", sets.at(3));
  }
  TEST_F(LazySetCoverTest, BetterThanSet) {
    sc_->AddRule({"dog"});
    sc_->AddRule({"cat"});
    
    sc_->cover_->push_back("dog");
    sc_->cover_->push_back("cat");

    sc_->ResetProcessingInfo();
    sc_->AddRule({"dog", "rain"});
    set<uint64_t> dog_uncovered_rules({0, 2});
    set<uint64_t> rain_uncovered_rules({2});
    uint64_t before_uncovered = 0;
    
    EXPECT_FALSE(sc_->BetterThanSet("dog", &dog_uncovered_rules, &before_uncovered));
    EXPECT_EQ(0, dog_uncovered_rules.size());
    EXPECT_FALSE(sc_->BetterThanSet("cat", &dog_uncovered_rules, &before_uncovered));
  }

  TEST_F(LazySetCoverTest, GetBestSetToMoveUp) { 
    sc_->AddRule({"dog"});
    pair<string, uint64_t> best_move_up;
    sc_->GetBestSetToMoveUp(&best_move_up);
    EXPECT_EQ("dog", best_move_up.first);
    EXPECT_EQ(0, best_move_up.second);
    sc_->cover_->push_back("dog");
    sc_->ResetProcessingInfo();

    sc_->AddRule({"dog", "rain"});
    best_move_up = make_pair("", 0);
    sc_->GetBestSetToMoveUp(&best_move_up);
    EXPECT_EQ("", best_move_up.first);
    EXPECT_EQ(0, best_move_up.second);
    sc_->ResetProcessingInfo();

    sc_->AddRule({"rain"});
    best_move_up = make_pair("", 0);
    sc_->GetBestSetToMoveUp(&best_move_up);
    EXPECT_EQ("rain", best_move_up.first);
    EXPECT_EQ(0, best_move_up.second);

}
  TEST_F(LazySetCoverTest, InsertNewSet) { 
    {    
      sc_->AddRule({"dog"});
      string name = sc_->InsertNewSet(make_pair("dog", 0));
      EXPECT_EQ(1, sc_->cover_->size());
      EXPECT_EQ(name, sc_->cover_->back());
      EXPECT_TRUE(sc_->cover_order_->find(name) !=
		  sc_->cover_order_->end());
      EXPECT_EQ(0, sc_->cover_order_->at(name));
    }
    {
      sc_.reset(new LazySetCover);
      sc_->AddRule({"dog"});
      sc_->AddRule({"dog", "cat"});
      sc_->cover_->push_back("dog");
      sc_->cover_->push_back("cat");
      sc_->ResetProcessingInfo();
      sc_->AddRule({"rain"});
      string name = sc_->InsertNewSet(make_pair("rain", 2));
      EXPECT_EQ(3, sc_->cover_->size());
      EXPECT_EQ(name, sc_->cover_->front());
      EXPECT_TRUE(sc_->cover_order_->find(name) !=
		  sc_->cover_order_->end());
      EXPECT_EQ(0, sc_->cover_order_->at(name));
      EXPECT_EQ(0, sc_->cover_order_->at("dog"));
    }
}
  /*
  TEST_F(LazySetCoverTest, FirstSetThatCoversLastRule) { }
  TEST_F(LazySetCoverTest, UpdateCoverRules) { }
  TEST_F(LazySetCoverTest, FixNumUncoveredUsingCoverRules) { }
  */
  TEST_F(LazySetCoverTest, CleanUpEmptySets) { 
    sc_->AddRule({"dog"});
    sc_->AddRule({"cat", "dog"});
    
    sc_->cover_->push_back("dog");
    sc_->cover_->push_back("cat");

    sc_->ResetProcessingInfo();    
    EXPECT_EQ(2, sc_->set_processing_infos_->size());
    EXPECT_EQ(0, sc_->set_processing_infos_->at("cat").GetNumRules());
    // Can put any set in empty sets.
    set<string> empty_sets({"dog"});
    sc_->CleanUpEmptySets(empty_sets);
    EXPECT_EQ(1, sc_->cover_->size());
    EXPECT_EQ("cat", sc_->cover_->back());
    EXPECT_EQ(1, sc_->set_processing_infos_->size());
    EXPECT_EQ(1, sc_->cover_order_->size());
    EXPECT_EQ(2, sc_->set_infos_->size());
}

  TEST_F(LazySetCoverTest, ChangeSetName) {
    sc_->AddRule({"dog"});
    sc_->AddRule({"cat", "dog"});
    
    sc_->cover_->push_back("dog");
    sc_->cover_->push_back("cat");

    sc_->ResetProcessingInfo();    
    EXPECT_EQ(2, sc_->set_processing_infos_->size());
    EXPECT_EQ(0, sc_->set_processing_infos_->at("cat").GetNumRules());

    sc_->ChangeSetName("dog", "pig");
    EXPECT_TRUE(sc_->set_infos_->find("dog") == sc_->set_infos_->end());
    EXPECT_TRUE(sc_->set_infos_->find("pig") != sc_->set_infos_->end());
    EXPECT_EQ("pig", sc_->rule_processing_infos_->at(0).first_covered_by);
    EXPECT_EQ("pig", sc_->rule_processing_infos_->at(1).first_covered_by);
    // But rule_infos is not changed.
}

}  // namespace incremental_atpg
