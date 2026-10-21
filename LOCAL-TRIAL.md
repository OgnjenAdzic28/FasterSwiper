# FasterSwiper local trial

This fork keeps a visible Spaces slide and adds a 160 ms quadratic ease-out preset
in Settings > Animation. The preset changes only horizontal duration and easing.
It does not overwrite saved preferences on startup, enable interception, or change
vertical gestures or keyboard shortcuts.

It also preserves all modifiers in macOS's configured shortcuts and avoids
sending a cancellation gesture when a switch has not moved.

## Build

Use the prerequisites in [README.md](README.md), then run:

```sh
bazel test --build_tests_only //src/... --test_output=errors
bazel build -c opt //src/app:FasterSwiper_local_app
```

The local target has its own bundle identifier, preferences and permission grant.
The upstream `FasterSwiper_app` target remains available. Unzip
`bazel-bin/src/app/FasterSwiper_local_app.zip` and copy `FasterSwiper Local.app` to
Applications. A local build is not a notarized upstream release.

If Bazel reports an incomplete Android SDK while building this macOS app, unset
`ANDROID_HOME` and `ANDROID_SDK_ROOT` for the build command only:

```sh
env -u ANDROID_HOME -u ANDROID_SDK_ROOT bazel build -c opt //src/app:FasterSwiper_local_app
```

## Try it

1. Quit the upstream FasterSwiper before opening FasterSwiper Local. Run only one
   gesture interceptor at a time.
2. Grant the permissions requested by the local app in System Settings. It needs
   its own grant even if the upstream app is already authorized.
3. In Animation, enable horizontal interception, use the 160 ms preset, and leave
   vertical interception off for the initial trial. In Keyboard, enable Mission
   Control shortcut interception and leave numbered jump shortcuts off.
4. Try adjacent Control-arrow moves in both directions, including into and out of
   fullscreen apps. Check the first and last Space, reversing direction, and each
   display. Try a normal trackpad swipe and a swipe cancelled before release.
5. Use a disposable text field to check when typing starts reaching the destination.
   Report missing or misdirected input separately from visual smoothness.

To return to the baseline, quit FasterSwiper Local and open the upstream
FasterSwiper. Both apps retain their own settings. Nothing here changes SIP,
Reduce Motion, macOS shortcuts, or login items.

The tests record synthetic events in memory; they do not send input to the desktop.
Passing them and building the app does not prove live focus handoff or fix the
upstream macOS 27 multi-Space bounce issue. This is a shorter-transition trial,
not a promise that windows accept input while Spaces are still moving.
