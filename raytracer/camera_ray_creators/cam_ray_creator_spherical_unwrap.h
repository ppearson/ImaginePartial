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

#ifndef CAM_RAY_CREATOR_SPHERICAL_UNWRAP_H
#define CAM_RAY_CREATOR_SPHERICAL_UNWRAP_H

#include "camera_ray_creator.h"

namespace Imagine
{

class CamRayCreatorSphericalUnwrap : public CameraRayCreator
{
public:
	CamRayCreatorSphericalUnwrap() : CameraRayCreator(),
		m_distanceFromOrigin(50.0f)
	{

	}

	virtual void init(const Params& params)
	{

	}

	virtual Ray createBasicCameraRay(float x, float y) const
	{
		const float theta = kPI * y * m_invHeight;
		const float phi = kPITimes2 * (m_width - x) * m_invWidth;

		const float sinTheta = sinf(theta);
		const float sinPhi = sinf(phi);
		const float cosPhi = cosf(phi);
		const float cosTheta = cosf(theta);

		Normal newDirection(sinTheta * sinPhi, cosTheta, sinTheta * cosPhi);
//		newDirection = m_transform.transform(newDirection);

		Point position = (Point)newDirection;
		position *= m_distanceFromOrigin;

		Point targetPoint(0.0f, 0.0f, 0.0f);

//		position = m_transform.transform(position);
//		targetPoint = m_transform.transform(targetPoint);

		newDirection = (Normal)(targetPoint - position);

		Ray ray1(position, newDirection, RAY_CAMERA);
		ray1.direction.normalise();
/*
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
*/
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
//		newDirection = m_transform.transform(newDirection);

		Point position = (Point)newDirection;
		position *= m_distanceFromOrigin;

		Point targetPoint(0.0f, 0.0f, 0.0f);

//		position = m_transform.transform(position);
//		targetPoint = m_transform.transform(targetPoint);

		newDirection = (Normal)(targetPoint - position);

		Ray ray1(position, newDirection, RAY_CAMERA);
		ray1.direction.normalise();
/*
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
*/
		return ray1;
	}

protected:
	float		m_distanceFromOrigin;
};

} // namespace Imagine

#endif // CAM_RAY_CREATOR_SPHERICAL_UNWRAP_H

