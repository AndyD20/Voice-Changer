# Voice Changer
Real-time voice modification app

## License

This project is licensed under the GNU Affero General Public License v3.0 (AGPLv3). See [LICENSE](LICENSE) for the full text.

It is built on [JUCE](https://juce.com/), whose free license is AGPLv3, so the application as a whole must be distributed under AGPLv3.

## Building

Requires CMake 3.25+ and a C++20 compiler (Visual Studio 2022 on Windows). Dependencies (JUCE, Signalsmith Stretch, GoogleTest) are downloaded automatically on first configure.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
```

Pass `-DVOICE_CHANGER_BUILD_TESTS=OFF` to skip building the tests.
