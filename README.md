# langdetect-core

`langdetect-core` is a lightweight C++17 language detection core. It decodes
UTF-8 input, detects Unicode script families, and scores a small set of
Latin-script languages with character n-gram profiles.

The project is intentionally small and dependency-light. It is designed as a
reusable C++ library first, with room for CLI tooling, profile generation,
Python bindings, and evaluation tools later.

## Current Features

- UTF-8 decoding into Unicode code points.
- Script detection for Latin, Cyrillic, Arabic, Greek, Devanagari, Thai,
  Korean, Japanese, and CJK Han text.
- Latin-language scoring for:
  - English: `en`
  - German: `de`
  - French: `fr`
  - Spanish: `es`
  - Italian: `it`
- Ranked detection results with explicit result kinds:
  - `Language`
  - `Script`
  - `Unknown`
  - `Ambiguous`
- Conservative handling for short or weak input.
- CMake build with library and test targets.

## Project Status

This repository currently contains the core C++ implementation and tests. The
`langdetect` executable target exists, but the command-line interface is still a
placeholder. The `profiles`, `corpora`, `tools`, `python`, `examples`, and
`benchmarks` directories are reserved for future work.

Scores are useful for ranking candidates, but they are not calibrated
probabilities.

## Repository Layout

```text
langdetect-core/
|-- app/
|   `-- langdetect_cli.cpp
|-- include/
|   `-- langdetect/
|       |-- detector.hpp
|       |-- result.hpp
|       |-- script.hpp
|       `-- utf8.hpp
|-- src/
|   |-- detector.cpp
|   `-- utf8.cpp
|-- tests/
|   |-- test_detector.cpp
|   `-- test_utf8.cpp
|-- CMakeLists.txt
|-- LICENSE
`-- README.md
```

## Requirements

- CMake 3.15 or newer
- A C++17-compatible compiler
- Ninja, Make, Visual Studio, or another CMake-supported build system

## Build

From the repository root:

```powershell
cmake -S . -B build
cmake --build build
```

To explicitly use Ninja:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

The build creates these main targets:

- `langdetect_lib`: static library with the detection implementation
- `langdetect`: CLI executable target, currently scaffolded
- `test_utf8`: UTF-8 decoder test executable
- `test_detector`: detector test executable

## Run Tests

On Windows:

```powershell
.\build\test_utf8.exe
.\build\test_detector.exe
```

On Unix-like systems:

```bash
./build/test_utf8
./build/test_detector
```

Expected output includes:

```text
All tests passed
All detector tests passed
```

## C++ API Example

```cpp
#include "langdetect/detector.hpp"
#include <iostream>

int main() {
    LanguageDetector detector;
    auto results = detector.detect("The quick brown fox jumps over the lazy dog");

    if (!results.empty()) {
        std::cout << results[0].label << " " << results[0].score << "\n";
    }

    return 0;
}
```

The detector returns a `std::vector<DetectionScore>`:

```cpp
enum class ResultKind {
    Language,
    Script,
    Unknown,
    Ambiguous
};

struct DetectionScore {
    std::string label;
    ResultKind kind;
    double score;
};
```

## Detection Behavior

The detector first decodes UTF-8 input into Unicode code points. It then counts
script signals and uses that information to decide whether to return a script
result or run Latin-language scoring.

For Latin text, the detector:

1. Normalizes Latin characters.
2. Removes low-signal separators.
3. Builds weighted 1-gram, 2-gram, and 3-gram profiles.
4. Compares the input profile against built-in seed profiles.
5. Returns ranked results, or `unknown` / `ambiguous` when the signal is weak.

Short Latin input is intentionally treated as unreliable. For example, very
short strings such as `hi` return `unknown`.

## Supported Labels

Language labels:

```text
en, de, fr, es, it
```

Script labels:

```text
cyrillic, arabic, greek, devanagari, thai, korean, japanese, cjk_han
```

Fallback labels:

```text
unknown, ambiguous
```

## Known Limitations

- The CLI target is not implemented yet.
- Built-in Latin profiles are small seed profiles, not production-grade corpus
  profiles.
- Scores are relative ranking scores, not true probabilities.
- Short text can be unreliable.
- Similar Latin languages may be confused.
- Han-only CJK text is reported as script-level `cjk_han`, not as a specific
  language.
- Mixed-language segmentation is not implemented.

## Roadmap

- Implement the CLI input and JSON output contract.
- Add generated language profiles from licensed corpora.
- Add profile-building and evaluation tools.
- Expand detector tests for more scripts and edge cases.
- Add benchmarks.
- Add Python bindings after the C++ API stabilizes.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
