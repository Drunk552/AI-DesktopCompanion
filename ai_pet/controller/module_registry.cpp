#include "controller/module_registry.h"
#include "ai/gemma.h"
#include "ai/persona_loader.h"
#include "config/config.h"
#include "logger/logger.h"
#include "memory/sqlite.h"
#include "ui/ui.h"
#include "vision/camera.h"
#include "vision/emotion.h"
#include "vision/face.h"
#include "vision/vision_pipeline.h"

struct ModuleRegistry::Impl {
    ConfigManager& config;
    Camera camera;
    FaceDetector faceDetector;
    EmotionRecognizer emotionRecognizer;
    VisionPipeline visionPipeline;
    GemmaAI ai;
    MemoryDB memory;
    UIManager ui;
    PersonaLoader personaLoader;

    Impl(ConfigManager& cfg, const AppConfig& c)
        : config(cfg)
        , camera(c.camera_sdp_path)
        , faceDetector(c.face_model_path)
        , emotionRecognizer()
        , visionPipeline(faceDetector, emotionRecognizer)
        , ai(c.ollama_url, c.model_name)
        , memory(c.database_path) {}
};

ModuleRegistry::ModuleRegistry(const std::string& configPath) {
    ConfigManager& config = ConfigManager::instance();
    config.load(configPath);
    impl_ = std::make_unique<Impl>(config, config.get());
    LOGI("ModuleRegistry", "模块已装配");
}

ModuleRegistry::~ModuleRegistry() = default;

ConfigManager& ModuleRegistry::config() { return impl_->config; }
Camera& ModuleRegistry::camera() { return impl_->camera; }
VisionPipeline& ModuleRegistry::visionPipeline() { return impl_->visionPipeline; }
GemmaAI& ModuleRegistry::ai() { return impl_->ai; }
MemoryDB& ModuleRegistry::memory() { return impl_->memory; }
UIManager& ModuleRegistry::ui() { return impl_->ui; }
PersonaLoader& ModuleRegistry::personaLoader() { return impl_->personaLoader; }
