#include "langdetect/detector.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using Clock = std::chrono::high_resolution_clock;

double measure_ns(LanguageDetector& detector, const std::string& text, int iterations) {
    auto start = Clock::now();
    for (int i = 0; i < iterations; ++i) {
        detector.detect(text);
    }
    auto end = Clock::now();
    double total = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    return total / iterations;
}

int main() {
    LanguageDetector detector("profiles");

    // Very short samples mostly exercise fast script and unknown paths
    const std::string vs_en = "Hello world";
    const std::string vs_de = "Hallo Welt";
    const std::string vs_fr = "Bonjour monde";
    const std::string vs_es = "Hola mundo";
    const std::string vs_it = "Ciao mondo";
    const std::string vs_ru = "Привет мир";
    const std::string vs_ar = "مرحبا بالعالم";
    const std::string vs_ja = "こんにちは世界";
    const std::string vs_zh = "你好世界";

    // Normal samples are closer to the minimum useful profile-scoring size
    const std::string n_en =
        "Artificial intelligence is the simulation of human intelligence processes "
        "by machines, especially computer systems. These processes include learning, "
        "reasoning, and self-correction.";

    const std::string n_de =
        "Künstliche Intelligenz ist die Simulation menschlicher Intelligensprozesse "
        "durch Maschinen, insbesondere Computersysteme. Diese Prozesse umfassen Lernen, "
        "Schlußfolgern und Selbstkorrektur.";

    const std::string n_fr =
        "L'intelligence artificielle est la simulation des processus d'intelligence "
        "humaine par des machines en particulier des systèmes informatiques.";

    const std::string n_es =
        "La inteligencia artificial es la simulación de procesos de inteligencia "
        "humana por parte de máquinas especialmente sistemas informáticos.";

    const std::string n_it =
        "L'intelligenza artificiale è la simulazione dei processi di intelligenza "
        "umana da parte di macchine in particolare sistemi informatici.";

    const std::string n_ru =
        "Искусственный интеллект это симуляция процессов человеческого интеллекта "
        "машинами особенно компьютерными системами эти процессы включают обучение.";

    const std::string n_ar =
        "الذكاء الاصطناعي هو محاكاة عمليات الذكاء البشري بواسطة الآلات "
        "وخاصة أنظمة الكمبيوتر وتشمل هذه العمليات التعلم والتفكير.";

    const std::string n_ja =
        "人工知能とは、機械、特にコンピュータシステムによる人間の知能プロセスのシミュレーションです。"
        "これらのプロセスには、学習、推論、自己修正が含まれます。";

    const std::string n_zh =
        "人工智能是由机器特别是计算机系统对人类智能过程的模拟这些过程包括学习推理和自我纠正。";

    // Paragraph samples make cosine scoring cost easier to see
    const std::string p_en =
        "Artificial intelligence is the simulation of human intelligence processes "
        "by machines, especially computer systems. These processes include learning, "
        "reasoning, and self-correction. Particular applications of AI include expert "
        "systems, natural language processing, speech recognition and machine vision. "
        "As machines become increasingly capable, tasks considered to require "
        "intelligence are often removed from the definition of AI. Modern machine "
        "capabilities generally classified as AI include successfully understanding "
        "human speech, competing at the highest level in strategic game systems, "
        "autonomously operating cars and intelligent routing in content delivery.";

    const std::string p_de =
        "Künstliche Intelligenz ist die Simulation menschlicher Intelligensprozesse "
        "durch Maschinen insbesondere Computersysteme. Diese Prozesse umfassen Lernen "
        "Schlußfolgern und Selbstkorrektur. Besondere Anwendungen der KI umfassen "
        "Expertensysteme natürliche Sprachverarbeitung Spracherkennung und maschinelles "
        "Sehen. Da Maschinen immer leistungsfähiger werden werden Aufgaben die "
        "Intelligenz erfordern oft aus der Definition der KI herausgenommen.";

    const std::string p_fr =
        "L'intelligence artificielle est la simulation des processus d'intelligence "
        "humaine par des machines en particulier des systèmes informatiques. Ces "
        "processus comprennent l'apprentissage le raisonnement et l'autocorrection. "
        "Les applications particulières de l'IA comprennent les systèmes experts "
        "le traitement du langage naturel la reconnaissance vocale et la vision "
        "artificielle. À mesure que les machines deviennent plus capables les tâches "
        "nécessitant de l'intelligence sont souvent retirées de la définition de l'IA.";

    const std::string p_es =
        "La inteligencia artificial es la simulación de procesos de inteligencia "
        "humana por parte de máquinas especialmente sistemas informáticos. Estos "
        "procesos incluyen el aprendizaje el razonamiento y la autocorrección. "
        "Las aplicaciones particulares de la IA incluyen sistemas expertos "
        "procesamiento del lenguaje natural reconocimiento de voz y visión artificial. "
        "A medida que las máquinas se vuelven más capaces las tareas que requieren "
        "inteligencia a menudo se eliminan de la definición de IA.";

    const std::string p_it =
        "L'intelligenza artificiale è la simulazione dei processi di intelligenza "
        "umana da parte di macchine in particolare sistemi informatici. Questi "
        "processi includono l'apprendimento il ragionamento e l'autocorrezione. "
        "Le applicazioni particolari dell'IA includono sistemi esperti elaborazione "
        "del linguaggio naturale riconoscimento vocale e visione artificiale. "
        "Man mano che le macchine diventano sempre più capaci i compiti che richiedono "
        "intelligenza vengono spesso rimossi dalla definizione di IA.";

    const std::string p_ru =
        "Искусственный интеллект это моделирование процессов человеческого интеллекта "
        "машинами особенно компьютерными системами эти процессы включают обучение "
        "рассуждение и самокоррекцию конкретные применения искусственного интеллекта "
        "включают экспертные системы обработку естественного языка распознавание речи "
        "и машинное зрение по мере того как машины становятся все более способными "
        "задачи требующие интеллекта часто исключаются из определения искусственного "
        "интеллекта современные возможности машин включают понимание человеческой речи.";

    const std::string p_ar =
        "الذكاء الاصطناعي هو محاكاة عمليات الذكاء البشري بواسطة الآلات وخاصة أنظمة "
        "الكمبيوتر وتشمل هذه العمليات التعلم والتفكير والتصحيح الذاتي وتشمل التطبيقات "
        "الخاصة للذكاء الاصطناعي الأنظمة الخبيرة ومعالجة اللغة الطبيعية والتعرف على "
        "الكلام ورؤية الآلة ومع تزايد قدرات الآلات كثيرا ما تزال المهام التي تتطلب "
        "الذكاء من تعريف الذكاء الاصطناعي.";

    const std::string p_ja =
        "人工知能とは機械特にコンピュータシステムによる人間の知能プロセスのシミュレーションです。"
        "これらのプロセスには学習推論自己修正が含まれます。AIの特定のアプリケーションには"
        "エキスパートシステム自然言語処理音声認識マシンビジョンが含まれます。"
        "機械がますます高性能になるにつれて知能を必要とするタスクはAIの定義から"
        "削除されることが多くなっています。";

    const std::string p_zh =
        "人工智能是由机器特别是计算机系统对人类智能过程的模拟这些过程包括学习推理和自我纠正"
        "人工智能的特定应用包括专家系统自然语言处理语音识别和机器视觉随着机器变得越来越强大"
        "需要智能的任务往往从人工智能的定义中删除现代机器能力通常被归类为人工智能包括成功"
        "理解人类语言在战略游戏系统中处于最高水平以及自动驾驶汽车。";

    std::string large_en, large_ru, large_ja;
    large_en.reserve(1024 * 100);
    large_ru.reserve(1024 * 100);
    large_ja.reserve(1024 * 100);
    // Build large inputs by repeating paragraph text instead of keeping files around
    while (large_en.size() < 1024 * 100) large_en += p_en;
    while (large_ru.size() < 1024 * 100) large_ru += p_ru;
    while (large_ja.size() < 1024 * 100) large_ja += p_ja;

    struct TestCase {
        std::string name;
        const std::string& text;
        int iterations;
    };

    std::vector<TestCase> cases = {
        {"Very short EN  (<100 chars)",     vs_en,    10000},
        {"Very short DE  (<100 chars)",     vs_de,    10000},
        {"Very short FR  (<100 chars)",     vs_fr,    10000},
        {"Very short ES  (<100 chars)",     vs_es,    10000},
        {"Very short IT  (<100 chars)",     vs_it,    10000},
        {"Very short RU  (<100 chars)",     vs_ru,    10000},
        {"Very short AR  (<100 chars)",     vs_ar,    10000},
        {"Very short JA  (<100 chars)",     vs_ja,    10000},
        {"Very short ZH  (<100 chars)",     vs_zh,    10000},
        {"Normal EN      (100-500 chars)",  n_en,     5000},
        {"Normal DE      (100-500 chars)",  n_de,     5000},
        {"Normal FR      (100-500 chars)",  n_fr,     5000},
        {"Normal ES      (100-500 chars)",  n_es,     5000},
        {"Normal IT      (100-500 chars)",  n_it,     5000},
        {"Normal RU      (100-500 chars)",  n_ru,     5000},
        {"Normal AR      (100-500 chars)",  n_ar,     5000},
        {"Normal JA      (100-500 chars)",  n_ja,     5000},
        {"Normal ZH      (100-500 chars)",  n_zh,     5000},
        {"Paragraph EN   (500-5000 chars)", p_en,     1000},
        {"Paragraph DE   (500-5000 chars)", p_de,     1000},
        {"Paragraph FR   (500-5000 chars)", p_fr,     1000},
        {"Paragraph ES   (500-5000 chars)", p_es,     1000},
        {"Paragraph IT   (500-5000 chars)", p_it,     1000},
        {"Paragraph RU   (500-5000 chars)", p_ru,     1000},
        {"Paragraph AR   (500-5000 chars)", p_ar,     1000},
        {"Paragraph JA   (500-5000 chars)", p_ja,     1000},
        {"Paragraph ZH   (500-5000 chars)", p_zh,     1000},
        {"Large doc EN   (100KB+)",         large_en, 100},
        {"Large doc RU   (100KB+)",         large_ru, 100},
        {"Large doc JA   (100KB+)",         large_ja, 100},
    };

    std::cout << "\n=== langdetect benchmark ===\n\n";

    for (auto& tc : cases) {
        double avg_ns = measure_ns(detector, tc.text, tc.iterations);
        std::cout << tc.name << "\n";
        std::cout << "  size:       " << tc.text.size() << " bytes\n";
        std::cout << "  iterations: " << tc.iterations << "\n";
        std::cout << "  avg time:   " << avg_ns << " ns\n\n";
    }

    return 0;
}
