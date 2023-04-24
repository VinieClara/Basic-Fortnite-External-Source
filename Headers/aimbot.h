
#include "include.h"
#include "Data/basic_data.h"




Vector3 AimbotCorrection(float bulletVelocity, float bulletGravity, float targetDistance, Vector3 targetPosition, Vector3 targetVelocity) {
	Vector3 recalculated = targetPosition;
	float gravity = fabs(bulletGravity);
	float time = targetDistance / fabs(bulletVelocity);
	/* Bullet drop correction */
	float bulletDrop = (gravity / 250) * time * time;
	recalculated.z += bulletDrop * 120;
	/* Player movement correction */
	recalculated.x += time * (targetVelocity.x);
	recalculated.y += time * (targetVelocity.y);
	recalculated.z += time * (targetVelocity.z);
	return recalculated;
}


double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}


void aimbot(float x, float y)
{
	float ScreenCenterX = (Hopesar::Overlay::Width / 2);
	float ScreenCenterY = (Hopesar::Overlay::Height / 2);
	int AimSpeed = Hopesar::Settings::Aimbot::smooth;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	//WriteAngles(TargetX / 3.5f, TargetY / 3.5f);
	Hijacked_Move(TargetX, TargetY);

	return;
}



//std::mutex aimmutex;
void AimAt(DWORD_PTR mesh, uintptr_t actor)
{
	//aimmutex.lock();
	uint64_t currentactormesh = mesh;

	auto rootHead = CallBoneFunction(mesh, 68);


	if (Hopesar::Settings::Aimbot::prediction) {
		float distance = W2SCam.Location.Distance(rootHead) / 250;
		uint64_t CurrentActorRootComponent = driver.read<uint64_t>(actor + ROOTCMP_OFFSET);
		Vector3 vellocity = driver.read<Vector3>(CurrentActorRootComponent + VELLOCITY_OFFSET);
		Vector3 Predicted = AimbotCorrection(30000, -504, distance, rootHead, vellocity);
		Vector3 rootHeadOut = ProjectWorldToScreen(Predicted);
		if (rootHeadOut.x != 0 || rootHeadOut.y != 0 ) {
			aimbot(rootHeadOut.x, rootHeadOut.y);
		}
	}
	else {
		Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);
		if (rootHeadOut.y != 0 || rootHeadOut.y != 0)
		{
			aimbot(rootHeadOut.x, rootHeadOut.y);
		}
	}

	
	//aimmutex.unlock();
}
