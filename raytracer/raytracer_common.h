/*
 Imagine
 Copyright 2011-2015 Peter Pearson.

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

#ifndef RAYTRACER_COMMON_H
#define RAYTRACER_COMMON_H

#include <string.h> // for memset

#include "core/point.h"
#include "core/normal.h"
#include "core/vector.h"
#include "core/uv.h"
#include "core/ray.h"
#include "core/frame.h"

namespace Imagine
{

class Object;
class Medium;
class RenderTriangleHolder;
class Light;
class BakedBSDF;
class RenderThreadContext;

enum TileState
{
	eTSBlank,
	eTSInitial,
	eTSDraft,
	eTSAA,
	eTSAAAO,
	eTSDone
};

enum RenderTypeFlags
{
	RENDER_NORMAL = 1 << 1,
	RENDER_AMBIENT_OCCLUSION = 1 << 2,

	RENDER_ALL = RENDER_NORMAL | RENDER_AMBIENT_OCCLUSION
};

enum LightSamplingType
{
	eLSFullAllLights,					// sample all lights fully each hit
	eLSSampleLightsUniform,				// take n importance-sampled light samples per hit, weighting all lights equally
	eLSSampleLightsWeighted,			// take n importance-sampled light samples per hit, weighting lights based on light samples
	eLSSampleLightsRadiance,			// take n importance-sampled light samples per hit, weighting lights based on light radiance
	eLSSampleLightsRadianceWeighted,	// take n importance-sampled light samples per hit, weighting lights based on light radiance and sample weighting
	eLSSampleLightsRadianceLocalised	// take n importance-sampled light samples per hit, weighting lights based on light radiance + localised
};

enum StatisticsType
{
	eStatisticsNone,
	eStatisticsSimple,
	eStatisticsFull
};

enum StatisticsOutputType
{
	eStatsOutputConsole,
	eStatsOutputFile
};

enum ShadingFlags
{
	BSDF_IS_BACKLIT				= 1 << 0,
	SURFACE_IS_BACKFACING		= 1 << 1,
	SURFACE_SINGLE_SIDED		= 1 << 2,
	SURFACE_DOUBLE_SIDED		= 1 << 3
};

enum OtherHitResultFlags
{
	HIT_RESULT_SKIP_POST_INTERSECT	= 1 << 0
};

class RaytracerHost
{
public:
	virtual ~RaytracerHost()
	{
	}

	struct TileInfo
	{
		unsigned int		x;
		unsigned int		y;
		unsigned int		width;
		unsigned int		height;
		unsigned int		tileApronSize;
	};

	virtual void progressChanged(float) { }

	virtual void finished() { }

	virtual void tileDone(const TileInfo& tileInfo, unsigned int threadID) { }
};



class ShadingContext
{
public:
	ShadingContext(RenderThreadContext* pRenderThreadContext) : m_pRenderThreadContext(pRenderThreadContext)
	{

	}

	RenderThreadContext* getRenderThreadContext() const
	{
		return m_pRenderThreadContext;
	}


protected:
	RenderThreadContext*	m_pRenderThreadContext;
};

struct HitResult
{
	__finline HitResult() : triID(-1), objID(-1), haveDerivatives(false), eta(1.0f) ,time(0.0f), originType(RAY_UNDEFINED),
		rayWidth(0.0f), shadingFlags(0), otherFlags(0), intersectionError(0.0f), pObject(NULL), pMedium(NULL), pLight(NULL), pBakedBSDF(NULL),
		pTriangleHolder(NULL), pCustom1(NULL), pShadingContext(NULL)
	{
	}

	void setPointers(const Object* pObj, const Medium* pMed = NULL, const Light* pLi = NULL)
	{
		pObject = pObj;
		pMedium = pMed;
		pLight = pLi;
	}

	void reset()
	{
		triID = -1;
		objID = -1;

		hitPoint = Point();
		geometryNormal = Normal();
		shaderNormal = Normal();
		uv = UV();

		haveDerivatives = false;
		dpdx = Vector();
		dpdy = Vector();

		dpdu = Vector();
		dpdv = Vector();

		eta = 1.0f;
		time = 0.0f;
		originType = RAY_UNDEFINED;
		rayWidth = 0.0f;
		shadingFlags = 0;
		otherFlags = 0;
		intersectionError = 0.0f;
		pObject = NULL;
		pMedium = NULL;
		pLight = NULL;
		pBakedBSDF = NULL;
		pTriangleHolder = NULL;
		pCustom1 = NULL;
		// don't override pShadingContext
	}

	void setShadingContext(const ShadingContext& shadingContext)
	{
		pShadingContext = &shadingContext;
	}

	const ShadingContext* getShadingContext() const
	{
		return pShadingContext;
	}

	bool isBackfacing() const
	{
		return (shadingFlags & SURFACE_IS_BACKFACING);
	}
	
	bool isBackfacingSingleSided() const
	{
		return (shadingFlags & SURFACE_IS_BACKFACING) && (shadingFlags & SURFACE_SINGLE_SIDED);
	}
	
	bool isBackfacingDoubleSided() const
	{
		return (shadingFlags & SURFACE_IS_BACKFACING) && (shadingFlags & SURFACE_DOUBLE_SIDED);
	}

	void calculateInitialDerivatives()
	{
		const float determinant = dUV1u * dUV2v - dUV2u * dUV1v;

		if (determinant == 0.0f)
		{
			FrameOrthoBasis frame(geometryNormal);
			dpdu = frame.getTangentU();
			dpdv = frame.getTangentV();

			// don't do anything for normal derivatives, leave them as is (at 0,0,0)
		}
		else
		{
			const float invDet = 1.0f / determinant;

			// interpolate the tangents
			dpdu = (dp10 * dUV2v - dp20 * dUV1v);
			dpdv = (dp10 * -dUV2u + dp20 * dUV1u);

			dpdu *= invDet;
			dpdv *= invDet;

			// normal derivatives
			dndu = (dn1 * dUV2v - dn2 * dUV1v) * invDet;
			dndv = (dn1 * -dUV2u + dn2 * dUV1u) * invDet;
		}
	}

	uint32_t					triID;
	uint32_t					objID;

	float						beta;  // Barycentric u coordinate
	float						gamma; // Barycentric v coordinate

	Point						hitPoint; // hit point in world space
	Normal						geometryNormal;
	Normal						shaderNormal;
	UV							uv;

	// Edges
	Vector						dp10;
	Vector						dp20;

	// used for working out final partial dertivatives - in the future these could
	// be calculated on-the-fly
	float						dUV1u;
	float						dUV1v;
	float						dUV2u;
	float						dUV2v;

	// used for working out final partial dertivatives - in the future these could
	// be calculated on-the-fly
	Normal						dn1;
	Normal						dn2;

	// surface partial derivative wrt U param
	Vector						dpdu;
	// surface partial derivative wrt V param
	Vector						dpdv;

	// normal partial derivatives
	Normal						dndu;
	Normal						dndv;

	// U derivative wrt screen space
	float						dudx;
	float						dudy;
	// V derivative wrt screen space
	float						dvdx;
	float						dvdy;

	// offsets from hitPoint of the differential rays
	Vector						dpdx;
	Vector						dpdy;

	bool						haveDerivatives;

	float						eta;

	float						time;

	RayType						originType; // the type of ray which created the hit result
	float						rayWidth; // pretty crap hack, but... camera rays are 1.0f, the rougher surfaces are the more this increases

	unsigned int				shadingFlags;
	unsigned int				otherFlags;

	float						intersectionError;

	const Object*				pObject;
	const Medium*				pMedium;
	const Light*				pLight;
	const BakedBSDF*			pBakedBSDF;
	const RenderTriangleHolder*	pTriangleHolder;

	void*						pCustom1;

	// this is hacky, but it's by far the least-intrusive way of doing things,
	// given this has to propagate through integrators, acceleration structures,
	// BSDFs and textures...
	const ShadingContext*		pShadingContext;
};

struct SelectionHitResult
{
	SelectionHitResult() : m_count(0), m_faceIndex(0)
	{
		memset(m_pObjects, 0, 4 * sizeof(Object*));
	}

	const Object*	m_pObjects[4];
	unsigned int	m_count;

	// 0 == no face, otherwise the face index is m_faceIndex - 1
	unsigned int	m_faceIndex;
};


} // namespace Imagine

#endif // RAYTRACER_COMMON_H
