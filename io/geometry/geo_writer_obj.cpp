/*
 Imagine
 Copyright 2011-2018 Peter Pearson.

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

#include "geo_writer_obj.h"

#include <iomanip>
#include <fstream>

#include "geometry/editable_geometry_instance.h"
#include "geometry/standard_geometry_instance.h"
#include "geometry/editable_geometry_operations.h"

#include "object.h"
#include "objects/compound_object.h"
#include "objects/mesh.h"

namespace Imagine
{

GeoWriterObj::GeoWriterObj()
{
}

bool GeoWriterObj::writeFile(Object* pObject, const std::string& path, const GeoWriterOptions& options)
{
	if (!pObject)
		return false;

	std::fstream fileStream(path.c_str(), std::ios::out | std::ios::trunc);

	if (!fileStream)
	{
		return false;
	}
	
	bool writeNormals = options.exportNormals;

	Matrix4 overallTransform;
	bool transformValues = options.applyTransform;
	if (transformValues)
	{
		Vector rotation = pObject->getRotation();
		
		// TODO: for the moment, if there's a rotation component, we don't export the normals
		if (!rotation.isNull())
		{
			writeNormals = false;
		}
		
		overallTransform.translate(pObject->getPosition());
		overallTransform.rotate(rotation.x, rotation.y, rotation.z, Matrix4::eYXZ);
		float uniformScale = pObject->getUniformScale();
		overallTransform.scale(uniformScale, uniformScale, uniformScale);
		
		const Transform& objectTransform = pObject->transform();
		Vector objectScale = objectTransform.extractScale();
		if (!objectScale.isAllValue(objectScale.x))
		{
			// TODO: again, if we have a non-uniform scale, don't output normals for the moment
			writeNormals = false;
		}
		overallTransform.scale(objectScale.x, objectScale.y, objectScale.z);
	}

	bool writeUVs = options.exportUVs;

	CompoundObject* pCO = dynamic_cast<CompoundObject*>(pObject);
	if (!pCO)
	{
		GeometryInstanceGathered* pGeoInstance = pObject->getGeometryInstance();

		if (!pGeoInstance || (pGeoInstance->getTypeID() != 1 && pGeoInstance->getTypeID() != 3))
			return false;

		if (pGeoInstance->getTypeID() == 1)
		{
			EditableGeometryInstance* pEditableGeoInstance = dynamic_cast<EditableGeometryInstance*>(pGeoInstance);

			writeUVs = writeUVs && pEditableGeoInstance->hasPerVertexUVs();

			writeGeoInstanceEditablePoints(pEditableGeoInstance, fileStream, &overallTransform);

			if (writeUVs)
			{
				writeGeoInstanceEditableUVs(pEditableGeoInstance, fileStream);
			}
			
			if (writeNormals)
			{
				writeGeoInstanceEditableNormals(pEditableGeoInstance, fileStream);
			}

			writeGeoInstanceEditableFaces(pEditableGeoInstance, fileStream, 0, 0, writeUVs, 0, writeNormals);
		}
		else if (pGeoInstance->getTypeID() == 3)
		{
			StandardGeometryInstance* pStandardGeoInstance = dynamic_cast<StandardGeometryInstance*>(pGeoInstance);

			writeUVs = writeUVs && pStandardGeoInstance->hasPerVertexUVs();

			writeGeoInstanceStandardPoints(pStandardGeoInstance, fileStream, &overallTransform);

			if (writeUVs)
			{
				writeGeoInstanceStandardUVs(pStandardGeoInstance, fileStream);
			}
			
			if (writeNormals)
			{
				writeGeoInstanceStandardNormals(pStandardGeoInstance, fileStream);
			}

			writeGeoInstanceStandardFaces(pStandardGeoInstance, fileStream, 0, 0, writeUVs, 0, writeNormals);
		}
	}
	else
	{
		// write out as sub-objects if requested
		if (options.exportSubObjects)
		{
			unsigned int pointOffsetCount = 0;
			unsigned int uvsOffsetCount = 0;
			unsigned int normalsOffsetCount = 0;

			unsigned int subObjectCount = pCO->getSubObjectCount();
			for (unsigned int i = 0; i < subObjectCount; i++)
			{
				Object* pSubObject = pCO->getSubObject(i);
				
				// TODO: do this properly
				bool localWriteNormals = writeNormals && pSubObject->getRotation().isNull();
				
				bool localWriteUVs = writeUVs;

				Matrix4 subObjectTransform;
				subObjectTransform.translate(pSubObject->getPosition());
				subObjectTransform.rotate(pSubObject->getRotation().x, pSubObject->getRotation().y, pSubObject->getRotation().z, Matrix4::eYXZ);
				float uniformScale = pSubObject->getUniformScale();
				subObjectTransform.scale(uniformScale, uniformScale, uniformScale);
				
				Vector scale = pSubObject->transform().extractScale();
				if (!scale.isAllValue(scale.x))
				{
					// TODO: do this properly
					localWriteNormals = false;
				}
				subObjectTransform.scale(scale.x, scale.y, scale.z);

				Matrix4 combinedSubObjectTransform = overallTransform;
				combinedSubObjectTransform *= subObjectTransform;

				GeometryInstanceGathered* pGeoInstance = pSubObject->getGeometryInstance();

				if (!pGeoInstance || (pGeoInstance->getTypeID() != 1 && pGeoInstance->getTypeID() != 3))
					continue;

				if (pGeoInstance->getTypeID() == 1)
				{
					EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);

					fileStream << "# Object\n" << "g OBJECT\n";

					unsigned int pointsWritten = writeGeoInstanceEditablePoints(pEditableGeoInstance, fileStream, &combinedSubObjectTransform);
					
					if (localWriteUVs && !pEditableGeoInstance->hasPerVertexUVs())
					{
						localWriteUVs = false;
					}
					
					unsigned int uvsWritten = 0;
					unsigned int normalsWritten = 0;

					if (localWriteUVs)
					{
						uvsWritten = writeGeoInstanceEditableUVs(pEditableGeoInstance, fileStream);
					}
					
					if (localWriteNormals)
					{
						normalsWritten = writeGeoInstanceEditableNormals(pEditableGeoInstance, fileStream);
					}
					
					writeGeoInstanceEditableFaces(pEditableGeoInstance, fileStream, pointOffsetCount, uvsOffsetCount, localWriteUVs, normalsOffsetCount, localWriteNormals);
					
					uvsOffsetCount += uvsWritten;
					normalsOffsetCount += normalsWritten;
					pointOffsetCount += pointsWritten;
				}
				else
				{
					StandardGeometryInstance* pStandardGeoInstance = reinterpret_cast<StandardGeometryInstance*>(pGeoInstance);

					fileStream << "\n# Object\n" << "g OBJECT\n";

					unsigned int pointsWritten = writeGeoInstanceStandardPoints(pStandardGeoInstance, fileStream, &combinedSubObjectTransform);
					
					if (localWriteUVs && !pStandardGeoInstance->hasPerVertexUVs())
					{
						localWriteUVs = false;
					}
					
					unsigned int uvsWritten = 0;
					unsigned int normalsWritten = 0;

					if (localWriteUVs)
					{
						uvsWritten = writeGeoInstanceStandardUVs(pStandardGeoInstance, fileStream);
					}
					
					if (localWriteNormals)
					{
						normalsWritten = writeGeoInstanceStandardNormals(pStandardGeoInstance, fileStream);
					}
					
					writeGeoInstanceStandardFaces(pStandardGeoInstance, fileStream, pointOffsetCount, uvsOffsetCount, localWriteUVs, normalsOffsetCount, localWriteNormals);

					uvsOffsetCount += uvsWritten;
					normalsOffsetCount += normalsWritten;
					pointOffsetCount += pointsWritten;
				}
			}
		}
		else // otherwise, combine them all together into one object
		{
			// TODO: for the moment, cheat, and just don't state the new sub-objects, but this
			// means some readers won't be able to import it properly, so this needs to be done properly

			unsigned int pointOffsetCount = 0;
			unsigned int uvsOffsetCount = 0;
			unsigned int normalsOffsetCount = 0;

			unsigned int subObjectCount = pCO->getSubObjectCount();
			for (unsigned int i = 0; i < subObjectCount; i++)
			{
				Object* pSubObject = pCO->getSubObject(i);
				
				bool localWriteUVs = writeUVs;
				bool localWriteNormals = writeNormals;

				Matrix4 subObjectTransform;
				subObjectTransform.translate(pSubObject->getPosition());
				subObjectTransform.rotate(pSubObject->getRotation().x, pSubObject->getRotation().y, pSubObject->getRotation().z, Matrix4::eYXZ);
				float scale = pSubObject->getUniformScale();
				subObjectTransform.scale(scale, scale, scale);

				Matrix4 combinedSubObjectTransform = overallTransform;
				combinedSubObjectTransform *= subObjectTransform;

				GeometryInstanceGathered* pGeoInstance = pSubObject->getGeometryInstance();

				if (!pGeoInstance || (pGeoInstance->getTypeID() != 1 && pGeoInstance->getTypeID() != 3))
					continue;

				EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);
				
				if (localWriteUVs && !pEditableGeoInstance->hasPerVertexUVs())
				{
					localWriteUVs = false;
				}

				unsigned int pointsWritten = writeGeoInstanceEditablePoints(pEditableGeoInstance, fileStream, &combinedSubObjectTransform);
				unsigned int uvsWritten = 0;

				if (localWriteUVs)
				{
					uvsWritten = writeGeoInstanceEditableUVs(pEditableGeoInstance, fileStream);

					uvsOffsetCount += uvsWritten;
				}
				
				// write normals
				
				writeGeoInstanceEditableFaces(pEditableGeoInstance, fileStream, pointOffsetCount, uvsOffsetCount, localWriteUVs, normalsOffsetCount, localWriteNormals);

				pointOffsetCount += pointsWritten;
			}
		}
	}

	fileStream.close();

	return true;
}

unsigned int GeoWriterObj::writeGeoInstanceEditablePoints(EditableGeometryInstance* pGeoInstance, std::fstream& stream, Matrix4* pMatrix)
{
	// write out the points
	stream << "# Points\n";

	const std::deque<Point>& points = pGeoInstance->getPoints();
	std::deque<Point>::const_iterator it = points.begin();

	unsigned int pointsWritten = 0;

	if (pMatrix)
	{
		for (; it != points.end(); ++it)
		{
			Point point = pMatrix->transform(*it);

			stream << std::setprecision(6) << "v " << point.x << " " << point.y << " " << point.z << "\n";

			pointsWritten++;
		}
	}
	else
	{
		for (; it != points.end(); ++it)
		{
			const Point& point = *it;

			stream << std::setprecision(6) << "v " << point.x << " " << point.y << " " << point.z << "\n";

			pointsWritten++;
		}
	}

	return pointsWritten;
}

unsigned int GeoWriterObj::writeGeoInstanceEditableUVs(EditableGeometryInstance* pGeoInstance, std::fstream& stream)
{
	// write out the UVs
	stream << "# UVs\n";

	const std::deque<UV>& uvs = pGeoInstance->getUVs();
	std::deque<UV>::const_iterator it = uvs.begin();

	unsigned int uvsWritten = 0;

	for (; it != uvs.end(); ++it)
	{
		const UV& uv = *it;

		stream << std::setprecision(6) << "vt " << uv.u << " " << uv.v << "\n";

		uvsWritten++;
	}

	return uvsWritten;
}

unsigned int GeoWriterObj::writeGeoInstanceEditableNormals(EditableGeometryInstance* pGeoInstance, std::fstream& stream)
{
	// write out the Normals
	stream << "# Normals\n";
	
	// Note: we don't de-duplicate these, which obviously isn't efficient file-size-wise, but makes things
	//       much simplier.

	const std::deque<Normal>& normals = pGeoInstance->getNormals();
	std::deque<Normal>::const_iterator it = normals.begin();

	unsigned int normalsWritten = 0;

	for (; it != normals.end(); ++it)
	{
		const Normal& normal = *it;

		stream << std::setprecision(6) << "vn " << normal.x << " " << normal.y << " " << normal.z << "\n";

		normalsWritten++;
	}

	return normalsWritten;
}

void GeoWriterObj::writeGeoInstanceEditableFaces(EditableGeometryInstance* pGeoInstance, std::fstream& stream, unsigned int pointOffset,
										 unsigned int uvOffset, bool writeUVs, unsigned int normalOffset, bool writeNormals)
{
	stream << "\n\n# Faces\n";
	
	// convert to single value in the hope of helping branch predictor a bit...
	int indicesType = 0; // no UVs or Normals
	if (writeUVs && !writeNormals)
	{
		indicesType = 1;
	}
	else if (writeUVs && writeNormals)
	{
		indicesType = 2; // both
	}
	else
	{
		indicesType = 3; // no UVs
	}

	const std::deque<Face>& faces = pGeoInstance->getFaces();
	std::deque<Face>::const_iterator itFace = faces.begin();
	for (; itFace != faces.end(); ++itFace)
	{
		const Face& face = *itFace;

		unsigned int numVertices = face.getVertexCount();

		stream << "f";
		
		if (indicesType == 0)
		{
			// just point index
			for (unsigned int i = 0; i < numVertices; i++)
			{
				unsigned int vertexIndex = face.getVertexPosition(i);

				// just the point index
				stream << " " << vertexIndex + 1 + pointOffset;	
			}
		}
		else if (indicesType == 1)
		{
			// point index and UV index
			
			for (unsigned int i = 0; i < numVertices; i++)
			{
				unsigned int vertexIndex = face.getVertexPosition(i);
				unsigned int uvIndex = face.getVertexUV(i);
				
				stream << " " << vertexIndex + 1 + pointOffset << "/" << uvIndex + 1 + uvOffset << "/";
			}
		}
		else if (indicesType == 2)
		{
			// point index, UV index and Normal index
			
			for (unsigned int i = 0; i < numVertices; i++)
			{
				unsigned int vertexIndex = face.getVertexPosition(i);
				unsigned int uvIndex = face.getVertexUV(i);
				unsigned int normalIndex = face.getVertexNormal(i);
				
				stream << " " << vertexIndex + 1 + pointOffset << "/" << uvIndex + 1 + uvOffset << "/" << normalIndex + normalOffset;
			}
		}
		else if (indicesType == 3)
		{
			for (unsigned int i = 0; i < numVertices; i++)
			{
				unsigned int vertexIndex = face.getVertexPosition(i);
				unsigned int normalIndex = face.getVertexNormal(i);
				
				stream << " " << vertexIndex + 1 + pointOffset << "//" << normalIndex + normalOffset;
			}
		}

		stream << "\n";
	}
}

unsigned int GeoWriterObj::writeGeoInstanceStandardPoints(StandardGeometryInstance* pGeoInstance, std::fstream& stream, Matrix4* pMatrix)
{
	// write out the points
	stream << "# Points\n";

	const std::vector<Point>& points = pGeoInstance->getPoints();
	std::vector<Point>::const_iterator it = points.begin();

	unsigned int pointsWritten = 0;

	if (pMatrix)
	{
		for (; it != points.end(); ++it)
		{
			Point point = pMatrix->transform(*it);

			stream << std::setprecision(6) << "v " << point.x << " " << point.y << " " << point.z << "\n";

			pointsWritten++;
		}
	}
	else
	{
		for (; it != points.end(); ++it)
		{
			const Point& point = *it;

			stream << std::setprecision(6) << "v " << point.x << " " << point.y << " " << point.z << "\n";

			pointsWritten++;
		}
	}

	return pointsWritten;
}

unsigned int GeoWriterObj::writeGeoInstanceStandardUVs(StandardGeometryInstance* pGeoInstance, std::fstream& stream)
{
	// write out the UVs
	stream << "# UVs\n";

	const std::vector<UV>& uvs = pGeoInstance->getUVs();
	std::vector<UV>::const_iterator it = uvs.begin();

	unsigned int uvsWritten = 0;

	for (; it != uvs.end(); ++it)
	{
		const UV& uv = *it;

		stream << std::setprecision(6) << "vt " << uv.u << " " << uv.v << "\n";

		uvsWritten++;
	}

	return uvsWritten;
}

unsigned int GeoWriterObj::writeGeoInstanceStandardNormals(StandardGeometryInstance* pGeoInstance, std::fstream& stream)
{
	// write out the Normals
	stream << "# Normals\n";
	
	// Note: we don't de-duplicate these, which obviously isn't efficient file-size-wise, but makes things
	//       much simplier.

	const std::vector<Normal>& normals = pGeoInstance->getNormals();
	std::vector<Normal>::const_iterator it = normals.begin();

	unsigned int normalsWritten = 0;

	for (; it != normals.end(); ++it)
	{
		const Normal& normal = *it;

		stream << std::setprecision(6) << "vn " << normal.x << " " << normal.y << " " << normal.z << "\n";

		normalsWritten++;
	}

	return normalsWritten;
}

void GeoWriterObj::writeGeoInstanceStandardFaces(StandardGeometryInstance* pGeoInstance, std::fstream& stream, unsigned int pointOffset,
										 unsigned int uvOffset, bool writeUVs, unsigned int normalOffset, bool writeNormals)
{
	stream << "\n\n# Faces\n";
	
	// convert to single value in the hope of helping branch predictor a bit...
	int indicesType = 0; // no UVs or Normals
	if (writeUVs && !writeNormals)
	{
		indicesType = 1;
	}
	else if (writeUVs && writeNormals)
	{
		indicesType = 2; // both
	}
	else
	{
		indicesType = 3; // no UVs
	}

	
	const std::vector<uint32_t>& aPolyOffsets = pGeoInstance->getPolygonOffsets();
	const std::vector<uint32_t>& aPolyIndices = pGeoInstance->getPolygonIndices();
	
	const uint32_t* pUVIndices = pGeoInstance->getUVIndices();
	const uint32_t* pNormalIndices = pGeoInstance->getNormalIndices();

	unsigned int lastOffset = 0;

	std::vector<uint32_t>::const_iterator itPolyOffset = aPolyOffsets.begin();
	for (; itPolyOffset != aPolyOffsets.end(); ++itPolyOffset)
	{
		const uint32_t& offset = *itPolyOffset;

		unsigned int numVertices = offset - lastOffset;

		stream << "f";
		
		// TODO: need to cope with point-index-derived as well...
		
		if (indicesType == 0)
		{
			// just points
			for (unsigned int i = lastOffset; i < offset; i++)
			{
				unsigned int vertexIndex = aPolyIndices[i];

				stream << " " << vertexIndex + 1 + pointOffset;
			}
		}
		else if (indicesType == 1)
		{
			// points and UVs
			
			if (pUVIndices)
			{
				// we have UV indices
				for (unsigned int i = lastOffset; i < offset; i++)
				{
					unsigned int vertexIndex = aPolyIndices[i];
					unsigned int uvIndex = pUVIndices[i];
	
					stream << " " << vertexIndex + 1 + pointOffset << "/" << uvIndex + 1 + uvOffset << "/";
				}
			}
			else
			{
				// we're using implicit indices for the UVs
				for (unsigned int i = lastOffset; i < offset; i++)
				{
					unsigned int vertexIndex = aPolyIndices[i];
					unsigned int uvIndex = i;
	
					stream << " " << vertexIndex + 1 + pointOffset << "/" << uvIndex + 1 + uvOffset << "/";
				}
			}
		}
		else if (indicesType == 2)
		{
			// points, normals and UVs
			
			for (unsigned int i = lastOffset; i < offset; i++)
			{
				unsigned int vertexIndex = aPolyIndices[i];
				unsigned int uvIndex = pUVIndices ? pUVIndices[i] : i;
				unsigned int normalIndex = pNormalIndices ? pNormalIndices[i] : i;

				stream << " " << vertexIndex + 1 + pointOffset << "/" << uvIndex + 1 + uvOffset << "/" << normalIndex + 1 + normalOffset;
			}
		}
		else
		{
			// just points and normals
			if (pNormalIndices)
			{
				// we have normal indices
				for (unsigned int i = lastOffset; i < offset; i++)
				{
					unsigned int vertexIndex = aPolyIndices[i];
					unsigned int normalIndex = pNormalIndices[vertexIndex];
	
					stream << " " << vertexIndex + 1 + pointOffset << "//" << normalIndex + 1 + normalOffset;
				}
			}
			else
			{
				// we're using implicit indices for the Normals
				for (unsigned int i = lastOffset; i < offset; i++)
				{
					unsigned int vertexIndex = aPolyIndices[i];
					unsigned int normalIndex = i;
	
					stream << " " << vertexIndex + 1 + pointOffset << "//" << normalIndex + 1 + normalOffset;
				}
			}
		}

		stream << "\n";

		lastOffset += numVertices;
	}
}

} // namespace Imagine

namespace
{
	Imagine::GeoWriter* createGeoWriterObj()
	{
		return new Imagine::GeoWriterObj();
	}

	const bool registered = Imagine::FileIORegistry::instance().registerGeoWriter("obj", createGeoWriterObj);
}
