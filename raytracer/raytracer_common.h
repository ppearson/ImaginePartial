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
	SURFACE_IS_BACKFACING		= 1 << 1
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
	__finline HitResult() : triID(-1), haveDerivatives(false), time(0.0f), originType(RAY_UNDEFINED), raySharpness(0.0f), shadingFlags(0),
		intersectionError(0.0f), pObject(NULL),	pMedium(NULL), pLight(NULL), pBakedBSDF(NULL), pTriangleHolder(NULL),
		pCustom1(NULL), pShadingContext(NULL)
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

		hitPoint = Point();
		geometryNormal = Normal();
		shaderNormal = Normal();
		uv = UV();

		haveDerivatives = false;
		dpdx = Vector();
		dpdy = Vector();

		dPdu = Vector();
		dPdv = Vector();

		time = 0.0f;
		originType = RAY_UNDEFINED;
		raySharpness = 0.0f;
		shadingFlags = 0;
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

	void calculateUVTangents()
	{
		const float determinant = dUV1u * dUV2v - dUV2u * dUV1v;

		if (determinant == 0.0f)
		{
			FrameOrthoBasis frame(geometryNormal);
			dPdu = frame.getTangentU();
			dPdv = frame.getTangentV();
		}
		else
		{
			const float invDet = 1.0f / determinant;

			// interpolate the tangents
			dPdu = (dP1 * dUV2v - dP2 * dUV1v);
			dPdv = (dP1 * -dUV2u - dP2 * dUV1u);

			dPdu *= invDet;
			dPdv *= invDet;
		}
	}

	uint32_t					triID;

	float						beta;  // Barycentric u coordinate
	float						gamma; // Barycentric v coordinate

	Point						hitPoint;
	Normal						geometryNormal;
	Normal						shaderNormal;
	UV							uv;

	// Edges
	Vector						dP1;
	Vector						dP2;

	float						dUV1u;
	float						dUV1v;
	float						dUV2u;
	float						dUV2v;

	// surface derivative wrt U param
	Vector						dPdu;
	// surface derivative wrt V param
	Vector						dPdv;

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

	float						time;

	RayType						originType; // the type of ray which created the hit result
	float						raySharpness; // pretty crap hack, but... camera rays are 0.0f, the rougher surfaces are the more this increases

	unsigned int				shadingFlags;

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


#endif // RAYTRACER_COMMON_H
