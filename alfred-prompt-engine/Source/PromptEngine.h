#pragma once
#include <string>
#include <vector>

namespace alfred {

struct PromptResult {
    std::string prompt;
    std::string title;
    std::string style;
};

class PromptEngine {
public:
    PromptResult buildPrompt(const std::string& source);

private:
    struct Directive {
        std::string title;
        std::string style;
        std::string tempo;
        std::string key;
        std::vector<std::string> sections;
        std::vector<std::string> lyricThemes;
        std::vector<std::string> sonicCalls;
        std::vector<std::string> productionNotes;
        int loopBars = 0;
    };

    Directive        parse(const std::string& source);
    std::string      synthesize(const Directive& d);

    static std::string              annotation(const std::string& src, const std::string& tag);
    static std::vector<std::string> functionDefs(const std::string& src);
    static std::vector<std::string> stringLiterals(const std::string& src);
    static std::vector<std::string> inlineComments(const std::string& src);
    static std::vector<std::string> functionCalls(const std::string& src);
    static int                      loopIterations(const std::string& src);
    static std::string              humanize(const std::string& name);
    static std::string              join(const std::vector<std::string>& v, const std::string& sep);
};

} // namespace alfred
