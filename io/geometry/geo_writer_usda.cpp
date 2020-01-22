/*
 Imagine
 Copyright 2019-2020 Peter Pearson.

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

#include "geo_writer_usda.h"

#include <iomanip>
#include <sstream>
#include <cctype>

#include "geometry/editable_geometry_instance.h"
#include "geometry/standard_geometry_instance.h"
#include "geometry/editable_geometry_operations.h"

#include "object.h"
#include "objects/compound_object.h"
#include "objects/mesh.h"
#include "objects/primitives/sphere.h"

namespace Imagine
{

/*
#usda 1.0

def Xform "World"
{
    def Mesh "cube1"
    {
    float3[] extent = [(-1, -1, -1), (1, 1, 1)]
    int[] faceVertexCounts = [4, 4, 4, 4, 4, 4]
    int[] faceVertexIndices =  [0, 1, 2, 3, 4, 5, 6, 7, 0, 6, 5, 1, 4, 7, 3, 2, 0, 3, 7, 6, 4, 2, 1, 5]
    float3[] points = [(1, 1, 1), (-1, 1, 1), (-1, -1, 1), (1, -1, 1), (-1, -1, -1), (-1, 1, -1), (1, 1, -1), (1, -1, -1)]
    color3f[] primvars:displayColor = [(1, 1, 1), (0, 1, 1), (0, 0, 1), (1, 0, 1), (0, 0, 0), (0, 1, 0), (1, 1, 0), (1, 0, 0)] (
        interpolation = "vertex"
    )
    uniform token subdivisionScheme = "none"
   }
}

*/

GeoWriterUSDA::GeoWriterUSDA()
{
	
}

bool GeoWriterUSDA::writeFile(Object* pObject, const std::string& path, const GeoWriterOptions& options)
{
	std::fstream fileStream(path.c_str(), std::ios::out | std::ios::trunc);
	if (!fileStream)
	{
		return false;
	}
	
	// write pre-amble...
	fileStream << "#usda 1.0" << std::endl;
	fileStream << "# Written by Imagine." << std::endl;
	fileStream << std::endl;

	// this isn't *really* necessary, but...
	fileStream << "def Xform \"World\"" << std::endl;
	fileStream << "{" << std::endl;

	ProcessContext processContext(fileStream, options);
	
	writeObject(processContext, pObject, 1);
	
	fileStream << "}" << std::endl;
	
	fileStream.close();
	return true;
}

bool GeoWriterUSDA::writeObject(ProcessContext& processContext, const Object* pObject, unsigned int nestLevel)
{
	if (pObject->getObjectType() == ePrimitive && pObject->getObjectTypeID() == 9)
	{
		// check it's an actual plain sphere
		const Sphere* pSphere = dynamic_cast<const Sphere*>(pObject);
		if (pSphere->isPureSphere())
		{
			// it's a sphere

			std::string validUSDObjectName = generateValidObjectName(pObject->getName(), processContext.counts, eObjTypeSphere);

			writeStringLine(processContext.stream, nestLevel, "def Sphere \"" + validUSDObjectName + "\"");
			writeStringLine(processContext.stream, nestLevel, "{");
			writeObjectState(processContext.stream, pObject, processContext.options, nestLevel + 1);
			
			if (pSphere->getRadius() != 1.0f)
			{
				char szTemp[16];
				sprintf(szTemp, "%f", pSphere->getRadius());
				
				std::string radiusLine = "double radius = ";
				radiusLine.append(szTemp);
				
				writeStringLine(processContext.stream, nestLevel + 1, radiusLine);
			}
			writeStringLine(processContext.stream, nestLevel, "}");
		}
		else
		{
			// otherwise, it's had some modifications made to it, or it's a capped sphere, so write out the mesh
			writeMesh(processContext, pObject, nestLevel);
		}
	}
	else if (pObject->getObjectType() == eGeoMesh)
	{
		writeMesh(processContext, pObject, nestLevel);
	}
	else if (pObject->getObjectType() == eCollection && pObject->getObjectTypeID() == 14)
	{
		// write an Xform statement

		std::string validUSDObjectName = generateValidObjectName(pObject->getName(), processContext.counts, eObjTypeGroup);

		std::string groupStatementDef = "def Xform \"" + validUSDObjectName + "\"";

		// TODO: do we want a 'kind' ?

		writeStringLine(processContext.stream, nestLevel, groupStatementDef);
		writeStringLine(processContext.stream, nestLevel, "{");

		writeObjectState(processContext.stream, pObject, processContext.options, nestLevel + 1);

		// blank line...
		writeStringLine(processContext.stream, nestLevel + 1, "");

		// recurse into children
		const CompoundObject* pCO = dynamic_cast<const CompoundObject*>(pObject);
		if (pCO)
		{
			unsigned int subObjectCount = pCO->getSubObjectCount();
			for (unsigned int i = 0; i < subObjectCount; i++)
			{
				const Object* pSubObject = pCO->getSubObject(i);

				if (i > 0)
				{
					// blank line...
					writeStringLine(processContext.stream, nestLevel + 1, "");
				}

				writeObject(processContext, pSubObject, nestLevel + 1);
			}
		}

		writeStringLine(processContext.stream, nestLevel, "}");
	}
	else
	{
		return false;
	}
	
	return true;
}

