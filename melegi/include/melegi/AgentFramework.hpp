#pragma once
#include "Common.hpp"
#include "StyleDNA.hpp"
#include "AnalysisEngine.hpp"
#include "GenerationEngine.hpp"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <memory>
#include <atomic>
#include <variant>

namespace melegi {

// ── Message types ──────────────────────────────────────────────────────────
enum class MessageType {
    CreateIdea,       // ComposerAgent → others
    ShapeTexture,     // ProducerAgent
    BalanceMix,       // MixAgent
    ReviewAuthenticity,
    ScoreEmotion,
    ApplyStyleDNA,
    RequestRevision,
    AcceptOutput,
    RejectOutput,
};

struct AgentMessage {
    MessageType type;
    std::string from;
    std::string to;       // empty = broadcast
    std::string payload;  // JSON-compatible text
    float       priority {0.f};
    uint64_t    id       {0};
};

// ── AgentContext ───────────────────────────────────────────────────────────
struct AgentContext {
    StyleDNA        dna;
    GeneratedScore* score    {nullptr};
    AudioBuffer*    audioOut {nullptr};
    float           creativityBudget{1.f};  // how much change agent can make
};

// ── AgentBus ───────────────────────────────────────────────────────────────
// Thread-safe message bus shared by all agents.
class AgentBus {
public:
    void post(AgentMessage msg);
    bool poll(const std::string& recipient, AgentMessage& out);
    void clear();
    size_t pending(const std::string& recipient) const;

private:
    mutable std::mutex           mtx_;
    std::vector<AgentMessage>    inbox_;
    std::atomic<uint64_t>        nextId_{1};
};

// ── Agent base ─────────────────────────────────────────────────────────────
class Agent {
public:
    explicit Agent(std::string name, std::shared_ptr<AgentBus> bus)
        : name_(std::move(name)), bus_(std::move(bus)) {}
    virtual ~Agent() = default;

    const std::string& name() const { return name_; }
    virtual void       update(AgentContext& ctx) = 0;

protected:
    void post(AgentMessage msg) { bus_->post(std::move(msg)); }
    bool poll(AgentMessage& out) { return bus_->poll(name_, out); }

    std::string                name_;
    std::shared_ptr<AgentBus>  bus_;
};

// ── ComposerAgent ──────────────────────────────────────────────────────────
// Creates and evolves musical ideas. Originates the GeneratedScore.
class ComposerAgent : public Agent {
public:
    using Agent::Agent;
    void update(AgentContext& ctx) override;
private:
    int revisionCount_{0};
};

// ── ProducerAgent ──────────────────────────────────────────────────────────
// Shapes texture: applies StyleDNA, calls ShoegazeEngine, controls energy curve.
class ProducerAgent : public Agent {
public:
    using Agent::Agent;
    void update(AgentContext& ctx) override;
};

// ── MixAgent ──────────────────────────────────────────────────────────────
// Balances levels between CASH/ERUPTION buses. Applies master chain.
class MixAgent : public Agent {
public:
    using Agent::Agent;
    void update(AgentContext& ctx) override;
private:
    float cashGain_     {0.70f};
    float eruptionGain_ {0.30f};
};

// ── AuthenticityAgent ─────────────────────────────────────────────────────
// Rejects sterile output. Scores and enforces humanity threshold.
class AuthenticityAgent : public Agent {
public:
    using Agent::Agent;
    void update(AgentContext& ctx) override;
    static constexpr float MIN_AUTHENTICITY = 50.f;
private:
    int rejections_{0};
};

// ── CriticAgent ───────────────────────────────────────────────────────────
// Scores emotional impact [0,100]. Approves or requests revision.
class CriticAgent : public Agent {
public:
    using Agent::Agent;
    void update(AgentContext& ctx) override;
    float lastScore() const { return lastScore_; }
private:
    float lastScore_{0.f};
};

// ── AgentFramework ────────────────────────────────────────────────────────
// Orchestrates the multi-agent pipeline. Synchronous (no threads) for
// reproducibility; call run() for a complete collaborative pass.
class AgentFramework {
public:
    AgentFramework();

    // Run the full agent pipeline on a context. Returns when all agents
    // reach consensus (AcceptOutput) or maxRounds is exhausted.
    // Returns true if output was accepted, false if consensus wasn't reached.
    bool run(AgentContext& ctx, int maxRounds = 8);

    // Access individual agents for direct queries
    CriticAgent*        critic()        { return criticAgent_.get(); }
    AuthenticityAgent*  authenticity()  { return authAgent_.get(); }

private:
    std::shared_ptr<AgentBus>           bus_;
    std::unique_ptr<ComposerAgent>      composerAgent_;
    std::unique_ptr<ProducerAgent>      producerAgent_;
    std::unique_ptr<MixAgent>           mixAgent_;
    std::unique_ptr<AuthenticityAgent>  authAgent_;
    std::unique_ptr<CriticAgent>        criticAgent_;
};

} // namespace melegi
