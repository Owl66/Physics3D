#include "core.h"
#include "skyboxLayer.h"

#include <GLFW/glfw3.h>
#include <sstream>
#include "shader/shaders.h"
#include "view/screen.h"
#include "../graphics/renderer.h"
#include "../graphics/texture.h"
#include "../graphics/debug/visualDebug.h"
#include "../graphics/resource/textureResource.h"
#include "../graphics/gui/guiUtils.h"
#include "../util/resource/resourceManager.h"
#include "../graphics/mesh/primitive.h"
#include "graphics/meshRegistry.h"

namespace P3D::Application {

SkyboxCycle lightColorCycle;
SkyboxCycle skyColorCycle;
SkyboxCycle horizonColorCycle;
SkyboxCycle mistColorCycle;

Color lightColor;
Color skyColor;
Color mistColor;
Color horizonColor;
Vec2f mist;
float starBrightness;

Graphics::CubeMap* SkyboxLayer::skyboxTexture = nullptr;

float getScroll(const Screen* screen) {
	return static_cast<float>(std::atan2(screen->camera.viewMatrix(1, 0), screen->camera.viewMatrix(0, 0)) / screen->camera.fov / 2.0);
}
bool useNewSky;
bool pauze;
float time;

void updateMist(float time) {
	float mistFactor;
	if (time > 0.5F) 
		mistFactor = 1.0f - GUI::smoothstep(0.7083333f, 0.875f, time);
	else 
		mistFactor = GUI::smoothstep(0.20833333f, 0.375f, time);
	
	mist = Vec2f(15.0f + mistFactor * 20.0f, 75.0f + mistFactor * 50.0f);
}

void updateStars(float time) {
	if (time > 0.5F)
		starBrightness = GUI::smoothstep(0.9166667f, 1.0f, time);
	else 
		starBrightness = 1.0f - GUI::smoothstep(0.125f, 0.20833333f, time);

}

void SkyboxLayer::onInit(Engine::Registry64& registry) {
	skyboxTexture = new CubeMap("../res/skybox/right.jpg", "../res/skybox/left.jpg", "../res/skybox/top.jpg", "../res/skybox/bottom.jpg", "../res/skybox/front.jpg", "../res/skybox/back.jpg");

	ResourceManager::add<TextureResource>("night", "../res/textures/night.png");
	ResourceManager::add<TextureResource>("uv", "../res/textures/uv.png");

	lightColorCycle = SkyboxCycle(Color(0.42f, 0.45f, 0.90f), Color(1.0f, 0.95f, 0.95f), Color(1.0f, 0.45f, 0.56f), Color(1.0f, 0.87f, 0.6f), 3.0f, 8.0f, 18.0f);
	skyColorCycle = SkyboxCycle(Color(0.31f, 0.44f, 0.64f), Color(0.96f, 0.93f, 0.9f), Color(0.996f, 0.77f, 0.57f), Color(1.0f, 0.94f, 0.67f), 3.0f, 8.0f, 18.0f);
	horizonColorCycle = SkyboxCycle(Color(0.2f, 0.2f, 0.42f), Color(0.6f, 0.9f, 1.0f), Color(0.93f, 0.49f, 0.57f), Color(1.0f, 0.64f, 0.47f), 3.0f, 8.0f, 18.0f);
	mistColorCycle = SkyboxCycle(Color(0.29f, 0.41f, 0.61f), Color(0.96f, 0.9f, 0.77f), Color(1.0f, 0.68f, 0.85f), Color(1.0f, 0.87f, 0.82f), 3.0f, 8.0f, 18.0f);
}

void SkyboxLayer::onUpdate(Engine::Registry64& registry) {
	if (!pauze)
		time = fmod((float) (glfwGetTime() / 30.0), 1.0f);

	lightColor = lightColorCycle.sample(time);
	skyColor = skyColorCycle.sample(time);
	mistColor = mistColorCycle.sample(time);
	horizonColor = horizonColorCycle.sample(time);

	updateMist(time);
	updateStars(time);
}

void SkyboxLayer::onEvent(Engine::Registry64& registry, Engine::Event& event) {

}

void SkyboxLayer::onRender(Engine::Registry64& registry) {
	using namespace Graphics;
	using namespace Graphics::Renderer;
	graphicsMeasure.mark(GraphicsProcess::SKYBOX);

	Screen* screen = static_cast<Screen*>(this->ptr);
	
	beginScene();

	if (useNewSky) {
		disableCulling();
		disableDepthMask();
		enableBlending();
		ResourceManager::get<TextureResource>("night")->bind();

		Shaders::skyShader->setUniform("nightTexture", 0);
		Shaders::skyShader->updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);
		Shaders::skyShader->setUniform("starBrightness", starBrightness);
		Shaders::skyShader->setUniform("skyColor", Vec3f(skyColor));
		Shaders::skyShader->setUniform("horizonColor", Vec3f(horizonColor));
		
		float scroll = getScroll(screen);
		Shaders::skyShader->setUniform("scroll", scroll);
		Shaders::skyShader->setUniform("time", time);
		Shaders::skyShader->setUniform("skyboxSize", 550.0f);
		Shaders::skyShader->setUniform("segmentCount", 25.0f);

		Graphics::MeshRegistry::get(Graphics::MeshRegistry::sphere)->render();
	} else {
		disableCulling();
		disableDepthMask();
		enableBlending();

		Shaders::skyboxShader->updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);
		skyboxTexture->bind();

		Graphics::MeshRegistry::get(Graphics::MeshRegistry::sphere)->render();
	}

	endScene();
}

void SkyboxLayer::onClose(Engine::Registry64& registry) {
	skyboxTexture->close();
}

};