void GeoWriterUSDA::writeMesh(ProcessContext& processContext, const Object* pObject, unsigned int nestLevel)
{
	const GeometryInstance* pGeoInstance = pObject->getGeometryInstance();
	if (!pGeoInstance)
		return;
	
	std::string validUSDObjectName = generateValidObjectName(pObject->getName(), processContext.counts, eObjTypeMesh);
	writeStringLine(processContext.stream, nestLevel, "def Mesh \"" + validUSDObjectName + "\"");
	writeStringLine(processContext.stream, nestLevel, "{");
	
	// write bounds extent
	std::string extentStatement = "float3[] extent = [(";
	extentStatement += vectorValsToString(pObject->getBoundaryBox().getMinimum());
	extentStatement += "), (";
	extentStatement += vectorValsToString(pObject->getBoundaryBox().getMaximum());
	extentStatement += ")]";
	
	writeStringLine(processContext.stream, nestLevel + 1, extentStatement);
	
	// write vertices
	
	if (pGeoInstance->getTypeID() == 1)
	{
		// editable instance
		
		std::stringstream ssVertexCounts;
		std::stringstream ssVertexIndices;
		
		const EditableGeometryInstance* pEditableGeoInstance = dynamic_cast<const EditableGeometryInstance*>(pGeoInstance);
		
		bool exportUVs = processContext.options.exportUVs && pEditableGeoInstance->hasPerVertexUVs();
		
		std::stringstream ssSTIndices;
				
		const std::deque<Face>& faces = pEditableGeoInstance->getFaces();
		std::deque<Face>::const_iterator itFace = faces.begin();
		unsigned int faceCount = 0;
		for (; itFace != faces.end(); ++itFace, faceCount++)
		{
			const Face& face = *itFace;
	
			unsigned int numVertices = face.getVertexCount();
	
			if (faceCount > 0)
			{
				ssVertexCounts << ", ";
			}
						
			ssVertexCounts << numVertices;
			
			for (unsigned int i = 0; i < numVertices; i++)
			{
				unsigned int vertexIndex = face.getVertexPosition(i);
				
				if (!(faceCount == 0 && i == 0))
				{
					ssVertexIndices << ", ";
					
					if (exportUVs)
					{
						ssSTIndices << ", ";
					}
				}

				ssVertexIndices << " " << vertexIndex;
				
				if (exportUVs)
				{
					ssSTIndices << " " << face.getVertexUV(i);
				}
			}
		}
		
		std::string faceVertexCountStatement = "int[] faceVertexCounts = [" + ssVertexCounts.str() + "]";
		
		std::string faceVertexIndicesStatement = "int[] faceVertexIndices = [" + ssVertexIndices.str() + "]";
		
		writeStringLine(processContext.stream, nestLevel + 1, faceVertexCountStatement);
		writeStringLine(processContext.stream, nestLevel + 1, faceVertexIndicesStatement);
		
		// now the points
		
		std::stringstream ssPoints;
		
		unsigned int pointCount = 0;
		
		const std::deque<Point>& points = pEditableGeoInstance->getPoints();
		for (const Point& point : points)
		{
			if (pointCount++ > 0)
			{
				ssPoints << ", ";
			}
			
			ssPoints << "(";
			ssPoints << pointValsToString(point);
			ssPoints << ")";
		}
		
		std::string pointsStatement = "point3f[] points = [" + ssPoints.str() + "]";
		
		writeStringLine(processContext.stream, nestLevel + 1, pointsStatement);

		// now any UVs
		if (exportUVs)
		{
			std::stringstream ssUVs;

			unsigned int uvCount = 0;

			const std::deque<UV>& uvs = pEditableGeoInstance->getUVs();
			for (const UV& uv : uvs)
			{
				if (uvCount++ > 0)
				{
					ssUVs << ", ";
				}

				ssUVs << "(";
				ssUVs << uvValsToString(uv);
				ssUVs << ")";
			}

			std::string stsStatement = "texCoord2f[] primvars:st = [" + ssUVs.str() + "] (";

			writeStringLine(processContext.stream, nestLevel + 1, stsStatement);

			// TODO: do this properly...
			if (uvs.size() == points.size())
			{
				writeStringLine(processContext.stream, nestLevel + 2, "interpolation = \"vertex\"");
			}
			else
			{
				writeStringLine(processContext.stream, nestLevel + 2, "interpolation = \"faceVarying\"");
			}
			writeStringLine(processContext.stream, nestLevel + 1, ")");
						
			std::string stIndicesStatement = "int[] primvars:st:indices = [" + ssSTIndices.str() + "]";
			writeStringLine(processContext.stream, nestLevel + 1, stIndicesStatement);

			/*
			texCoord2f[] primvars:st = [(0, 0), (0, 1), (1, 0), (1, 1), (0, 0), (0, 1), (1, 0), (1, 1), (1, 1), (0, 0), (0, 1), (1, 0), (1, 1)] (
			interpolation = "faceVarying"
		)
		*/
		}
		
		writeStringLine(processContext.stream, nestLevel + 1, "uniform token subdivisionScheme = \"none\"");
	}
	else if (pGeoInstance->getTypeID() == 3)
	{
		std::stringstream ssVertexCounts;
		std::stringstream ssVertexIndices;
		
		const StandardGeometryInstance* pStandardGeoInstance = dynamic_cast<const StandardGeometryInstance*>(pGeoInstance);
		
		bool exportUVs = processContext.options.exportUVs && pStandardGeoInstance->hasPerVertexUVs();
		
		std::stringstream ssSTIndices;
		
		const std::vector<uint32_t>& aPolyOffsets = pStandardGeoInstance->getPolygonOffsets();
		const std::vector<uint32_t>& aPolyIndices = pStandardGeoInstance->getPolygonIndices();
		
		unsigned int polyOffsetCount = 0;
		unsigned int lastOffset = 0;
		for (uint32_t polyOffset : aPolyOffsets)
		{
			if (polyOffsetCount++ > 0)
			{
				ssVertexCounts << ", ";
			}
			
			unsigned int thisCount = polyOffset - lastOffset;
			
			ssVertexCounts << thisCount;
			
			lastOffset = polyOffset;
		}
		
		unsigned int polyIndicesCount = 0;
		for (uint32_t polyIndex : aPolyIndices)
		{
			if (polyIndicesCount++ > 0)
			{
				ssVertexIndices << ", ";
			}
			
			ssVertexIndices << polyIndex;
		}
		
		std::string faceVertexCountStatement = "int[] faceVertexCounts = [" + ssVertexCounts.str() + "]";
		
		std::string faceVertexIndicesStatement = "int[] faceVertexIndices = [" + ssVertexIndices.str() + "]";
		
		writeStringLine(processContext.stream, nestLevel + 1, faceVertexCountStatement);
		writeStringLine(processContext.stream, nestLevel + 1, faceVertexIndicesStatement);
		
		// now the points
		
		std::stringstream ssPoints;
		
		unsigned int pointCount = 0;
		
		const std::vector<Point>& points = pStandardGeoInstance->getPoints();
		for (const Point& point : points)
		{
			if (pointCount++ > 0)
			{
				ssPoints << ", ";
			}
			
			ssPoints << "(";
			ssPoints << pointValsToString(point);
			ssPoints << ")";
		}
		
		std::string pointsStatement = "point3f[] points = [" + ssPoints.str() + "]";
		
		writeStringLine(processContext.stream, nestLevel + 1, pointsStatement);
		
		// TODO: UVs and Normals...
		
		const uint32_t* pUVIndices = pStandardGeoInstance->getUVIndices();
		const uint32_t* pNormalIndices = pStandardGeoInstance->getNormalIndices();
		
		writeStringLine(processContext.stream, nestLevel + 1, "uniform token subdivisionScheme = \"none\"");
	}
	
	// blank line
	writeStringLine(processContext.stream, nestLevel + 1, "");

	writeObjectState(processContext.stream, pObject, processContext.options, nestLevel + 1);

	writeStringLine(processContext.stream, nestLevel, "}");
}

