# langdetect-core

`langdetect-core` is a small C++17 language detection project. It is not trying
to be a huge ML model or a perfect replacement for bigger libraries. The main
goal is to have a fast native core that can detect a few Latin languages, and
also give quick script-level answers for non-Latin text.

At the moment the project is in MVP state, but the basic pipeline is working:

- UTF-8 input is decoded into Unicode code points.
- Non-Latin scripts are detected mostly by Unicode ranges.
- Latin text is compared against generated n-gram profiles.
- A small CLI returns JSON output.
- There are tests and a benchmark target.

## What it detects now

Language-level detection is currently for these Latin languages:

```text
en, de, fr, es, it
```

Script-level detection exists for:

```text
cyrillic, arabic, greek, devanagari, thai, korean, japanese, cjk_han
```

There are also fallback results:

```text
unknown, ambiguous
```

The scores are useful for ranking, but they should not be read as real
probabilities. They are normalized similarity scores.

## How it works

The detector first decodes the text as UTF-8. After that it counts script
signals. If the input is clearly non-Latin, it can return early with a script
result, which is why Russian, Arabic, Japanese and Chinese benchmarks are much
faster.

For Latin text it does more work:

1. Normalize Latin letters.
2. Remove separators and keep useful word gaps.
3. Build weighted 1-gram, 2-gram and 3-gram profiles.
4. Compare that input profile with the JSON profiles in `profiles/`.
5. Return ranked results, or `unknown` / `ambiguous` if the signal is weak.

Very short Latin text is still not reliable, so the detector intentionally
returns `unknown` for many tiny inputs.

## Repository layout

```text
langdetect-core/
|-- app/
|   `-- langdetect_cli.cpp
|-- benchmarks/
|   `-- bench_detector.cpp
|-- corpora/
|   `-- raw/
|-- include/
|   `-- langdetect/
|-- profiles/
|   |-- de.json
|   |-- en.json
|   |-- es.json
|   |-- fr.json
|   `-- it.json
|-- python/
|   `-- bindings.cpp
|-- src/
|-- tests/
|-- tools/
|   |-- build_profile.py
|   |-- evaluate_profiles.py
|   `-- scrape_corpus.py
|-- CMakeLists.txt
`-- README.md
```

## Requirements

- CMake 3.15 or newer
- C++17 compiler
- `nlohmann_json`
- Python 3, needed for profile/corpus tools and the Python bindings
- `pybind11`, needed for the Python bindings

On my setup this was built with MSYS2/UCRT64 and GCC.

## Build

From the repo root:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

Main build targets:

- `langdetect_lib`
- `langdetect`
- `test_utf8`
- `test_detector`
- `bench_detector`

## CLI usage

Example:

```powershell
.\build\langdetect.exe "Artificial intelligence is changing how people work"
```

Example output:

```json
{
  "best": { "kind": "language", "label": "en", "score": 0.51 },
  "scores": [
    { "kind": "language", "label": "en", "score": 0.51 },
    { "kind": "language", "label": "de", "score": 0.20 }
  ]
}
```

The exact scores can change when profiles change.

## C++ usage

```cpp
#include "langdetect/detector.hpp"
#include <iostream>

int main() {
    LanguageDetector detector("profiles");
    auto results = detector.detect("The quick brown fox jumps over the lazy dog");

    if (!results.empty()) {
        std::cout << results[0].label << " " << results[0].score << "\n";
    }
}
```

The detector returns `std::vector<DetectionScore>`.

## Python bindings

The Python wrapper is implemented with pybind11 in `python/bindings.cpp`. It
builds a `langdetect` Python module around the same native detector used by the
CLI and C++ API.

The module exposes:

- `LanguageDetector`, constructed with a profiles directory.
- `LanguageDetector.detect(text)`, which returns ranked detection results.
- `DetectionScore`, with `label`, `kind`, and `score` attributes.
- `ResultKind`, with `Language`, `Script`, `Unknown`, and `Ambiguous` values.

Example usage after building the extension module:

```python
from langdetect import LanguageDetector

detector = LanguageDetector("profiles")
results = detector.detect("Artificial intelligence is changing how people work")

for result in results:
    print(result.label, result.kind, result.score)
```

## Tests

On Windows:

```powershell
.\build\test_utf8.exe
.\build\test_detector.exe
```

Benchmark:

```powershell
.\build\bench_detector.exe
```

There is also a small Python evaluation script:

```powershell
python tools\evaluate_profiles.py
```

## Profile tools

The profile data is generated from raw corpus text.

```powershell
python tools\scrape_corpus.py
python tools\build_profile.py
```

`scrape_corpus.py` downloads Wikipedia text for the selected languages.
`build_profile.py` turns those raw text files into JSON n-gram profiles.

This is enough for the MVP, but the corpus is still small and should be improved
before treating it as production quality.

## Known limitations

