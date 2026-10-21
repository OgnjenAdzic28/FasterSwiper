#include "src/engine/space-switch-operation.h"

#include "src/engine/axis-adapter.h"
#include "src/engine/const.h"
#include "src/macos-private.h"

#include <memory>
#include <vector>

#include <gtest/gtest.h>

namespace fasterswiper {
namespace {

// Record events in memory. These tests never post gestures to WindowServer.
class RecordingSink : public CGEventSink {
public:
  void Post(CGEventRef event) override {
    phases.push_back(CGEventGetIntegerValueField(event, kCGEventGesturePhase));
  }

  std::vector<int64_t> phases;
};

auto CreateOperation() {
  // This adapter has a fixed origin and does not query the current desktop.
  return ContinuousSpaceSwitchOperation::Create(
      std::make_unique<AppExposeHorizontalAxisAdapter>(),
      {.natural_scrolling_enabled = false});
}

TEST(SpaceSwitchOperationTest, AnUnstartedSwitchDoesNotEmitCancellation) {
  auto result = CreateOperation();
  ASSERT_TRUE(result.ok()) << result.status();
  RecordingSink sink;
  (*result)->Commit(&sink);
  EXPECT_TRUE(sink.phases.empty());
}

TEST(SpaceSwitchOperationTest, StayingAtTheCurrentPositionDoesNotEmitEvents) {
  auto result = CreateOperation();
  ASSERT_TRUE(result.ok()) << result.status();
  RecordingSink sink;
  (*result)->SetPosition(0, &sink);
  (*result)->Commit(&sink);
  EXPECT_TRUE(sink.phases.empty());
}

TEST(SpaceSwitchOperationTest, ReturningAfterMovementStillCancelsTheGesture) {
  auto result = CreateOperation();
  ASSERT_TRUE(result.ok()) << result.status();
  RecordingSink sink;
  (*result)->SetPosition(kOneSwipeInNanoswipes / 2, &sink);
  (*result)->SetPosition(0, &sink);
  (*result)->Commit(&sink);
  ASSERT_FALSE(sink.phases.empty());
  EXPECT_EQ(sink.phases.front(), kGestureBegan);
  EXPECT_EQ(sink.phases.back(), kGestureCancelled);
}

TEST(SpaceSwitchOperationTest, ACompletedSwitchEndsOnlyOnce) {
  auto result = CreateOperation();
  ASSERT_TRUE(result.ok()) << result.status();
  RecordingSink sink;
  (*result)->SetPosition(kOneSwipeInNanoswipes, &sink);
  (*result)->Commit(&sink);
  const auto first_commit = sink.phases;
  (*result)->Commit(&sink);
  EXPECT_EQ(sink.phases, first_commit);
  ASSERT_FALSE(sink.phases.empty());
  EXPECT_EQ(sink.phases.front(), kGestureBegan);
  EXPECT_EQ(sink.phases.back(), kGestureEnded);
}

}  // namespace
}  // namespace fasterswiper
