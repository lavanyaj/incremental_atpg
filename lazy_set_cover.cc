#include "lazy_set_cover.h"

#include <vector>
#include <cstring>
#include <string>
#include <memory>
#include <map>
#include <list>
#include <algorithm> // for std::sort
#include <stdint.h>
#include <utility>
#include <log4cxx/logger.h>

#include "gtest/gtest_prod.h"
#include "set_cover.h"

namespace incremental_atpg {
  using std::vector;
  using std::list;
  using std::string;
  using std::strcmp;
  using std::unique_ptr;
  using std::map;
  using std::make_pair;
  using std::pair;
  using std::bind;
  using std::sort;
  using std::unique;

  using log4cxx::LoggerPtr;
  using log4cxx::Logger;
  using log4cxx::Level;

  LazySetCover::LazySetCover()
    : cover_order_ (new map<string, uint64_t>) {
    lazy_set_cover_logger = Logger::getLogger("LazySetCover");
    lazy_set_cover_logger->setLevel(log4cxx::Level::getWarn());
  }

  LazySetCover::LazySetCover(map<string, SetInfo>* set_infos,
			       vector<RuleInfo>* rule_infos)
    : SetCover(set_infos, rule_infos),
      cover_order_ (new map<string, uint64_t>) {
    lazy_set_cover_logger = Logger::getLogger("LazySetCover");
    lazy_set_cover_logger->setLevel(log4cxx::Level::getWarn());
  }

  LazySetCover::LazySetCover(map<string, SetInfo>* set_infos,
			     vector<RuleInfo>* rule_infos,
			     map<string, SetProcessingInfo>* set_processing_infos,
			     vector<RuleProcessingInfo>* rule_processing_infos,
			     list<string>* cover)
    : SetCover(set_infos, rule_infos, set_processing_infos, 
	       rule_processing_infos, cover),
      cover_order_ (new map<string, uint64_t>) {
    lazy_set_cover_logger = Logger::getLogger("LazySetCover");
    lazy_set_cover_logger->setLevel(log4cxx::Level::getWarn());
  }  


  string LazySetCover::InsertNewSet(pair<string, uint64_t> best_move_up) {
    if (best_move_up.first.empty()) {
      LOG4CXX_ERROR(lazy_set_cover_logger, "Set name to insert, is empty.");
      return "";
    }

    string tmp_set_name = best_move_up.first + string("_tmp");

    // Insert in set_infos_
    SetInfo tmp_info = set_infos_->at(best_move_up.first);
    set_infos_->insert(make_pair(tmp_set_name, tmp_info));

    // Insert in set_processing_infos_
    SetProcessingInfo tmp_sp;
    for (auto rule : tmp_info.all_rules) {
      if (rule != rule_infos_->size() - 1) {
	tmp_sp.AddRule(rule);
      }
    }
    set_processing_infos_->insert(make_pair(tmp_set_name, tmp_sp));

    // Insert in cover_ and cover_order_
    if (best_move_up.second == 0) {
      cover_order_->insert(make_pair(tmp_set_name, cover_->size()));
      cover_->push_back(tmp_set_name);
    } else {
      uint64_t order = 0;
      list<string>::iterator it;
      for (it = cover_->begin(); it != cover_->end(); it++) {
	if (set_processing_infos_->at(*it).num_uncovered 
	    == best_move_up.second) {
	  cover_->insert(it, tmp_set_name);
	  cover_order_->insert(make_pair(tmp_set_name, order + 0.5));
	  break;
	}
	++order;
      }
    }
    return tmp_set_name;
    
  }

  string LazySetCover::FirstSetThatCoversLastRule() {
    string first_set_that;
    set<string> last_rule_in_sets = 
      set<string>(rule_infos_->back().all_sets.begin(),
		  rule_infos_->back().all_sets.end());
    
    for (auto set_name : *cover_.get()) {
      if (last_rule_in_sets.find(set_name) != 
	  last_rule_in_sets.end()) {
	first_set_that = set_name;
	break;
      }
    }
    if (first_set_that.empty()) {
      LOG4CXX_ERROR(lazy_set_cover_logger, "Can't find any set in cover with last rule.");
    }
    return first_set_that;
  }

