#include "app/module_registry.h"
#include "intelligence/llm/gemma.h"
#include "intelligence/memory/memory_db.h"
#include "intelligence/persona/persona_loader.h"
#include "shared/config/config.h"
#include "shared/logger/logger.h"
#include "ui/ui.h"
#include "perception/camera.h"
#include "perception/emotion.h"
#include "perception/face.h"
#include "perception/vision_pipeline.h"

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

BrainServiceRefs ModuleRegistry::brainServices() {
    return {impl_->ai, impl_->memory, impl_->personaLoader};
}

RuntimeServiceRefs ModuleRegistry::runtimeServices() {
    return {impl_->config, impl_->camera, impl_->visionPipeline, impl_->ui};
}