void GeoWriterUSDA::writeObjectState(std::fstream& stream, const Object* pObject, const GeoWriterOptions& options, unsigned int nestLevel)
{
	// write Transform
	
	bool haveTranslate = false;
	bool haveRotate = false;
	bool haveScale = false;
	
	const Transform& transform = pObject->transform();
	if (!transform.position().isNull())
	{
		const Vector translate = transform.position().getVector();
		
		std::string transformFullStatements = "double3 xformOp:translate = (";
		transformFullStatements += vectorValsToString(translate);
		transformFullStatements += ")";
		
		writeStringLine(stream, nestLevel, transformFullStatements);
		
		haveTranslate = true;
	}
	
	if (!transform.rotation().isNull())
	{
		const Vector rotate = transform.rotation().getVector();
		
		std::string rotationFullStatements = "double3 xformOp:rotateYXZ = (";
		rotationFullStatements += vectorValsToString(rotate);
		rotationFullStatements += ")";
		
		writeStringLine(stream, nestLevel, rotationFullStatements);
		
		haveRotate = true;
	}
	
	if (haveTranslate && haveRotate)
	{
		writeStringLine(stream, nestLevel, "uniform token[] xformOpOrder = [\"xformOp:translate\", \"xformOp:rotateYXZ\"]");
	}
	else if (haveTranslate)
	{
		writeStringLine(stream, nestLevel, "uniform token[] xformOpOrder = [\"xformOp:translate\"]");
	}
	else if (haveRotate)
	{
		writeStringLine(stream, nestLevel, "uniform token[] xformOpOrder = [\"xformOp:rotateYXZ\"]");
	}
}

