#include "melegi/AgentFramework.hpp"
#include "melegi/AuthenticityEngine.hpp"
#include "melegi/ShoegazeEngine.hpp"
#include "melegi/GenerationEngine.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace melegi {

// ── AgentBus ──────────────────────────────────────────────────────────────
void AgentBus::post(AgentMessage msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    msg.id = nextId_++;
    inbox_.push_back(std::move(msg));
}

bool AgentBus::poll(const std::string& recipient, AgentMessage& out) {
    std::lock_guard<std::mutex> lock(mtx_);
    for (size_t i = 0; i < inbox_.size(); ++i) {
        if (inbox_[i].to.empty() || inbox_[i].to == recipient) {
            out = inbox_[i];
            inbox_.erase(inbox_.begin() + i);
            return true;
        }
    }
    return false;
}

void AgentBus::clear() {
    std::lock_guard<std::mutex> lock(mtx_);
    inbox_.clear();
}

size_t AgentBus::pending(const std::string& recipient) const {
    std::lock_guard<std::mutex> lock(mtx_);
    size_t cnt = 0;
    for (auto& m : inbox_)
        if (m.to.empty() || m.to == recipient) ++cnt;
    return cnt;
}

// ── ComposerAgent ──────────────────────────────────────────────────────────
void ComposerAgent::update(AgentContext& ctx) {
    AgentMessage msg;
    while (poll(msg)) {
        if (msg.type == MessageType::RequestRevision) {
            ++revisionCount_;
            // Modulate DNA slightly for next revision
            ctx.dna.Darkness = std::min(1.f, ctx.dna.Darkness + 0.05f * revisionCount_);
            ctx.dna.EmotionalIntensity = std::min(1.f, ctx.dna.EmotionalIntensity + 0.03f);
        }
    }

    if (!ctx.score) return;

    // Generate / evolve the score
    if (revisionCount_ == 0 && ctx.score->chordNotes.empty()) {
        GeoAstralEngine gae;
        auto astralData = GeoAstralEngine::now(40.71f, -74.0f);
        astralData = gae.compute(astralData);
        auto influence = gae.influence(astralData);

        float durSec = ctx.score->arrangement.totalBeats > 0.f
            ? ctx.score->arrangement.totalBeats / (ctx.score->arrangement.bpm / 60.f)
            : 8.f;

        GenerationEngine gen(revisionCount_);
        *ctx.score = gen.generate(ctx.dna, influence, durSec);

        post({MessageType::ApplyStyleDNA, name_, "ProducerAgent", "Score ready", 1.f});
    }
}

// ── ProducerAgent ──────────────────────────────────────────────────────────
void ProducerAgent::update(AgentContext& ctx) {
    AgentMessage msg;
    bool triggered = false;
    while (poll(msg)) {
        if (msg.type == MessageType::ApplyStyleDNA) triggered = true;
    }
    if (!triggered || !ctx.audioOut || !ctx.audioOut->valid()) return;

    // Apply shoegaze processing driven by DNA
    ShoegazeEngine sge;
    auto params = ShoegazeParams::fromDNA(ctx.dna);

    // Scale params by creativityBudget (don't over-process)
    params.hazeAmount      *= ctx.creativityBudget * 0.5f;
    params.saturationAmount *= ctx.creativityBudget * 0.4f;

    *ctx.audioOut = sge.process(*ctx.audioOut, params);

    post({MessageType::BalanceMix, name_, "MixAgent", "Textures applied", 1.f});
}

// ── MixAgent ──────────────────────────────────────────────────────────────
void MixAgent::update(AgentContext& ctx) {
    AgentMessage msg;
    bool triggered = false;
    while (poll(msg)) {
        if (msg.type == MessageType::BalanceMix) triggered = true;
    }
    if (!triggered || !ctx.audioOut || !ctx.audioOut->valid()) return;

    // Apply CASH/ERUPTION bus balance
    // CASH bus: 70% — already the main content
    // ERUPTION bus: 30% — would be mixed in from a separate buffer
    // Here we apply the overall output gain law: never exceed –1 dBFS
    for (uint32_t c = 0; c < ctx.audioOut->numChannels; ++c)
        for (uint64_t f = 0; f < ctx.audioOut->numFrames; ++f)
            ctx.audioOut->at(c, f) *= cashGain_;

    // Peak limiting (safety)
    float pk = ctx.audioOut->peakAmplitude();
    if (pk > 0.89f) ctx.audioOut->normalize(0.88f);

    post({MessageType::ReviewAuthenticity, name_, "AuthenticityAgent", "Mix balanced", 1.f});
}

