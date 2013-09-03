#include "online_set_cover.h"
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

  class OnlineSetCoverTest : public testing::Test {
  protected:
    OnlineSetCoverTest() { 
      log4cxx::BasicConfigurator::resetConfiguration();
      log4cxx::BasicConfigurator::configure();
    }

    virtual void SetUp() {
      sc_.reset(new OnlineSetCover);
    }
    virtual void TearDown() {
    }
    std::unique_ptr<OnlineSetCover> sc_;
  };

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
    EXPECT_EQ("dog", sc_->cover_->front());

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"rain\"}");
    sc_->AddRule({"rain"});
    sc_->UpdateCover();
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("rain", sc_->cover_->front());
    EXPECT_EQ("dog", sc_->cover_->back());
  }

}  // namespace incremental_atpg