void GeoWriterUSDA::writeStringLine(std::fstream& stream, unsigned int nestLevel, const std::string& stringVal)
{
	for (unsigned int i = 0; i < nestLevel; i++)
	{
		stream << "    ";
	}
	
	stream << stringVal;
	stream << std::endl;
}

std::string GeoWriterUSDA::vectorValsToString(const Vector& vec)
{
	char szTemp[128];
	sprintf(szTemp, "%g, %g, %g", vec.x, vec.y, vec.z);
	
	std::string finalVal(szTemp);
	return finalVal;
}

std::string GeoWriterUSDA::pointValsToString(const Point& point)
{
	char szTemp[128];
	sprintf(szTemp, "%g, %g, %g", point.x, point.y, point.z);
	
	std::string finalVal(szTemp);
	return finalVal;
}

std::string GeoWriterUSDA::uvValsToString(const UV& uv)
{
	char szTemp[64];
	sprintf(szTemp, "%g, %g", uv.u, uv.v);

	std::string finalVal(szTemp);
	return finalVal;
}

// this is rather primitive and likely excessive, but it provides useful benefits when exporting objects that have been imported from things like Obj files
// which often don't have valid names...
std::string GeoWriterUSDA::generateValidObjectName(const std::string& originalName, ObjectTypeNameCounts& nameCounts, ObjectType objectType)
{
	// Note: technically speaking, names in USD for meshes, group scopes, etc, only have to be unique per
	//       scope, so we could relax the uniqueness restrictions a bit by scoping things properly, but that's a bit more involved, as we'd
	//       need a stack to push/pop stuff...

	if (!originalName.empty() && !std::isdigit(originalName[0]))
	{
		// this name is acceptable if it hasn't been used already...

		bool hasBeenUsedAlready = m_aUsedNames.count(originalName) > 0;
		if (hasBeenUsedAlready)
		{
			// we need to generate a unique one
			std::string candidateName = buildName(nameCounts, objectType);
			while (m_aUsedNames.count(candidateName) > 0)
			{
				candidateName = buildName(nameCounts, objectType);
			}

			m_aUsedNames.insert(candidateName);

			return candidateName;
		}
		else
		{
			// we can use the original...

			m_aUsedNames.insert(originalName);

			return originalName;
		}
	}
	else if (originalName.empty())
	{
		// generate a new one
		std::string candidateName = buildName(nameCounts, objectType);
		while (m_aUsedNames.count(candidateName) > 0)
		{
			candidateName = buildName(nameCounts, objectType);
		}

		m_aUsedNames.insert(candidateName);

		return candidateName;
	}
	else
	{
		// on the assumption that the original name might be useful if prefixed with something valid, try just sticking a simple prefix on
		// to begin with...

		unsigned int prefixAttempts = 0;

		static const unsigned int maxPrefixAttempts = 10;

		std::string candidateName = prependObjectTypeName(originalName, objectType) + "_" + std::to_string(prefixAttempts);

		while (m_aUsedNames.count(candidateName) > 0 && prefixAttempts < maxPrefixAttempts)
		{
			prefixAttempts++;
			candidateName = prependObjectTypeName(originalName, objectType) + "_" + std::to_string(prefixAttempts);
		}

		if (prefixAttempts >= maxPrefixAttempts)
		{
			// give up, and just generate a generic one...
			candidateName = buildName(nameCounts, objectType);
			while (m_aUsedNames.count(candidateName) > 0)
			{
				candidateName = buildName(nameCounts, objectType);
			}
		}

		m_aUsedNames.insert(candidateName);

		return candidateName;
	}
}