- Latin languages are the slow path because they use n-gram profile scoring.
- The current profiles only cover `en`, `de`, `fr`, `es`, and `it`.
- Non-Latin detection is mostly script-level, not language-level.
- Chinese and Japanese are not fully separated when the input is mostly Han
  characters.
- Scores are not calibrated probabilities.
- Short text is hard, especially for Latin languages.
- The CLI JSON is written manually and is still pretty simple.

## Benchmarks

Hardware:

- CPU: 11th Gen Intel Core i7-11800H @ 2.30GHz
- RAM: 16GB
- OS: Windows 11, MSYS2 UCRT64
- Compiler: GCC 15.2.0
- Standard: C++17

Raw benchmark output:

```text
=== langdetect benchmark ===

Very short EN  (<100 chars)
  size:       11 bytes
  iterations: 10000
  avg time:   1752.54 ns

Very short DE  (<100 chars)
  size:       10 bytes
  iterations: 10000
  avg time:   1775.89 ns

Very short FR  (<100 chars)
  size:       13 bytes
  iterations: 10000
  avg time:   1895.89 ns

Very short ES  (<100 chars)
  size:       10 bytes
  iterations: 10000
  avg time:   1710.25 ns

Very short IT  (<100 chars)
  size:       10 bytes
  iterations: 10000
  avg time:   1707.38 ns

Very short RU  (<100 chars)
  size:       19 bytes
  iterations: 10000
  avg time:   984.03 ns

Very short AR  (<100 chars)
  size:       25 bytes
  iterations: 10000
  avg time:   1077.73 ns

Very short JA  (<100 chars)
  size:       21 bytes
  iterations: 10000
  avg time:   815.59 ns

Very short ZH  (<100 chars)
  size:       12 bytes
  iterations: 10000
  avg time:   690.82 ns

Normal EN      (100-500 chars)
  size:       181 bytes
  iterations: 5000
  avg time:   322802 ns

Normal DE      (100-500 chars)
  size:       190 bytes
  iterations: 5000
  avg time:   331631 ns

Normal FR      (100-500 chars)
  size:       143 bytes
  iterations: 5000
  avg time:   257503 ns

Normal ES      (100-500 chars)
  size:       140 bytes
  iterations: 5000
  avg time:   259844 ns

Normal IT      (100-500 chars)
  size:       136 bytes
  iterations: 5000
  avg time:   249071 ns

Normal RU      (100-500 chars)
  size:       277 bytes
  iterations: 5000
  avg time:   5444.6 ns

Normal AR      (100-500 chars)
  size:       221 bytes
  iterations: 5000
  avg time:   5163.9 ns

Normal JA      (100-500 chars)
  size:       222 bytes
  iterations: 5000
  avg time:   3017.5 ns

Normal ZH      (100-500 chars)
  size:       126 bytes
  iterations: 5000
  avg time:   2340.04 ns

Paragraph EN   (500-5000 chars)
  size:       665 bytes
  iterations: 1000
  avg time:   863416 ns

Paragraph DE   (500-5000 chars)
  size:       445 bytes
  iterations: 1000
  avg time:   623422 ns

Paragraph FR   (500-5000 chars)
  size:       526 bytes
  iterations: 1000
  avg time:   663179 ns

Paragraph ES   (500-5000 chars)
  size:       497 bytes
  iterations: 1000
  avg time:   644613 ns

Paragraph IT   (500-5000 chars)
  size:       498 bytes
  iterations: 1000
  avg time:   634124 ns

Paragraph RU   (500-5000 chars)
  size:       1004 bytes
  iterations: 1000
  avg time:   19963.8 ns

Paragraph AR   (500-5000 chars)
  size:       617 bytes
  iterations: 1000
  avg time:   17652.9 ns

Paragraph JA   (500-5000 chars)
  size:       508 bytes
  iterations: 1000
  avg time:   7900.5 ns

Paragraph ZH   (500-5000 chars)
  size:       453 bytes
  iterations: 1000
  avg time:   6117.2 ns

Large doc EN   (100KB+)
  size:       102410 bytes
  iterations: 100
  avg time:   5.49685e+07 ns

Large doc RU   (100KB+)
  size:       102408 bytes
  iterations: 100
  avg time:   1.48842e+06 ns

Large doc JA   (100KB+)
  size:       102616 bytes
  iterations: 100
  avg time:   1.01538e+06 ns
```

The main thing visible here is that Latin is slower. That makes sense because
Latin text goes through the whole profile building and cosine similarity path.
Script-only results are much cheaper.

## Conclusion and possible improvements

The MVP is working, but the Latin hot path needs more work. Some likely next
steps:

- Top-K pruning: compare only against the most frequent n-grams, not every
  n-gram in the profile.
- Better data structures: replace `unordered_map` with sorted arrays or a more
  cache-friendly structure in the hot path.
- Reuse more buffers while building input profiles.
- Add larger and cleaner corpus data for the Latin profiles.
- Add more language-level support outside Latin later.

## License

MIT License. See [LICENSE](LICENSE).