  void LazySetCover::ResetProcessingInfo() {
      MakeCoverOrderMap();
      SetCover::ResetProcessingInfo();
  }
  void LazySetCover::UpdateCoverRules(string last_rule_covered_by) {
    uint64_t last_rule = rule_infos_->size() - 1;
    if (set_processing_infos_->find(last_rule_covered_by)
	== set_processing_infos_->end()) {
      LOG4CXX_ERROR(lazy_set_cover_logger, "Set not in @set_processing_infos_");
      return;
    }
    SetProcessingInfo& sp = set_processing_infos_->operator[](last_rule_covered_by);
    sp.AddRule(last_rule);

    if (cover_order_->find(last_rule_covered_by)
	== cover_order_->end()) {
	LOG4CXX_ERROR(lazy_set_cover_logger, "Set "
		      << last_rule_covered_by << " not in @cover_order_");
	return;
      } 

    // Given that this is the first rule to cover last rule      
    rule_processing_infos_->operator[](last_rule) = 
      RuleProcessingInfo(last_rule_covered_by);


    for (auto rule_id : sp.GetRules()) {
      if (rule_id >= rule_processing_infos_->size()) {
	LOG4CXX_ERROR(lazy_set_cover_logger, "Rule " 
		      << rule_id << " not in @rule_processing_infos_");
	continue;
    }

      const string& now_covered_by = rule_processing_infos_->at(rule_id).first_covered_by;
      if (cover_order_->find(now_covered_by)
	== cover_order_->end()) {
	LOG4CXX_ERROR(lazy_set_cover_logger, "Set not in @cover_order_");
	continue;
      } 
      if (now_covered_by != last_rule_covered_by) {
	if (cover_order_->at(now_covered_by) < cover_order_->at(last_rule_covered_by)) {
	  sp.RemoveRule(rule_id);
	} else {
	  if (set_processing_infos_->find(now_covered_by)
	      == set_processing_infos_->end()) {
	    LOG4CXX_ERROR(lazy_set_cover_logger, "Set " << now_covered_by
			  << " not in @set_processing_infos_");
	    continue;
	  } 

	  SetProcessingInfo& other_sp = set_processing_infos_->operator[](now_covered_by);
	  other_sp.RemoveRule(rule_id);
	  rule_processing_infos_->operator[](rule_id).first_covered_by
	    = last_rule_covered_by;
	}
      }
    }
  }

  void LazySetCover::FixNumUncoveredUsingCoverRules(set<string>* empty_sets) {
    empty_sets->clear();
    uint64_t num_uncovered = rule_infos_->size();
    for (auto set_name : *cover_) {
        SetProcessingInfo& tmp = set_processing_infos_->operator[](set_name);
	tmp.num_uncovered = num_uncovered;
	num_uncovered -= tmp.GetNumRules();
	if (tmp.GetNumRules() == 0) {
	  empty_sets->insert(set_name);
	}
    }
  }

  void LazySetCover::CleanUpEmptySets(const set<string>& empty_sets) {
      using std::placeholders::_1;
      auto check_empty = [&] (const string& set_name) { 
	return empty_sets.find(set_name) != empty_sets.end();};
      cover_->remove_if(check_empty);
      for (auto set_name : empty_sets) {
	set_processing_infos_->erase(set_name);
	cover_order_->erase(set_name);
      }
  }

  void LazySetCover::ChangeSetName(const string& tmp_set_name,
				   const string& real_set_name) {

    // Change in cover. Make sure cover doesn't have @real_set_name.
    list<string>::iterator it;
    for (it = cover_->begin(); it != cover_->end(); it++) {
      if (*it == tmp_set_name) {
	break;
      }
    }
    auto new_it = cover_->insert(it, real_set_name);
    cover_->erase(++new_it);

    // Change/ replace in @set_processing_infos_.
    SetProcessingInfo sp = set_processing_infos_->at(tmp_set_name);
    set_processing_infos_->erase(tmp_set_name);
    set_processing_infos_->operator[](real_set_name) = sp;

    // Change in @rule_processing_infos_.
    for (auto rule_id: sp.GetRules()) {
      // TODO(lav): Check it was tmp_set_name before.
      rule_processing_infos_->operator[](rule_id)
	= RuleProcessingInfo(real_set_name);
    }

    // Change/ replace in @set_infos_.
    SetInfo info = set_infos_->at(tmp_set_name);
    set_infos_->erase(tmp_set_name);
    set_infos_->operator[](real_set_name) = info;

    // Change/ replace in @cover_order_.
    double order = cover_order_->at(tmp_set_name);
    cover_order_->erase(tmp_set_name);
    cover_order_->operator[](real_set_name) = order;
  }

