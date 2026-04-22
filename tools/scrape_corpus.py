import os
import wikipediaapi
import time
from typing import Dict, List

# Languages we build raw corpora for
LANGUAGES: List[str] = ['en', 'de', 'fr', 'es', 'it']

# Common topics that usually exist in all target Wikipedias
ARTICLES: Dict[str, List[str]] = {
    'en': [
        'Artificial_intelligence', 'Computer_science', 'Mathematics', 'Physics', 'History',
        'Philosophy', 'Geography', 'Economics', 'Literature', 'Biology',
        'Chemistry', 'Psychology', 'Astronomy', 'Architecture', 'Music',
        'Politics', 'Sociology', 'Linguistics', 'Medicine', 'Engineering'
    ],
    'de': [
        'Künstliche_Intelligenz', 'Informatik', 'Mathematik', 'Physik', 'Geschichte',
        'Philosophie', 'Geographie', 'Wirtschaft', 'Literatur', 'Biologie',
        'Chemie', 'Psychologie', 'Astronomie', 'Architektur', 'Musik',
        'Politik', 'Soziologie', 'Linguistik', 'Medizin', 'Ingenieurwesen'
    ],
    'fr': [
        'Intelligence_artificielle', 'Informatique', 'Mathématiques', 'Physique', 'Histoire',
        'Philosophie', 'Géographie', 'Économie', 'Littérature', 'Biologie',
        'Chimie', 'Psychologie', 'Astronomie', 'Architecture', 'Musique',
        'Politique', 'Sociologie', 'Linguistique', 'Médecine', 'Ingénierie'
    ],
    'es': [
        'Inteligencia_artificial', 'Informática', 'Matemáticas', 'Física', 'Historia',
        'Filosofía', 'Geografía', 'Economía', 'Literatura', 'Biología',
        'Química', 'Psicología', 'Astronomía', 'Arquitectura', 'Música',
        'Política', 'Sociología', 'Lingüística', 'Medicina', 'Ingeniería'
    ],
    'it': [
        'Intelligenza_artificiale', 'Informatica', 'Matematica', 'Fisica', 'Storia',
        'Filosofia', 'Geografia', 'Economia', 'Letteratura', 'Biologia',
        'Chimica', 'Psicologia', 'Astronomia', 'Architettura', 'Musica',
        'Politica', 'Sociologia', 'Linguistica', 'Medicina', 'Ingegneria'
    ]
}
BASE_DIR: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


# Raw corpora are stored where the profile builder already expects them
OUTPUT_DIR: str = os.path.join(BASE_DIR, "corpora", "raw")
os.makedirs(OUTPUT_DIR, exist_ok=True)


def fetch_articles(lang: str) -> str:
    print(f"[+] Fetching for language: {lang}")

    wiki = wikipediaapi.Wikipedia(
        language=lang,
        user_agent='langdetect-core/1.0'
    )

    combined_text: List[str] = []

    for title in ARTICLES[lang]:
        page = wiki.page(title)

        if not page.exists():
            print(f"[-] Missing: {title} ({lang})")
            continue

        print(f"[✓] {title}")

        text = page.text.strip()

        if text:
            combined_text.append(text)

        # Pause a little between requests to avoid annoying the API
        time.sleep(0.5)

    return "\n\n".join(combined_text)


def save_corpus(lang: str, text: str) -> None:
    path = os.path.join(OUTPUT_DIR, f"{lang}.txt")

    with open(path, "w", encoding="utf-8") as f:
        f.write(text)

    print(f"[+] Saved: {path}")


def main() -> None:
    for lang in LANGUAGES:
        text = fetch_articles(lang)

        if text:
            save_corpus(lang, text)
        else:
            print(f"[!] No data for {lang}")


if __name__ == "__main__":
    main()
