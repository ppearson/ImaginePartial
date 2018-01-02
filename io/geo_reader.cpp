/*
 Imagine
 Copyright 2011-2012 Peter Pearson.

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

#include "geo_reader.h"

#include "objects/mesh.h"
#include "objects/compound_object.h"

#include "settings.h"

#include "core/hash.h"

#include "geometry/standard_geometry_instance.h"

#include "utils/maths/fixed.h"
#include "utils/maths/fixed.cpp"

namespace Imagine
{

GeoReader::GeoReader() : m_newObject(NULL), m_pScene(NULL)
{
}

GeoReader::~GeoReader()
{
}

void GeoReader::postProcess()
{
	m_newObject->recalculateBoundaryBoxFromGeometry();
	m_newObject->calculateTransformedBoundaryBox();
	m_newObject->updateBoundaryBox();

	if (m_readOptions.setBBoxDrawMode)
	{
		m_newObject->setDisplayType(eBoundaryBox);
	}

//	discardDuplicateGeometry(1);

	Settings& settings = Settings::instance();

	bool centreObject = m_readOptions.centreObject;
	bool scaleToFit = m_readOptions.scaleToFit;
	float scaleToFitSize = m_readOptions.scaleToFitSize;
	bool sitGeometryOnPlane = m_readOptions.standObjectOnPlane;

	if (scaleToFit)
	{
		unsigned int scaleToFitType = settings.getInt("import/scale_to_fit_type", 1);
		m_newObject->scaleObjectToFit(scaleToFitSize, scaleToFitType == 1);
	}

	if (centreObject)
	{
		m_newObject->repositionCentreOfObject();
	}

	if (centreObject)
	{
		m_newObject->centreObject(sitGeometryOnPlane);
	}
}

void GeoReader::removeDuplicatePoints(Object* pObject)
{
	GeometryInstanceGathered* pGeoInstance = pObject->getGeometryInstance();
	if (!pGeoInstance || pGeoInstance->getTypeID() != 1)
		return;

	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);

	std::deque<Point>& aObjectPoints = pEditableGeoInstance->getPoints();
//	std::deque<Normal>& aVertexNormals = getObjectVertexNormals();
	std::deque<Face>& aObjectFaces = pEditableGeoInstance->getFaces();

	Hash mainHash; // dummy object

	std::map<HashValue, unsigned int> aPointsMap;

	// map which points to replace
	std::map<unsigned int, unsigned int> aReplacementIndices;

	unsigned int duplicatePoints = 0;

	unsigned int pointIndex = 0;

	std::deque<Point>::iterator it = aObjectPoints.begin();
	for (; it != aObjectPoints.end(); ++it)
	{
		Point& point = *it;

		HashValue pointHash = mainHash.hashPoint(point);

		std::map<HashValue, unsigned int>::iterator itFind = aPointsMap.find(pointHash);
		if (itFind == aPointsMap.end())
		{
			aPointsMap[pointHash] = pointIndex;
		}
		else
		{
			// we've got it already, so add this point map
			unsigned int replacementPointIndex = (*itFind).second;

			aReplacementIndices[pointIndex] = replacementPointIndex;

			duplicatePoints ++;
		}

		pointIndex++;
	}

	if (aReplacementIndices.empty())
		return;

	// otherwise, replace

	std::deque<Face>::iterator itFace = aObjectFaces.begin();
	for (; itFace != aObjectFaces.end(); ++itFace)
	{

	}

	fprintf(stderr, "duplicate points: %u\t in %zu\n", duplicatePoints, aObjectPoints.size());
}

void GeoReader::discardDuplicateGeometry(unsigned short precisionThreshold)
{
	CompoundObject* pCO = dynamic_cast<CompoundObject*>(m_newObject);

	// only bother doing this for Compound Objects...
	if (!pCO)
		return;

	// note: due to float accuracy issues, this isn't very exact...

	// first of all, see if any Geometry Instances have boundary boxes of the same volume
	std::map<fixedpoint<2>, GeoInstanceOverview> aVolumes;

	unsigned int subObjectCount = pCO->getSubObjectCount();
	for (unsigned int i = 0; i < subObjectCount; i++)
	{
		Object* pSubObject = pCO->getSubObject(i);

		GeometryInstanceGathered* pGeoInstance = pSubObject->getGeometryInstance();

		if (!pGeoInstance || pGeoInstance->getTypeID() != 1)
			continue;

		EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);

//		float bbVolume = pGeoInstance->getBoundaryBox().volume();
		float bbVolume = pGeoInstance->getBoundaryBox().fixedVolume(precisionThreshold);

		fprintf(stderr, "GeoInst: %d\t volume: %f\n", i, bbVolume);

		fixedpoint<2> fbbVolume = bbVolume;

		std::map<fixedpoint<2>, GeoInstanceOverview>::iterator itFind = aVolumes.find(fbbVolume);
		if (itFind == aVolumes.end())
		{
			GeoInstanceOverview newItem;
			newItem.add(pEditableGeoInstance, pSubObject);
			aVolumes[bbVolume] = newItem;
		}
		else
		{
			GeoInstanceOverview& item = itFind->second;
			item.add(pEditableGeoInstance, pSubObject);
		}
	}

	std::vector<GeoInstanceOverview> aFinalList;

	// now filter out items with only one, or ones where the boundary boxes aren't exactly the same size...
	std::map<fixedpoint<2>, GeoInstanceOverview>::iterator it = aVolumes.begin();
	for (; it != aVolumes.end(); ++it)
	{
		GeoInstanceOverview& overviewItem = it->second;

		if (overviewItem.count <= 1)
			continue;

		// check bbox exactly

		bool allMatch = true;
		BoundaryBox firstBB = overviewItem.aGeoInstances[0]->getBoundaryBox();
		for (unsigned int i = 1; i < overviewItem.count; i++)
		{
			if (!overviewItem.aGeoInstances[i]->getBoundaryBox().fixedHasSameVolume(firstBB, precisionThreshold))
				allMatch = false;
		}

		if (allMatch)
			aFinalList.push_back(overviewItem);
	}

	fprintf(stderr, "Final list: %zu items.\n", aFinalList.size());

	// now swap out Duplicate GeoInstances, and offset parentObjects' positions to compensate
	std::vector<GeoInstanceOverview>::iterator itFinal = aFinalList.begin();
	for (; itFinal != aFinalList.end(); ++itFinal)
	{

	}
}

void GeoReader::applyMatrixToMesh(Matrix4& matrix, Mesh* pMesh, bool recalculateNormals)
{
	GeometryInstanceGathered* pGeoInstance = pMesh->getGeometryInstance();

	if (!pGeoInstance)
		return;

	if (pGeoInstance->getTypeID() == 1)
	{
		EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);

		std::deque<Point>& aPoints = pEditableGeoInstance->getPoints();
		std::deque<Face>& aFaces = pEditableGeoInstance->getFaces();

		std::deque<Point>::iterator it = aPoints.begin();
		std::deque<Point>::iterator itEnd = aPoints.end();

		for (; it != itEnd; ++it)
		{
			Point& point = *it;
			point = matrix.transform(point);
		}

		if (!recalculateNormals)
			return;

		// recalculate the face normals due to the change
		std::deque<Face>::iterator itFace = aFaces.begin();
		for (; itFace != aFaces.end(); ++itFace)
		{
			Face& face = *itFace;
			face.calculateNormal(pEditableGeoInstance);
		}
	}
	else if (pGeoInstance->getTypeID() == 3)
	{
		StandardGeometryInstance* pStandardGeoInstance = reinterpret_cast<StandardGeometryInstance*>(pGeoInstance);

		std::vector<Point>& aPoints = pStandardGeoInstance->getPoints();

		std::vector<Point>::iterator itPoint = aPoints.begin();
		for (; itPoint != aPoints.end(); ++itPoint)
		{
			Point& point = *itPoint;

			point = matrix.transform(point);
		}
	}
}

std::deque<Face>& GeoReader::getObjectFaces()
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(m_newObject->getGeometryInstance());
	return pEditableGeoInstance->getFaces();
}

std::deque<Point>& GeoReader::getObjectPoints()
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(m_newObject->getGeometryInstance());
	return pEditableGeoInstance->getPoints();
}

std::deque<Normal>& GeoReader::getObjectVertexNormals()
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(m_newObject->getGeometryInstance());
	return pEditableGeoInstance->getNormals();
}

std::deque<UV>& GeoReader::getObjectVertexUVs()
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(m_newObject->getGeometryInstance());
	return pEditableGeoInstance->getUVs();
}

std::deque<Point>& GeoReader::getSubObjectPoints(Mesh* subObject)
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(subObject->getGeometryInstance());
	return pEditableGeoInstance->getPoints();
}

std::deque<Face>& GeoReader::getSubObjectFaces(Mesh* subObject)
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(subObject->getGeometryInstance());
	return pEditableGeoInstance->getFaces();
}

std::deque<Normal>& GeoReader::getSubObjectVertexNormals(Mesh* subObject)
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(subObject->getGeometryInstance());
	return pEditableGeoInstance->getNormals();
}

std::deque<UV>& GeoReader::getSubObjectVertexUVs(Mesh* subObject)
{
	EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(subObject->getGeometryInstance());
	return pEditableGeoInstance->getUVs();
}

} // namespace Imagine
