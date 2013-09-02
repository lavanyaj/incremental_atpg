#ifndef INCREMENTAL_ATPG_LAZY_SET_COVER_H_
#define INCREMENTAL_ATPG_LAZY_SET_COVER_H_
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

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::map;
  using std::make_pair;
  using std::pair;
  using std::set;

  class LazySetCover : public SetCover {
  public:
    log4cxx::LoggerPtr lazy_set_cover_logger;
    LazySetCover();
    LazySetCover(map<string, SetInfo>* set_infos,
		     vector<RuleInfo>* rule_infos);
    // AddRule inherited from SetCover.
    // Get..ProcessingInfo also.

    // Finds set cover to cover latest rule added.
    // @cover_ should cover all rules up to last one.
    // @set_processing_infos_ and @rule_processing_infos_ should
    // be correct up to last rule added.
    // @set_infos and @rule_infos_ contain all info through last rule.
  void UpdateCover();

  protected:

  // Need @cover_order_, @rule_processing_infos_ @set_processing_infos_ up to last rule
  // and @set_infos, @rule_infos through last rule.
  // Populates @before_uncovered with the number of uncovered rules, 
  // at which point, set_name should
  // be inserted in current cover. According to the heuristic, this is the
  // earliest position in the cover, where any existing set in cover has rules
  // in common with @set_name and would cover fewer new rules than @set_name,
  // were it to be inserted instead. Returns false in case of error or if 
  // there's no such position.
  bool WhereWouldSetGo(const string& set_name, uint64_t* before_uncovered);

  // Populate @cover_order_ with sets in cover and their index.
  void MakeCoverOrderMap();

  // Return true if order[@lhs] < order[@rhs].
  // Both should be in @order.
  bool CompareUsingMap(const string& lhs, const string& rhs,
		       const map<string, uint64_t>& order);
  
  // Sorts @sets in ascending order by index of set in @cover_order_.
  bool SortByCoverOrder(vector<string>* sets);

  // Remove duplicates from @sets.
  void GetUnique(vector<string>* sets);

  // Returns true, if @uncovered_rules has more rules than @other_set_name's
  // cover_rules (plus one, if @set_infos_ indicates it also has the last rule added.)
  // Then fills in @before_uncovered with uncovered rules when @other_set_name
  // was added to cover (0 if it's not in cover, shouldn't happen will log warning.)
  // Otherwise removes common rules from @uncovered_rules and returns false.
  bool BetterThanSet(const string& other_set_name, set<uint64_t>* uncovered_rules,
		     uint64_t* before_uncovered);

  /////////////////////////////////////////////////////////////////

  // Finds best set to move up, to cover last rule added.
  void GetBestSetToMoveUp(pair<string, uint64_t>* best_move_up);

  // Makes a copy of @best_move_up.first and inserts in cover
  // when there are @best_move_up.second rules to cover.
  // Updates @set_infos_, @set_processing_infos_, @cover_
  // and @cover_order_.
  // Returns name of copy.
  string InsertNewSet(pair<string, uint64_t> best_move_up);

  // Iterates through cover, referring to set_processing_infos_
  // and rules_info_ to return the name of the first set that covers
  // the latest rule added.
  string FirstSetThatCoversLastRule();

  // Updates @cover_rules for sets in @set_processing_infos
  // and @first_covered_by for rules in @rule_processing_infos
  // given that @last_rule_covered_by is the first set
  // to cover the last rule. Uses @cover_order_ to find
  // relative order of sets. 
  void UpdateCoverRules(string last_rule_covered_by);

  // Iterates through cover, and resets num_uncovered starting
  // with rule_infos_->size() and decreasing it by number of 
  // rules in each set's @cover_rules. Fills @empty_sets
  // with names of set that don't cover any new rules.
  void FixNumUncoveredUsingCoverRules(set<string>* empty_sets);

  // Remove @empty_sets from @cover_, @cover_order_ and @set_processing_infos
  // They can't be in @rule_processing_infos_ obviously. TODO(lav): sanity check.
  void CleanUpEmptySets(const set<string>& empty_sets);

  // Change set name in @set_infos, @..processing etc.
  void ChangeSetName(const string& tmp_set_name,
		     const string& real_set_name);

  // Include @cover_order_ and @.._processing_infos_
  void ResetProcessingInfo();
  unique_ptr<map<string, uint64_t> > cover_order_;
  private:
    friend class LazySetCoverTest;
    FRIEND_TEST(LazySetCoverTest, UpdateCover);
    FRIEND_TEST(LazySetCoverTest, BetterThanSet);
    FRIEND_TEST(LazySetCoverTest, WhereWouldSetGo);
    FRIEND_TEST(LazySetCoverTest, MakeCoverOrderMap);
    FRIEND_TEST(LazySetCoverTest, SortByCoverOrder);
    FRIEND_TEST(LazySetCoverTest, GetUnique);
    FRIEND_TEST(LazySetCoverTest, GetBestSetToMoveUp);
    FRIEND_TEST(LazySetCoverTest, InsertNewSet);
    FRIEND_TEST(LazySetCoverTest, FirstSetThatCoversLastRule);
    FRIEND_TEST(LazySetCoverTest, UpdateCoverRules);
    FRIEND_TEST(LazySetCoverTest, FixNumUncoveredUsingCoverRules);
    FRIEND_TEST(LazySetCoverTest, CleanUpEmptySets);
    FRIEND_TEST(LazySetCoverTest, ChangeSetName);
  };
}  // namespace incremental_atpg
#endif  // INCREMENTAL_ATPG_LAZY_SET_COVER_H_
