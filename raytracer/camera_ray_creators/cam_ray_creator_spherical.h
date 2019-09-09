/*
 Imagine
 Copyright 2013-2016 Peter Pearson.

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

#ifndef CAM_RAY_CREATOR_SPHERICAL_H
#define CAM_RAY_CREATOR_SPHERICAL_H

#include "camera_ray_creator.h"

namespace Imagine
{

class CamRayCreatorSpherical : public CameraRayCreator
{
public:
	virtual Ray createBasicCameraRay(float x, float y) const
	{
		const float theta = kPI * y * m_invHeight;
		const float phi = kPITimes2 * (m_width - x) * m_invWidth;

		const float sinTheta = sinf(theta);
		const float sinPhi = sinf(phi);
		const float cosPhi = cosf(phi);
		const float cosTheta = cosf(theta);

		Normal newDirection(sinTheta * sinPhi, cosTheta, sinTheta * cosPhi);

		Ray ray1(Point(), newDirection, RAY_CAMERA);
		ray1 = m_transform.transform(ray1);
		ray1.direction.normalise();

		ray1.tMin = m_nearClippingPlane;

		if (m_createDifferentials)
		{
			float diffTheta = kPI * (y + 1.0f) * m_invHeight;
			float diffPhi = kPITimes2 * (m_width - (x + 1)) * m_invWidth;

			float diffSinTheta = sinf(diffTheta);

			Normal diffXDirection(sinTheta * sinf(diffPhi), cosTheta, sinTheta * cosf(diffPhi));
			Normal diffYDirection(diffSinTheta * sinPhi, cosf(diffTheta), diffSinTheta * cosPhi);

			diffXDirection = m_transform.transform(diffXDirection);
			diffXDirection.normalise();

			diffYDirection = m_transform.transform(diffYDirection);
			diffYDirection.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDirection, diffYDirection);
		}

		return ray1;
	}

	virtual Ray createCameraRay(float x, float y, SampleBundle& sampleBundle, unsigned int sampleIndex) const
	{
		const float theta = kPI * y * m_invHeight;
		const float phi = kPITimes2 * (m_width - x) * m_invWidth;

		const float sinTheta = sinf(theta);
		const float sinPhi = sinf(phi);
		const float cosPhi = cosf(phi);
		const float cosTheta = cosf(theta);

		Normal newDirection(sinTheta * sinPhi, cosTheta, sinTheta * cosPhi);

		Ray ray1(Point(), newDirection, RAY_CAMERA);
		ray1 = m_transform.transform(ray1);
		ray1.direction.normalise();

		ray1.tMin = m_nearClippingPlane;

		if (m_createDifferentials)
		{
			float diffTheta = kPI * (y + 1.0f) * m_invHeight;
			float diffPhi = kPITimes2 * (m_width - (x + 1)) * m_invWidth;

			float diffSinTheta = sinf(diffTheta);

			Normal diffXDirection(sinTheta * sinf(diffPhi), cosTheta, sinTheta * cosf(diffPhi));
			Normal diffYDirection(diffSinTheta * sinPhi, cosf(diffTheta), diffSinTheta * cosPhi);

			diffXDirection = m_transform.transform(diffXDirection);
			diffXDirection.normalise();

			diffYDirection = m_transform.transform(diffYDirection);
			diffYDirection.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDirection, diffYDirection);
		}

		return ray1;
	}
};

// Camera static, but varying time per-ray
class CamRayCreatorSphericalMB : public CamRayCreatorSpherical
{
public:
	virtual Ray createCameraRay(float x, float y, SampleBundle& sampleBundle, unsigned int sampleIndex) const
	{
		const float theta = kPI * y * m_invHeight;
		const float phi = kPITimes2 * (m_width - x) * m_invWidth;

		const float sinTheta = sinf(theta);
		const float sinPhi = sinf(phi);
		const float cosPhi = cosf(phi);
		const float cosTheta = cosf(theta);

		Normal newDirection(sinTheta * sinPhi, cosTheta, sinTheta * cosPhi);

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
			float diffTheta = kPI * (y + 1.0f) * m_invHeight;
			float diffPhi = kPITimes2 * (m_width - (x + 1)) * m_invWidth;

			float diffSinTheta = sinf(diffTheta);

			Normal diffXDirection(sinTheta * sinf(diffPhi), cosTheta, sinTheta * cosf(diffPhi));
			Normal diffYDirection(diffSinTheta * sinPhi, cosf(diffTheta), diffSinTheta * cosPhi);

			diffXDirection = m_transform.transform(diffXDirection);
			diffXDirection.normalise();

			diffYDirection = m_transform.transform(diffYDirection);
			diffYDirection.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDirection, diffYDirection);
		}

		return ray1;
	}
};

// Camera MB as well
class CamRayCreatorSphericalMBFull : public CamRayCreatorSpherical
{
public:
	virtual Ray createCameraRay(float x, float y, SampleBundle& sampleBundle, unsigned int sampleIndex) const
	{
		const float theta = kPI * y * m_invHeight;
		const float phi = kPITimes2 * (m_width - x) * m_invWidth;

		const float sinTheta = sinf(theta);
		const float sinPhi = sinf(phi);
		const float cosPhi = cosf(phi);
		const float cosTheta = cosf(theta);

		Normal newDirection(sinTheta * sinPhi, cosTheta, sinTheta * cosPhi);

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
			float diffTheta = kPI * (y + 1.0f) * m_invHeight;
			float diffPhi = kPITimes2 * (m_width - (x + 1)) * m_invWidth;

			float diffSinTheta = sinf(diffTheta);

			Normal diffXDirection(sinTheta * sinf(diffPhi), cosTheta, sinTheta * cosf(diffPhi));
			Normal diffYDirection(diffSinTheta * sinPhi, cosf(diffTheta), diffSinTheta * cosPhi);

			diffXDirection = ssT.m_transform.transform(diffXDirection);
			diffXDirection.normalise();

			diffYDirection = ssT.m_transform.transform(diffYDirection);
			diffYDirection.normalise();

			ray1.setRayDifferentials(ray1.startPosition, ray1.startPosition, diffXDirection, diffYDirection);
		}

		return ray1;
	}
};

} // namespace Imagine

#endif // CAM_RAY_CREATOR_SPHERICAL_H
