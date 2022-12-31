# CppSimConnect - a Modern C++ Library for interacting with SimConnect

Design goals:

1. Use C++20. Visual Studio 2019 and 2022 have _great_ support for it, so there is no reason to stay in the past.
2. Hide "`SimConnect.h`" as completely as possible.
3. No "`#define`"s, "`#ifdef`"s, "`#if`"s, and such; if it must be compiletime, use `const`, `constexpr`, and `consteval`.
4. No `WIN32` types; if you need to fix implementation size, use the typedefs from `<cstdint>`.
5. Use reactive programming; not just [ranges](https://en.cppreference.com/w/cpp/ranges), but especially [Observables and Observers](https://reactivex.io/intro.html), because they work great with the "ask now, process when available" approach, without having to re-invent the one big message-dispatcher loop for every program.
6. Use classes to encapsulate/abstract concepts, so we can hide all those ugly numerical IDs that won't tell you what they stand for.

And most of all: Always try to resist the urge to do the compiler's work and create ugly and difficult to understand code to gain a few nano-seconds. If that means you don't want to use this library because it "wastes performance," try to measure the impact in a complete setup first. With a modern CPU and graphics card it will generally be the guys at Asobo who need to worry about such things, only rarely the add-on builders. And even then, I'm not advocating to ignore performance; just that there is often more to gain from good algorithmic design than from hyper-optimizing low-level code.

**PS**
    You probably want to be careful when you're going all-out with templates and write you own template library. _That_ is probably a use-case where it pays to sacrafice some readability to performance. See this great presentation ["Your compiler understands it, but does anyone else?"](https://www.youtube.com/watch?v=9ch7tZN4jeI).

## Structure of this repository

* The [CppSimConnect](./CppSimConnect/) directory contains the library itself, except for the actual interface layer to SimConnect. That is kept separate to allow for any differences between the MSFS, Prepar3D, and FSX SimConnect libraries.
* The [CppSimConnectMSFS](./CppSimConnectMSFS/) implements simulator state and library calls for MSFS.
* The [CppSimConnectTester](./CppSimConnectTester/) directory contains a simple test app.

Later, `CppSimConnectP3D` and `CppSimConnectFSX` will be added.