import subprocess
import json
import os
from typing import List, Tuple

BASE_DIR: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CLI: str = os.path.join(BASE_DIR, "build", "langdetect.exe")

TEST_CASES: List[Tuple[str, str]] = [
    ("en", "The quick brown fox jumps over the lazy dog and runs through the forest"),
    ("en", "Artificial intelligence is transforming the way we live and work today"),
    ("de", "Die Künstliche Intelligenz verändert die Art und Weise wie wir leben und arbeiten"),
    ("de", "Die Mathematik ist eine Wissenschaft die sich mit Zahlen und Strukturen befasst"),
    ("fr", "L'intelligence artificielle est une discipline scientifique dont l'objectif est de créer des machines capables de simuler l'intelligence humaine et d'effectuer des tâches complexes"),
    ("fr", "La philosophie est l'étude des questions fondamentales sur l'existence et la connaissance"),
    ("es", "La inteligencia artificial está transformando la forma en que vivimos y trabajamos"),
    ("es", "La matemática es la ciencia que estudia las estructuras cantidades y cambios"),
    ("it", "L'intelligenza artificiale sta trasformando il modo in cui viviamo e lavoriamo"),
    ("it", "La matematica è la scienza che studia le strutture le quantità e i cambiamenti"),
]


# The CLI prints JSON, and this helper keeps the evaluation loop small
def detect(text: str) -> str:
    result = subprocess.run(
        [CLI, text],
        capture_output=True,
        text=True,
        encoding="utf-8"
    )
    try:
        data = json.loads(result.stdout)
        return data["best"]["label"]
    except Exception:
        return "error"


def main() -> None:
    correct: int = 0
    total: int = len(TEST_CASES)

    for expected, text in TEST_CASES:
        predicted = detect(text)
        status = "✓" if predicted == expected else "✗"
        print(f"[{status}] expected={expected} predicted={predicted}")
        print(f"    {text[:60]}...")
        if predicted == expected:
            correct += 1

    print(f"\nAccuracy: {correct}/{total} ({100 * correct / total:.1f}%)")

if __name__ == "__main__":
    main()
