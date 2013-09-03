#ifndef INCREMENTAL_ATPG_SET_COVER_H_
#define INCREMENTAL_ATPG_SET_COVER_H_
#include <vector>
#include <set>
#include <string>
#include <memory>
#include <map>
#include <list>
#include <stdint.h>
#include <log4cxx/logger.h>

#include "gtest/gtest_prod.h"

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::map;
  using std::list;
  using std::set;
  using log4cxx::LoggerPtr;
  using log4cxx::Logger;
  using log4cxx::Level;

  struct SetInfo {
    // Updated only every new rule.
    vector<uint64_t> all_rules; 
    void AddRule(uint64_t new_rule) {
      all_rules.push_back(new_rule);
    }
  };
  struct SetProcessingInfo {
    // Maintained by the set cover algorithms for sets in cover.
    // Number of rules not in cover, that this set covers.
    set<uint64_t> covers_rules;
    // Number of uncovered rules just before this set
    // was added to cover.
    uint64_t num_uncovered;
    void AddRule(uint64_t new_rule) {
      covers_rules.insert(new_rule);
    }
    void RemoveRule(uint64_t rule) {
      covers_rules.erase(rule);
    }
    uint64_t GetNumRules() const {
      return covers_rules.size();
    }
    set<uint64_t> GetRules() const {
      return covers_rules;
    }
  };

  struct RuleInfo {
    const vector<string> all_sets;
  RuleInfo(const vector<string>& all_sets)
  : all_sets(all_sets) { }
  };

  struct RuleProcessingInfo {
    // Used by set cover algorithm.
    // Maintained for all rules, to keep track
    // of which ones have been covered already.
    RuleProcessingInfo() 
    : first_covered_by("") { }
    RuleProcessingInfo(const string& first_set) 
    : first_covered_by(first_set) { }

    string first_covered_by;
  };

  class SetCover {
  public:
    log4cxx::LoggerPtr set_cover_logger;
    SetCover() 
      : set_infos_(new map<string, SetInfo>),
      rule_infos_(new vector<RuleInfo>),
      set_processing_infos_(new map<string, SetProcessingInfo>),
      rule_processing_infos_(new vector<RuleProcessingInfo>),
      cover_(new list<string>) {
      set_cover_logger = Logger::getLogger("SetCover");
      set_cover_logger->setLevel(log4cxx::Level::getWarn());

    }
    // Takes ownership of all @.._infos. Avoids copying when
    // we're switching between two kinds.
    SetCover(map<string, SetInfo>* set_infos,
	     vector<RuleInfo>* rule_infos,
	     map<string, SetProcessingInfo>* set_processing_infos,
	     vector<RuleProcessingInfo>* rule_processing_infos,
	     list<string>* cover)
      : set_infos_(set_infos),
      rule_infos_(rule_infos),
      set_processing_infos_(set_processing_infos),
      rule_processing_infos_(rule_processing_infos),
      cover_(cover) {
      set_cover_logger = Logger::getLogger("SetCover");
      set_cover_logger->setLevel(log4cxx::Level::getWarn());
      }      

    // For testing.
    explicit SetCover(map<string, SetInfo>* set_infos,
	     vector<RuleInfo>* rule_infos);

    // Update @set_infos_ and @rule_infos_ for new rule.
    void AddRule(const vector<string>& sets);
    list<string> GetCover() const;
    map<string, SetInfo> GetSetInfos() const;
    vector<RuleInfo> GetRuleInfos() const;
    map<string, SetProcessingInfo> GetSetProcessingInfos() const;
    vector<RuleProcessingInfo> GetRuleProcessingInfos() const;

    list<string>* ReleaseCover();
    map<string, SetInfo>* ReleaseSetInfos();
    vector<RuleInfo>* ReleaseRuleInfos();
    map<string, SetProcessingInfo>* ReleaseSetProcessingInfos();
    vector<RuleProcessingInfo>* ReleaseRuleProcessingInfos();

  protected:

    // Resets processing using @cover, @set_infos_ and @rule_infos.
    void ResetProcessingInfo();
    // Removes sets which don't cover new rules from cover.
    // Also includes duplicates.
    void RemoveEmptySets();
    // Data.
    unique_ptr<map<string, SetInfo> > set_infos_;
    unique_ptr<vector<RuleInfo> > rule_infos_;
    unique_ptr<map<string, SetProcessingInfo> > set_processing_infos_;
    unique_ptr<vector<RuleProcessingInfo> > rule_processing_infos_;
    unique_ptr<list<string> > cover_;
  private:
    friend class SetCoverTest;
    FRIEND_TEST(SetCoverTest, SetUp);
    FRIEND_TEST(SetCoverTest, ResetProcessingInfo);
  };
}  // namespace incremental_atpg
#endif  // INCREMENTAL_ATPG_SET_COVER_H_
