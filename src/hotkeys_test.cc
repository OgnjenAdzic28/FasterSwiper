#include "src/hotkeys.h"

#include "src/cf-util.h"
#include "src/event.h"

#include <array>

#include <gtest/gtest.h>

namespace fasterswiper {
namespace {

CFUniquePtr<CFMutableDictionaryRef> MakeDictionary() {
  return WrapCFUnique(
      CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks));
}

CFUniquePtr<CFMutableDictionaryRef> RightArrowSettings(CGEventFlags modifiers,
                                                       bool enabled = true) {
  auto parameters =
      WrapCFUnique(CFArrayCreateMutable(nullptr, 0, &kCFTypeArrayCallBacks));
  for (int64_t value :
       std::array<int64_t, 3>{65535, 124, static_cast<int64_t>(modifiers)}) {
    auto number =
        WrapCFUnique(CFNumberCreate(nullptr, kCFNumberSInt64Type, &value));
    CFArrayAppendValue(parameters.get(), number.get());
  }

  auto value = MakeDictionary();
  CFDictionarySetValue(value.get(), CFSTR("parameters"), parameters.get());
  auto shortcut = MakeDictionary();
  CFDictionarySetValue(shortcut.get(), CFSTR("value"), value.get());
  CFDictionarySetValue(shortcut.get(), CFSTR("enabled"),
                       enabled ? kCFBooleanTrue : kCFBooleanFalse);
  auto settings = MakeDictionary();
  CFDictionarySetValue(settings.get(), CFSTR("81"), shortcut.get());
  return settings;
}

TEST(HotkeysTest, PreservesConfiguredModifierCombinations) {
  for (CGEventFlags modifiers : std::array<CGEventFlags, 5>{
           kCGEventFlagMaskControl, kCGEventFlagMaskAlternate,
           kCGEventFlagMaskCommand,
           kCGEventFlagMaskControl | kCGEventFlagMaskShift,
           kCGEventFlagMaskControl | kCGEventFlagMaskAlternate}) {
    SCOPED_TRACE(modifiers);
    auto settings = RightArrowSettings(modifiers);
    auto parsed = ParseHotkeyConfiguration(settings.get());
    ASSERT_TRUE(parsed.ok()) << parsed.status();

    const Hotkey& hotkey = parsed->move_space_right;
    EXPECT_EQ(hotkey.modifiers, modifiers);
    EXPECT_TRUE((KeyEvent{.key_code = 124, .modifiers = modifiers})
                    .ConcernsHotkey(hotkey));
    EXPECT_FALSE((KeyEvent{.key_code = 124}).ConcernsHotkey(hotkey));
  }
}

TEST(HotkeysTest, DoesNotInterceptAnIncompleteShortcut) {
  auto settings =
      RightArrowSettings(kCGEventFlagMaskControl | kCGEventFlagMaskShift);
  auto parsed = ParseHotkeyConfiguration(settings.get());
  ASSERT_TRUE(parsed.ok()) << parsed.status();
  EXPECT_FALSE((KeyEvent{.key_code = 124, .modifiers = kCGEventFlagMaskControl})
                   .ConcernsHotkey(parsed->move_space_right));
}

TEST(HotkeysTest, IgnoresDeviceFlagsWhenMatchingAnArrowShortcut) {
  auto settings =
      RightArrowSettings(kCGEventFlagMaskControl | kCGEventFlagMaskSecondaryFn |
                         kCGEventFlagMaskNumericPad);
  auto parsed = ParseHotkeyConfiguration(settings.get());
  ASSERT_TRUE(parsed.ok()) << parsed.status();
  EXPECT_EQ(parsed->move_space_right.modifiers, kCGEventFlagMaskControl);
  EXPECT_TRUE((KeyEvent{.key_code = 124,
                        .modifiers = kCGEventFlagMaskControl |
                                     kCGEventFlagMaskSecondaryFn |
                                     kCGEventFlagMaskNumericPad})
                  .ConcernsHotkey(parsed->move_space_right));
}

TEST(HotkeysTest, KeepsDisabledShortcutsDisabled) {
  auto settings = RightArrowSettings(kCGEventFlagMaskAlternate, false);
  auto parsed = ParseHotkeyConfiguration(settings.get());
  ASSERT_TRUE(parsed.ok()) << parsed.status();
  EXPECT_FALSE(
      (KeyEvent{.key_code = 124, .modifiers = kCGEventFlagMaskAlternate})
          .ConcernsHotkey(parsed->move_space_right));
}

TEST(HotkeysTest, UsesNativeDefaultsForUnspecifiedShortcuts) {
  auto settings = MakeDictionary();
  auto parsed = ParseHotkeyConfiguration(settings.get());
  ASSERT_TRUE(parsed.ok()) << parsed.status();
  EXPECT_TRUE(parsed->move_space_left.enabled);
  EXPECT_EQ(parsed->move_space_left.key_code, 123);
  EXPECT_EQ(parsed->move_space_left.modifiers, kCGEventFlagMaskControl);
  EXPECT_EQ(parsed->move_space_right.key_code, 124);
  EXPECT_EQ(parsed->open_mission_control.key_code, 126);
  EXPECT_EQ(parsed->open_app_expose.key_code, 125);
}

}  // namespace
}  // namespace fasterswiper
