#include "set_cover.h"

#include <set>
#include <vector>
#include <string>
#include <stdlib.h>
#include <memory>
#include <list>
#include <log4cxx/logger.h>

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::make_pair;
  using std::list;
  using std::set;
  using log4cxx::LoggerPtr;
  using log4cxx::Logger;
  using log4cxx::Level;

  // Takes ownership of @set_infos, @rule_infos.
  SetCover::SetCover(map<string, SetInfo>* set_infos,
		     vector<RuleInfo>* rule_infos)
    : set_infos_(set_infos),
      rule_infos_(rule_infos),
      set_processing_infos_(new map<string, SetProcessingInfo>),
      rule_processing_infos_(new vector<RuleProcessingInfo>),
      cover_(new list<string>) {
    set_cover_logger = Logger::getLogger("SetCover");
    set_cover_logger->setLevel(log4cxx::Level::getWarn());
    set_cover_logger->setLevel(log4cxx::Level::getWarn());
  }

  void SetCover::AddRule(const vector<string>& sets) {
    uint64_t new_rule = rule_infos_->size();
    rule_infos_->push_back(sets);
    map<string, SetInfo>::iterator it;
    for (auto const& set_id : sets) {
      if ((it = set_infos_->find(set_id)) != set_infos_->end()) {
	it->second.AddRule(new_rule);
      } else {
	SetInfo info;
	info.AddRule(new_rule);
	set_infos_->insert(make_pair(set_id, info));
      }
    }
  }

  map<string, SetInfo> SetCover::GetSetInfos() const {
    return *set_infos_.get();
  }

  vector<RuleInfo> SetCover::GetRuleInfos() const {
    return *rule_infos_.get();
  }

  map<string, SetProcessingInfo> SetCover::GetSetProcessingInfos() const {
    return *set_processing_infos_.get();
  }

  vector<RuleProcessingInfo> SetCover::GetRuleProcessingInfos() const {
    return *rule_processing_infos_.get();
  }

  list<string> SetCover::GetCover() const {
    return *cover_.get();
  }

  void SetCover::ResetProcessingInfo() {
    set_processing_infos_.reset(new map<string, SetProcessingInfo>);
    rule_processing_infos_.reset(new vector<RuleProcessingInfo>);
    vector<string>::iterator set_id;
    uint64_t num_uncovered_rules = rule_infos_->size();

    for (uint64_t i = 0; i < num_uncovered_rules; i++) {
      RuleProcessingInfo uncovered_rule;
      rule_processing_infos_->push_back(uncovered_rule);
    }

    if (cover_->size() == 0) {
      LOG4CXX_WARN(set_cover_logger,
		   "Cover is empty.");
      return;
    }

    map<string, SetInfo>::iterator sIt;
    map<string, SetProcessingInfo>::iterator spIt;

    for (auto const& set_id : *cover_.get()) {
      if ((sIt = set_infos_->find(set_id))
	  == set_infos_->end()) {
	LOG4CXX_WARN(set_cover_logger, "Set " << set_id << " not in set_infos_.");
	continue;
      }
      // Check we haven't processed this already.
      if ((spIt = set_processing_infos_->find(set_id))
	  != set_processing_infos_->end()) {
	LOG4CXX_INFO(set_cover_logger, "Set " << set_id << " duplicate in cover.");
	continue;
      } else {
	SetProcessingInfo sp;
	sp.num_uncovered = num_uncovered_rules;
	spIt = set_processing_infos_->insert(make_pair(set_id, sp)).first;
      }

      for (auto const& rule_id : sIt->second.all_rules) {
	if (rule_id >= rule_infos_->size()) {
	  LOG4CXX_WARN(set_cover_logger, "Rule " << rule_id << " not in infos_.");
	  continue;
	}
	RuleProcessingInfo rule_info(set_id);
	if (!rule_processing_infos_->at(rule_id).first_covered_by.empty()){
	  LOG4CXX_INFO(set_cover_logger, "Rule " << rule_id 
		       << " already covered before set " << set_id);
	} else {
	  rule_processing_infos_->at(rule_id).first_covered_by = set_id;
	  spIt->second.AddRule(rule_id);
	  --num_uncovered_rules;
	}
      }
    }		   
  }
  }  // namespace incremental_atpg