// Note: this increments the object type count
std::string GeoWriterUSDA::buildName(ObjectTypeNameCounts& nameCounts, ObjectType objectType)
{
	std::string finalName;

	if (objectType == eObjTypeGroup)
	{
		finalName = "Group" + std::to_string(nameCounts.groupCount++);
	}
	else if (objectType == eObjTypeMesh)
	{
		finalName = "Mesh" + std::to_string(nameCounts.meshCount++);
	}
	else if (objectType == eObjTypeSphere)
	{
		finalName = "Sphere" + std::to_string(nameCounts.sphereCount++);
	}

	return finalName;
}

std::string GeoWriterUSDA::prependObjectTypeName(const std::string& originalName, ObjectType objectType)
{
	if (objectType == eObjTypeGroup)
	{
		return "Grp_" + originalName;
	}
	else if (objectType == eObjTypeMesh)
	{
		return "Msh_" + originalName;
	}
	else if (objectType == eObjTypeSphere)
	{
		return "Sphr_" + originalName;
	}
	else
	{
		return "Othr_" + originalName;
	}
}

} // namespace Imagine

namespace
{
	Imagine::GeoWriter* createGeoWriterUSDA()
	{
		return new Imagine::GeoWriterUSDA();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerGeoWriter("usda", createGeoWriterUSDA);
}