  void LazySetCover::GetBestSetToMoveUp(pair<string, uint64_t>* best_move_up) {
    const vector<string>& move_up_sets = rule_infos_->back().all_sets;
    *best_move_up = make_pair("", 0);
    uint64_t before_uncovered = 0;
    for (auto set_name : move_up_sets) {
      if (WhereWouldSetGo(set_name, &before_uncovered)) {
	if (best_move_up->first.empty() ||
	    (before_uncovered > best_move_up->second) ||
	    (before_uncovered == best_move_up->second &&
	     strcmp(best_move_up->first.c_str(), set_name.c_str()) < 0)) {
	  best_move_up->first = set_name;
	  best_move_up->second = before_uncovered;
	}
      }
    }
  }

  void LazySetCover::UpdateCover() { 
    if (rule_infos_->size() == 0) {
      LOG4CXX_WARN(lazy_set_cover_logger, "No rule yet.");
      return;
    }
    MakeCoverOrderMap();
    string last_rule_covered_by;

    pair<string, uint64_t> best_move_up;
    GetBestSetToMoveUp(&best_move_up);
    
    if (!best_move_up.first.empty()) {
    // Insert temp. set if some set can be moved up.
      last_rule_covered_by = InsertNewSet(best_move_up);
    } else {
      // If cover is unchanged, find first set that covers latest rule.
      last_rule_covered_by = FirstSetThatCoversLastRule();
    }
    LOG4CXX_INFO(lazy_set_cover_logger, "Should cover with " 
		 << last_rule_covered_by);

    if (last_rule_covered_by.empty()) {
      LOG4CXX_ERROR(lazy_set_cover_logger, "Can't find set to cover last rule.");
      return;
    }

    // Update processing info. for last rule.
    rule_processing_infos_->push_back(RuleProcessingInfo(last_rule_covered_by));

    // Update covers_rules for last_rule_covered_by and related sets.
    UpdateCoverRules(last_rule_covered_by);

    LOG4CXX_INFO(lazy_set_cover_logger, "Updated Cover Rules.");

    // Fix num_uncovered in set_processing_infos_ using covers_rules.
    set<string> empty_sets;
    FixNumUncoveredUsingCoverRules(&empty_sets);

    LOG4CXX_INFO(lazy_set_cover_logger, "Fixed NumUncovered using Cover Rules.");

    CleanUpEmptySets(empty_sets);
      
    LOG4CXX_INFO(lazy_set_cover_logger, "Cleaned Up Empty Sets.");
    // Change tmp_name back to regular name
    // In cover, set_infos, set_processing_infos, cover_order.
    if (!best_move_up.first.empty()) {
      ChangeSetName(last_rule_covered_by, best_move_up.first);
    }

  }
  void LazySetCover::MakeCoverOrderMap() { 
    cover_order_.reset(new map<string, uint64_t>);
    uint64_t order = 0;
    for (auto const& set_name : *cover_.get()) {
      cover_order_->insert(make_pair(set_name, order));
      ++order;
    }
  }

  bool LazySetCover::WhereWouldSetGo(const string& set_name, uint64_t* before_uncovered) { 
    vector<string> covered_by_sets;
    if (set_infos_->find(set_name) == set_infos_->end()) {
      LOG4CXX_ERROR(lazy_set_cover_logger, 
		    "Set " << set_name << " not in set_infos_.");
      return false;
    }

    // Always compare with the first set. TODO(lav): why.
    if (cover_->size() > 0) {
      covered_by_sets.push_back(cover_->front());
    }

    SetInfo info = set_infos_->at(set_name);
    for (auto const& rule_id : info.all_rules) {
      if (rule_processing_infos_->size() <= rule_id
	  || rule_processing_infos_->operator[](rule_id).first_covered_by.empty()) {
	if (rule_id != rule_infos_->size() - 1) {
	  LOG4CXX_ERROR(lazy_set_cover_logger, "Rule " 
		       << rule_id << " not covered/ in processing_..");
	}
	continue;
	
      }
      covered_by_sets.push_back(rule_processing_infos_->operator[](rule_id).first_covered_by);
    }
    // Compare with all the other sets that contain new rule? If they're in cover.
    for (auto const& other_set_with_new_rule : rule_infos_->back().all_sets) {
      if (set_processing_infos_->find(other_set_with_new_rule)
	  != set_processing_infos_->end()) {
	covered_by_sets.push_back(other_set_with_new_rule);
      }
    }

    if (cover_order_->size() != cover_->size()) {
      LOG4CXX_WARN(lazy_set_cover_logger, "Cover has " << cover_->size()
		      << " sets, cover_order_ has " << cover_order_->size()
		      << ". MakeCoverOrderMap().");
      MakeCoverOrderMap();
    }

    if (!SortByCoverOrder(&covered_by_sets)) {
      LOG4CXX_ERROR(lazy_set_cover_logger, "Couldn't sort covering sets.");
      return false;
    }

    GetUnique(&covered_by_sets);

    *before_uncovered = 0;
    set<uint64_t> uncovered_rules(info.all_rules.cbegin(), info.all_rules.cend());
    for (auto const& other_set_name : covered_by_sets) {
      LOG4CXX_INFO(lazy_set_cover_logger, "Comparing " << set_name
		   << "(" << uncovered_rules.size() << ") vs " << other_set_name);
      if (BetterThanSet(other_set_name, &uncovered_rules, before_uncovered)) {
    	return true;
      }
    }
    // If it's not better than any set in cover, but it still 
    // has at least one rule by the end, insert at end, when before_uncovered is 0.
    if (uncovered_rules.size() > 0) {   
      return true;
    }

    // TODO(lav): not an error though.
    return false;
  }