// ── AuthenticityAgent ──────────────────────────────────────────────────────
void AuthenticityAgent::update(AgentContext& ctx) {
    AgentMessage msg;
    bool triggered = false;
    while (poll(msg)) {
        if (msg.type == MessageType::ReviewAuthenticity) triggered = true;
    }
    if (!triggered || !ctx.audioOut || !ctx.audioOut->valid()) return;

    AuthenticityEngine ae;
    auto report = ae.score(*ctx.audioOut);

    std::ostringstream os;
    os << report.diagnosis;

    if (!report.passesAuthenticity) {
        ++rejections_;
        if (rejections_ <= 2) {
            // Apply humanization and retry
            HumanizationMap hm;
            TimingEngine te;
            BeatGrid grid = te.buildGrid(122.f, ctx.audioOut->durationSeconds());
            hm = te.generateHumanizationMap(grid, ctx.dna, nullptr, rejections_ * 17);
            *ctx.audioOut = ae.process(*ctx.audioOut, hm, ctx.dna);

            post({MessageType::RequestRevision, name_, "ComposerAgent",
                  "Authenticity failed: " + os.str(), 2.f});
            return;
        }
        // Accept after 2 retries (better than silence)
    }

    post({MessageType::ScoreEmotion, name_, "CriticAgent", os.str(), 1.f});
}

// ── CriticAgent ───────────────────────────────────────────────────────────
void CriticAgent::update(AgentContext& ctx) {
    AgentMessage msg;
    bool triggered = false;
    while (poll(msg)) {
        if (msg.type == MessageType::ScoreEmotion) triggered = true;
    }
    if (!triggered || !ctx.audioOut || !ctx.audioOut->valid()) return;

    AuthenticityEngine ae;
    auto report = ae.score(*ctx.audioOut);

    // Emotional score: combine authenticity with DNA-driven expressiveness
    float emotionBonus = ctx.dna.EmotionalIntensity * 30.f;
    float darknessBonus = ctx.dna.Darkness * 10.f;
    lastScore_ = clamp(report.overallScore + emotionBonus + darknessBonus, 0.f, 100.f);

    if (lastScore_ >= 60.f) {
        post({MessageType::AcceptOutput, name_, "", "Score: " +
              std::to_string(static_cast<int>(lastScore_)), 0.f});
    } else {
        post({MessageType::RequestRevision, name_, "ComposerAgent",
              "Low score: " + std::to_string(static_cast<int>(lastScore_)), 1.5f});
    }
}

// ── AgentFramework ────────────────────────────────────────────────────────
AgentFramework::AgentFramework() {
    bus_ = std::make_shared<AgentBus>();
    composerAgent_ = std::make_unique<ComposerAgent>("ComposerAgent", bus_);
    producerAgent_ = std::make_unique<ProducerAgent>("ProducerAgent", bus_);
    mixAgent_      = std::make_unique<MixAgent>("MixAgent",      bus_);
    authAgent_     = std::make_unique<AuthenticityAgent>("AuthenticityAgent", bus_);
    criticAgent_   = std::make_unique<CriticAgent>("CriticAgent", bus_);
}

bool AgentFramework::run(AgentContext& ctx, int maxRounds) {
    bus_->clear();
    // Kick off the pipeline
    bus_->post({MessageType::CreateIdea, "Framework", "ComposerAgent", "Begin", 2.f});

    for (int round = 0; round < maxRounds; ++round) {
        composerAgent_->update(ctx);
        if (ctx.score && !ctx.score->chordNotes.empty() && ctx.audioOut
            && !ctx.audioOut->valid()) {
            // Render score to audio for subsequent agents
            GenerationEngine gen;
            *ctx.audioOut = gen.renderToAudio(*ctx.score, DEFAULT_SAMPLE_RATE);
            bus_->post({MessageType::ApplyStyleDNA, "Framework", "ProducerAgent",
                        "Audio rendered", 1.f});
        }
        producerAgent_->update(ctx);
        mixAgent_->update(ctx);
        authAgent_->update(ctx);
        criticAgent_->update(ctx);

        // Check for acceptance
        AgentMessage acc;
        bool accepted = false;
        while (bus_->poll("", acc)) {
            if (acc.type == MessageType::AcceptOutput) {
                accepted = true;
            }
        }
        if (accepted) return true;
    }
    return false; // Consensus not reached in maxRounds
}

} // namespace melegi
