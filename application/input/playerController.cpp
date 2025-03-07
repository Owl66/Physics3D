#include "core.h"

#include "playerController.h"


#include <Physics3D/world.h>

#include "application.h"
#include "view/screen.h"
#include "standardInputHandler.h"
#include "engine/options/keyboardOptions.h"
#include "../extendedPart.h"

#define RUN_SPEED 5
#define JUMP_SPEED 6
#define AIR_RUN_SPEED_FACTOR 2


namespace P3D::Application {
void PlayerController::apply(WorldPrototype* world) {
	// Player movement
	if(!screen.camera.flying) {
		using namespace Engine;
		ExtendedPart* player = screen.camera.attachment;
		Physical* playerPhys = player->getPhysical();

		Vec3f playerX = screen.camera.cframe.rotation * Vec3(1, 0, 0);
		Vec3f playerZ = screen.camera.cframe.rotation * Vec3(0, 0, 1);

		Vec3 up(0, 1, 0);
		Vec3 forward = normalize(playerZ % up % up);
		Vec3 right = -normalize(playerX % up % up);

		Vec3 total(0, 0, 0);
		if(handler->getKey(KeyboardOptions::Move::forward))
			total += forward;
		if(handler->getKey(KeyboardOptions::Move::backward))
			total -= forward;
		if(handler->getKey(KeyboardOptions::Move::right))
			total += right;
		if(handler->getKey(KeyboardOptions::Move::left))
			total -= right;

		Vec3 runVector = (lengthSquared(total) >= 0.00005) ? normalize(total) * RUN_SPEED : Vec3(0, 0, 0);
		Vec3 desiredSpeed = runVector;
		Vec3 actualSpeed = playerPhys->getMotion().getVelocity();
		Vec3 speedToGain = desiredSpeed - actualSpeed;
		speedToGain.y = 0;

		playerPhys->mainPhysical->applyForceAtCenterOfMass(speedToGain * player->getMass() * AIR_RUN_SPEED_FACTOR);

		if(handler->getKey(KeyboardOptions::Move::jump))
			runVector += Vec3(0, JUMP_SPEED, 0);

		player->properties.conveyorEffect = runVector;
	}
}
};


