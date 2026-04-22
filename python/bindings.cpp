#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "langdetect/detector.hpp"
#include "langdetect/result.hpp"

namespace py = pybind11;

PYBIND11_MODULE(langdetect, m) {
    m.doc() = "Language detection library";

    // Expose ResultKind enum
    py::enum_<ResultKind>(m, "ResultKind")
        .value("Language",  ResultKind::Language)
        .value("Script",    ResultKind::Script)
        .value("Unknown",   ResultKind::Unknown)
        .value("Ambiguous", ResultKind::Ambiguous)
        .export_values();

    // Expose DetectionScore struct
    py::class_<DetectionScore>(m, "DetectionScore")
        .def_readonly("label", &DetectionScore::label)
        .def_readonly("kind",  &DetectionScore::kind)
        .def_readonly("score", &DetectionScore::score)
        .def("__repr__", [](const DetectionScore& s) {
            return "DetectionScore(label='" + s.label + "', score=" + std::to_string(s.score) + ")";
        });

    // Expose LanguageDetector class
    py::class_<LanguageDetector>(m, "LanguageDetector")
        .def(py::init<const std::string&>(), py::arg("profiles_dir") = "profiles")
        .def("detect", &LanguageDetector::detect, py::arg("text"),
             "Detect the language of the given text");
}