  bool LazySetCover::CompareUsingMap(const string& lhs,
				     const string& rhs,
				     const map<string, uint64_t>& order) {
    if (order.find(lhs) == order.end() ||
	order.find(rhs) == order.end()) {
      return strcmp(lhs.c_str(), rhs.c_str()) < 0;
    } else {
      return order.at(lhs) < order.at(rhs);
    }
  }

  // Sorts @sets by index of set in @cover_order_.
  bool LazySetCover::SortByCoverOrder(vector<string>* sets) { 
    // Check all the sets are in cover_order_
    for (auto const& set_name : *sets) {
      if (cover_order_->find(set_name) == cover_order_->end()) {
	LOG4CXX_ERROR(lazy_set_cover_logger, "Set " << set_name
		     << " not in cover_order_.");
	return false;
      }
    }
      using std::placeholders::_1;
      using std::placeholders::_2;

      auto compare_callback = bind(&LazySetCover::CompareUsingMap, this,
				   _1, _2, *cover_order_.get());
      sort(sets->begin(), sets->end(), compare_callback);
      return true;
  }

  // Remove duplicates from @sets. @sets should be sorted.
  void LazySetCover::GetUnique(vector<string>* sets) { 
    auto new_end = unique(sets->begin(), sets->end());
    sets->resize(std::distance(sets->begin(), new_end));
  }

  // Returns true, if @uncovered_rules has more rules than @other_set_name's
  // covers_rules (plus one, if @set_infos_ indicates it also has the last rule added.)
  // Otherwise removes common rules from @uncovered_rules and returns false.
  bool LazySetCover::BetterThanSet(const string& other_set_name, 
				   set<uint64_t>* uncovered_rules,
				   uint64_t* before_uncovered) { 
    if (set_processing_infos_->find(other_set_name)
	== set_processing_infos_->end()) {
      LOG4CXX_ERROR(lazy_set_cover_logger, "Other set " << other_set_name
		    << " not in set_processing_infos_, maybe not in cover too?");
      // TODO(lav): kind of arbitrary
      return false;
    }
    SetProcessingInfo info = set_processing_infos_->at(other_set_name);
    uint64_t other_set_covers = info.covers_rules.size();
    // Last rule info not added to processing yet so check rule_infos
    const vector<string>& last_rule_in = rule_infos_->back().all_sets;
    bool other_set_has_last_rule = false;
    if (find(last_rule_in.cbegin(), last_rule_in.cend(), other_set_name) 
	!= last_rule_in.cend()) {
      ++other_set_covers;
      other_set_has_last_rule = true;
    }
    LOG4CXX_INFO(lazy_set_cover_logger, "Other set " << other_set_name
		 << " covers " << other_set_covers << " and "
		 << "size of uncovered rules is " << uncovered_rules->size());
    if (uncovered_rules->size() > other_set_covers) {
      *before_uncovered = info.num_uncovered;
      return true;
    }
    // TODO(lav): use set to store uncovered rules.
    if (other_set_has_last_rule) {
      uncovered_rules->erase(rule_infos_->size()-1);
    }
    for (auto const& rule : info.covers_rules) {
      uncovered_rules->erase(rule);
    }
    return false;
  }

}  // namespace incremental_atpg

