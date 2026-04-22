import os
import json
from collections import Counter
from typing import Dict, List, Mapping


BASE_DIR: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RAW_DIR: str = os.path.join(BASE_DIR, "corpora", "raw")
OUT_DIR: str = os.path.join(BASE_DIR, "profiles")

os.makedirs(OUT_DIR, exist_ok=True)

NGRAM_SIZE: int = 2
TOP_K: int = 300


# Keep letters and spaces only so the profile is about language, not punctuation
def clean_text(text: str) -> str:
    text = text.lower()
    return "".join(c for c in text if c.isalpha() or c == ' ')


# Build simple character n-grams while skipping windows that cross word gaps
def build_ngrams(text: str, n: int) -> List[str]:
    ngrams: List[str] = []
    for i in range(len(text) - n + 1):
        ngram = text[i:i+n]
        if " " not in ngram:  # skip spaces
            ngrams.append(ngram)
    return ngrams


# Convert counts into relative frequencies for the detector profile
def normalize(counter: Mapping[str, int]) -> Dict[str, float]:
    total = sum(counter.values())
    return {k: v / total for k, v in counter.items()}


# Read one raw language corpus and turn it into a frequency table
def process_language(file_path: str) -> Dict[str, float]:
    with open(file_path, "r", encoding="utf-8") as f:
        text = f.read()

    text = clean_text(text)

    if len(text) < 100:
        raise ValueError(f"Text too small: {file_path}")

    ngrams = build_ngrams(text, NGRAM_SIZE)

    counts = Counter(ngrams).most_common(TOP_K)
    counts = dict(counts)

    freqs = normalize(counts)

    return freqs


def main() -> None:
    for filename in os.listdir(RAW_DIR):
        if not filename.endswith(".txt"):
            continue

        lang = filename.split(".")[0]
        path = os.path.join(RAW_DIR, filename)

        print(f"[+] Processing {lang}")

        try:
            freqs = process_language(path)

            assert len(freqs) > 0, f"Empty profile for {lang}"

            output = {
                "language": lang,
                "version": 1,
                "ngram_size": NGRAM_SIZE,
                "ngrams": freqs
            }

            out_path = os.path.join(OUT_DIR, f"{lang}.json")

            with open(out_path, "w", encoding="utf-8") as f:
                json.dump(output, f, ensure_ascii=False, indent=2)

            print(f"[✓] Saved {out_path} ({len(freqs)} n-grams)")

        except Exception as e:
            print(f"[!] Failed for {lang}: {e}")


if __name__ == "__main__":
    main()
