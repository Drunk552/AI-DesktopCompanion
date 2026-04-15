#pragma once

#include <memory>
#include <string>

class ConfigManager;
class Camera;
class VisionPipeline;
class GemmaAI;
class MemoryDB;
class UIManager;
class PersonaLoader;

class ModuleRegistry {
public:
    explicit ModuleRegistry(const std::string& configPath = "config.json");
    ~ModuleRegistry();

    ModuleRegistry(const ModuleRegistry&) = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;

    ConfigManager& config();
    Camera& camera();
    VisionPipeline& visionPipeline();
    GemmaAI& ai();
    MemoryDB& memory();
    UIManager& ui();
    PersonaLoader& personaLoader();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
