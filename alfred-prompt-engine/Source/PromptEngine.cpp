#include "PromptEngine.h"

#include <algorithm>
#include <regex>
#include <set>
#include <sstream>

namespace alfred {

// ── Public ────────────────────────────────────────────────────────────────────

PromptResult PromptEngine::buildPrompt(const std::string& source) {
    const auto d = parse(source);
    PromptResult r;
    r.prompt = synthesize(d);
    r.title  = d.title;
    r.style  = d.style;
    return r;
}

// ── Private: parse ─────────────────────────────────────────────────────────────

PromptEngine::Directive PromptEngine::parse(const std::string& source) {
    Directive d;
    d.title            = annotation(source, "title");
    d.style            = annotation(source, "style");
    d.tempo            = annotation(source, "tempo");
    d.key              = annotation(source, "key");
    d.sections         = functionDefs(source);
    d.lyricThemes      = stringLiterals(source);
    d.sonicCalls       = functionCalls(source);
    d.productionNotes  = inlineComments(source);
    d.loopBars         = loopIterations(source);
    return d;
}

// ── Private: synthesize ───────────────────────────────────────────────────────

std::string PromptEngine::synthesize(const Directive& d) {
    std::ostringstream out;

    // Genre / style + key + tempo
    if (!d.style.empty()) {
        out << d.style;
        if (!d.key.empty())   out << ", in " << d.key;
        if (!d.tempo.empty()) out << ", at " << d.tempo;
        out << ". ";
    } else {
        if (!d.key.empty())   out << "Key of " << d.key << ". ";
        if (!d.tempo.empty()) out << d.tempo << ". ";
    }

    // Song structure from function names
    if (!d.sections.empty()) {
        out << "Structure: ";
        for (std::size_t i = 0; i < d.sections.size(); ++i) {
            if (i > 0) out << ", then ";
            out << humanize(d.sections[i]);
        }
        out << ". ";
    }

    // Loop / repetition
    if (d.loopBars > 0)
        out << "Built on a " << d.loopBars << "-bar repeating loop. ";

    // Sonic palette from function calls
    if (!d.sonicCalls.empty())
        out << "Sonic palette includes " << join(d.sonicCalls, ", ") << ". ";

    // Lyrical themes from string literals
    if (!d.lyricThemes.empty()) {
        out << "Lyrical themes: ";
        for (std::size_t i = 0; i < d.lyricThemes.size(); ++i) {
            if (i > 0) out << "; ";
            out << '"' << d.lyricThemes[i] << '"';
        }
        out << ". ";
    }

    // Production notes from inline comments
    if (!d.productionNotes.empty())
        out << "Direction: " << join(d.productionNotes, ". ") << ".";

    std::string result = out.str();
    if (result.empty()) result = "A unique, expressive musical composition.";
    return result;
}

// ── Static parsers ─────────────────────────────────────────────────────────────

std::string PromptEngine::annotation(const std::string& src, const std::string& tag) {
    std::regex re("//\\s*@" + tag + "\\s+([^\n]+)");
    std::smatch m;
    if (std::regex_search(src, m, re)) {
        auto v = m[1].str();
        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.back()))) v.pop_back();
        return v;
    }
    return {};
}

std::vector<std::string> PromptEngine::functionDefs(const std::string& src) {
    std::regex re(R"((?:void|auto|int|float|double|bool)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\()");
    std::vector<std::string> out;
    for (auto it = std::sregex_iterator(src.begin(), src.end(), re);
         it != std::sregex_iterator(); ++it)
        out.push_back((*it)[1].str());
    return out;
}

std::vector<std::string> PromptEngine::stringLiterals(const std::string& src) {
    // Match quoted strings of at least 4 chars
    std::regex re(R"("([^"\\]{4,})")");
    std::vector<std::string> out;
    for (auto it = std::sregex_iterator(src.begin(), src.end(), re);
         it != std::sregex_iterator(); ++it)
        out.push_back((*it)[1].str());
    return out;
}

std::vector<std::string> PromptEngine::inlineComments(const std::string& src) {
    // Non-annotation comments only
    std::regex re(R"(//(?!\s*@)\s*(.+))");
    std::vector<std::string> out;
    for (auto it = std::sregex_iterator(src.begin(), src.end(), re);
         it != std::sregex_iterator(); ++it) {
        auto text = (*it)[1].str();
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
            text.pop_back();
        if (!text.empty()) out.push_back(text);
    }
    return out;
}

std::vector<std::string> PromptEngine::functionCalls(const std::string& src) {
    static const std::set<std::string> kKeywords = {
        "void","auto","int","float","double","bool","char","if","for","while",
        "switch","return","new","delete","sizeof","static_cast","dynamic_cast",
        "reinterpret_cast","std","juce","auto"
    };

    const auto defs = functionDefs(src);
    const std::set<std::string> defSet(defs.begin(), defs.end());

    std::regex re(R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\()");
    std::set<std::string> seen;
    std::vector<std::string> out;

    for (auto it = std::sregex_iterator(src.begin(), src.end(), re);
         it != std::sregex_iterator(); ++it) {
        const std::string name = (*it)[1].str();
        if (defSet.count(name) || kKeywords.count(name) || seen.count(name)) continue;
        seen.insert(name);
        out.push_back(humanize(name));
    }
    return out;
}

int PromptEngine::loopIterations(const std::string& src) {
    // Grab the upper bound from "for (... ; x < N ; ...)"
    std::regex re(R"(for\s*\([^;]*;\s*[a-zA-Z_]\w*\s*<\s*(\d+))");
    std::smatch m;
    if (std::regex_search(src, m, re)) {
        try { return std::stoi(m[1].str()); } catch (...) {}
    }
    if (src.find("for") != std::string::npos ||
        src.find("while") != std::string::npos)
        return 8;
    return 0;
}

std::string PromptEngine::humanize(const std::string& name) {
    std::string out;
    bool lastLower = false;
    for (char c : name) {
        if (c == '_') {
            if (!out.empty()) out += ' ';
            lastLower = false;
        } else if (std::isupper(static_cast<unsigned char>(c)) && lastLower) {
            out += ' ';
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            lastLower = false;
        } else {
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            lastLower = std::islower(static_cast<unsigned char>(c)) != 0;
        }
    }
    return out;
}

std::string PromptEngine::join(const std::vector<std::string>& v, const std::string& sep) {
    std::string out;
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i > 0) out += sep;
        out += v[i];
    }
    return out;
}

} // namespace alfred
