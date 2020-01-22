/*
 Imagine
 Copyright 2016 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

#ifndef CAM_RAY_CREATOR_FISHEYE_H
#define CAM_RAY_CREATOR_FISHEYE_H

#include "camera_ray_creator.h"

namespace Imagine
{

class CamRayCreatorFishEye : public CameraRayCreator
{
public:
	virtual void init(const Params& params)
	{
		m_fovRadians = radians(m_FOV);
	}

	virtual Ray createBasicCameraRay(float x, float y) const
	{
		const float offsetX = ((m_invWidth * x) * 2.0f) - 1.0f;
		// need to invert Y
		const float offsetY = ((1.0f - (m_invHeight * y)) * 2.0f) - 1.0f;

		float rad = (offsetX * offsetX) + (offsetY * offsetY);
		rad = sqrtf(rad);

		if (rad > 1.0f)
		{
			// can't sample this direction
			Ray ray1(Point(), Normal(0.0f, 0.0f, 0.0f), RAY_UNDEFINED);
			return ray1;
		}

		const float phi = rad * m_fovRadians * 0.5f;
		const float theta = atan2f(offsetY, offsetX);
		const float sinPhi = sinf(phi);

		Normal newDirection(sinPhi * cosf(theta), sinPhi * sinf(theta), -cosf(phi));

		Ray ray1(Point(), newDirection, RAY_CAMERA);
		ray1 = m_transform.transform(ray1);
		ray1.direction.normalise();

		ray1.tMin = m_nearClippingPlane;

		if (m_createDifferentials)
		{
			Normal diffXDir;
			Normal diffYDir;
			calculateDifferentials(x, y, offsetX, offsetY, diffXDir, diffYDir);

			diffXDir = m_transform.transform(diffXDir);
			diffXDir.normalise();
			diffYDir = m_transform.transform(diffYDir);
			diffYDir.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDir, diffYDir);
		}

		return ray1;
	}

	virtual Ray createCameraRay(float x, float y, SampleBundle& sampleBundle, unsigned int sampleIndex) const
	{
		const float offsetX = ((m_invWidth * x) * 2.0f) - 1.0f;
		// need to invert Y
		const float offsetY = ((1.0f - (m_invHeight * y)) * 2.0f) - 1.0f;

		float rad = (offsetX * offsetX) + (offsetY * offsetY);
		rad = sqrtf(rad);

		if (rad > 1.0f)
		{
			// can't sample this direction
			Ray ray1(Point(), Normal(0.0f, 0.0f, 0.0f), RAY_UNDEFINED);
			return ray1;
		}

		const float phi = rad * m_fovRadians * 0.5f;
		const float theta = atan2f(offsetY, offsetX);
		const float sinPhi = sinf(phi);

		Normal newDirection(sinPhi * cosf(theta), sinPhi * sinf(theta), -cosf(phi));

		Ray ray1(Point(), newDirection, RAY_CAMERA);
		ray1 = m_transform.transform(ray1);
		ray1.direction.normalise();

		ray1.tMin = m_nearClippingPlane;

		if (m_createDifferentials)
		{
			Normal diffXDir;
			Normal diffYDir;
			calculateDifferentials(x, y, offsetX, offsetY, diffXDir, diffYDir);

			diffXDir = m_transform.transform(diffXDir);
			diffXDir.normalise();
			diffYDir = m_transform.transform(diffYDir);
			diffYDir.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDir, diffYDir);
		}

		return ray1;
	}

protected:
	void calculateDifferentials(float x, float y, float offsetX, float offsetY, Normal& diffXDir, Normal& diffYDir) const
	{
		const float diffXOffset = ((m_invWidth * (x + 1.0f)) * 2.0f) - 1.0f;
		const float diffYOffset = ((1.0f - (m_invHeight * (y + 1.0f))) * 2.0f) - 1.0f;

		float diffXRad = (diffXOffset * diffXOffset) + (offsetY * offsetY);
		diffXRad = sqrtf(diffXRad);
		float diffYRad = (offsetX * offsetX) + (diffYOffset * diffYOffset);
		diffYRad = sqrtf(diffYRad);

		const float diffXPhi = diffXRad * m_fovRadians * 0.5f;
		const float diffYPhi = diffYRad * m_fovRadians * 0.5f;
		const float diffXTheta = atan2f(offsetY, diffXOffset);
		const float diffYTheta = atan2f(diffYOffset, offsetX);
		const float diffXSinPhi = sinf(diffXPhi);
		const float diffYSinPhi = sinf(diffYPhi);

		diffXDir = Normal(diffXSinPhi * cosf(diffXTheta), diffXSinPhi * sinf(diffXTheta), -cosf(diffXPhi));
		diffYDir = Normal(diffYSinPhi * cosf(diffYTheta), diffYSinPhi * sinf(diffYTheta), -cosf(diffYPhi));
	}

protected:
	float	m_fovRadians; // cached converted value
};

// static camera, different time per ray
class CamRayCreatorFishEyeMB : public CamRayCreatorFishEye
{
public:
	virtual Ray createCameraRay(float x, float y, SampleBundle& sampleBundle, unsigned int sampleIndex) const
	{
		const float offsetX = ((m_invWidth * x) * 2.0f) - 1.0f;
		// need to invert Y
		const float offsetY = ((1.0f - (m_invHeight * y)) * 2.0f) - 1.0f;

		float rad = (offsetX * offsetX) + (offsetY * offsetY);
		rad = sqrtf(rad);

		if (rad > 1.0f)
		{
			// can't sample this direction
			Ray ray1(Point(), Normal(0.0f, 0.0f, 0.0f), RAY_UNDEFINED);
			return ray1;
		}

		const float phi = rad * m_fovRadians * 0.5f;
		const float theta = atan2f(offsetY, offsetX);
		const float sinPhi = sinf(phi);

		Normal newDirection(sinPhi * cosf(theta), sinPhi * sinf(theta), -cosf(phi));

		float time = sampleBundle.getTimeSample(sampleIndex);
		float timeFull = time;
		if (m_relativeTimes)
		{
			timeFull = linearInterpolate(m_shutterOpen, m_shutterClose, time);
		}

		Ray ray1(Point(), newDirection, time, timeFull, RAY_CAMERA);
		ray1 = m_transform.transform(ray1);
		ray1.direction.normalise();

		ray1.tMin = m_nearClippingPlane;

		if (m_createDifferentials)
		{
			Normal diffXDir;
			Normal diffYDir;
			calculateDifferentials(x, y, offsetX, offsetY, diffXDir, diffYDir);

			diffXDir = m_transform.transform(diffXDir);
			diffXDir.normalise();
			diffYDir = m_transform.transform(diffYDir);
			diffYDir.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDir, diffYDir);
		}

		return ray1;
	}
};

// Camera MB as well
class CamRayCreatorFishEyeMBFull : public CamRayCreatorFishEye
{
public:
	virtual Ray createCameraRay(float x, float y, SampleBundle& sampleBundle, unsigned int sampleIndex) const
	{
		const float offsetX = ((m_invWidth * x) * 2.0f) - 1.0f;
		// need to invert Y
		const float offsetY = ((1.0f - (m_invHeight * y)) * 2.0f) - 1.0f;

		float rad = (offsetX * offsetX) + (offsetY * offsetY);
		rad = sqrtf(rad);

		if (rad > 1.0f)
		{
			// can't sample this direction
			Ray ray1(Point(), Normal(0.0f, 0.0f, 0.0f), RAY_UNDEFINED);
			return ray1;
		}

		const float phi = rad * m_fovRadians * 0.5f;
		const float theta = atan2f(offsetY, offsetX);
		const float sinPhi = sinf(phi);

		Normal newDirection(sinPhi * cosf(theta), sinPhi * sinf(theta), -cosf(phi));

		float time = sampleBundle.getTimeSample(sampleIndex);
		float timeFull = time;
		if (m_relativeTimes)
		{
			timeFull = linearInterpolate(m_shutterOpen, m_shutterClose, time);
		}

		Ray ray1(Point(), newDirection, time, timeFull, RAY_CAMERA);

		SnapShotTransform ssT;
		m_transform.getTransformAtTime(time, ssT);

		ray1 = ssT.m_transform.transform(ray1);
		ray1.direction.normalise();

		ray1.tMin = m_nearClippingPlane;

		if (m_createDifferentials)
		{
			Normal diffXDir;
			Normal diffYDir;
			calculateDifferentials(x, y, offsetX, offsetY, diffXDir, diffYDir);

			diffXDir = ssT.m_transform.transform(diffXDir);
			diffXDir.normalise();
			diffYDir = ssT.m_transform.transform(diffYDir);
			diffYDir.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDir, diffYDir);
		}

		return ray1;
	}
};

} // namespace Imagine

#endif // CAM_RAY_CREATOR_FISHEYE_H

