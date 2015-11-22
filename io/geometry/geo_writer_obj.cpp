/*
 Imagine
 Copyright 2011-2014 Peter Pearson.

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

#include "geometry/editable_geometry_operations.h"
#include "object.h"
#include "objects/compound_object.h"
#include "objects/mesh.h"

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

	Matrix4 overallTransform;
	bool transformValues = options.applyTransform;
	if (transformValues)
	{
		Vector rotation = pObject->getRotation();
		overallTransform.translate(pObject->getPosition());
		overallTransform.rotate(rotation.x, rotation.y, rotation.z, Matrix4::eYXZ);
		float scale = pObject->getUniformScale();
		overallTransform.scale(scale, scale, scale);
	}

	bool writeUVs = true;

	CompoundObject* pCO = dynamic_cast<CompoundObject*>(pObject);
	if (!pCO)
	{
		GeometryInstanceGathered* pGeoInstance = pObject->getGeometryInstance();

		if (!pGeoInstance || pGeoInstance->getTypeID() != 1)
			return false;

		EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);

		writeUVs = writeUVs && pEditableGeoInstance->hasPerVertexUVs();

		writeGeoInstancePoints(pEditableGeoInstance, fileStream, &overallTransform);

		if (writeUVs)
		{
			writeGeoInstanceUVs(pEditableGeoInstance, fileStream);
		}

		writeGeoInstanceFaces(pEditableGeoInstance, fileStream, 0, 0, writeUVs);
	}
	else
	{
		// write out as sub-objects if requested
		if (options.exportSubObjects)
		{
			unsigned int pointOffsetCount = 0;
			unsigned int uvsOffsetCount = 0;

			unsigned int subObjectCount = pCO->getSubObjectCount();
			for (unsigned int i = 0; i < subObjectCount; i++)
			{
				Object* pSubObject = pCO->getSubObject(i);

				Matrix4 subObjectTransform;
				subObjectTransform.translate(pSubObject->getPosition());
				subObjectTransform.rotate(pSubObject->getRotation());
				float scale = pSubObject->getUniformScale();
				subObjectTransform.scale(scale, scale, scale);

				Matrix4 combinedSubObjectTransform = overallTransform;
				combinedSubObjectTransform *= subObjectTransform;

				GeometryInstanceGathered* pGeoInstance = pSubObject->getGeometryInstance();

				if (!pGeoInstance || pGeoInstance->getTypeID() != 1)
					continue;

				EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);

				fileStream << "\n# Object\n" << "g OBJECT\n";

				unsigned int pointsWritten = writeGeoInstancePoints(pEditableGeoInstance, fileStream, &combinedSubObjectTransform);

				if (writeUVs && pEditableGeoInstance->hasPerVertexUVs())
				{
					unsigned int uvsWritten = writeGeoInstanceUVs(pEditableGeoInstance, fileStream);

					writeGeoInstanceFaces(pEditableGeoInstance, fileStream, pointOffsetCount, uvsOffsetCount, true);

					uvsOffsetCount += uvsWritten;
				}
				else
				{
					// don't write UVs
					writeGeoInstanceFaces(pEditableGeoInstance, fileStream, pointOffsetCount, 0, false);
				}

				pointOffsetCount += pointsWritten;
			}
		}
		else // otherwise, combine them all together into one object
		{
			// TODO: for the moment, cheat, and just don't state the new sub-objects, but this
			// means some readers won't be able to import it properly, so this needs to be done properly

			unsigned int pointOffsetCount = 0;
			unsigned int uvsOffsetCount = 0;

			unsigned int subObjectCount = pCO->getSubObjectCount();
			for (unsigned int i = 0; i < subObjectCount; i++)
			{
				Object* pSubObject = pCO->getSubObject(i);

				Matrix4 subObjectTransform;
				subObjectTransform.translate(pSubObject->getPosition());
				subObjectTransform.rotate(pSubObject->getRotation());
				float scale = pSubObject->getUniformScale();
				subObjectTransform.scale(scale, scale, scale);

				Matrix4 combinedSubObjectTransform = overallTransform;
				combinedSubObjectTransform *= subObjectTransform;

				GeometryInstanceGathered* pGeoInstance = pSubObject->getGeometryInstance();

				if (!pGeoInstance || pGeoInstance->getTypeID() != 1)
					continue;

				EditableGeometryInstance* pEditableGeoInstance = reinterpret_cast<EditableGeometryInstance*>(pGeoInstance);

				unsigned int pointsWritten = writeGeoInstancePoints(pEditableGeoInstance, fileStream, &combinedSubObjectTransform);

				if (writeUVs && pEditableGeoInstance->hasPerVertexUVs())
				{
					unsigned int uvsWritten = writeGeoInstanceUVs(pEditableGeoInstance, fileStream);

					writeGeoInstanceFaces(pEditableGeoInstance, fileStream, pointOffsetCount, uvsOffsetCount, true);

					uvsOffsetCount += uvsWritten;
				}
				else
				{
					// don't write UVs
					writeGeoInstanceFaces(pEditableGeoInstance, fileStream, pointOffsetCount, 0, false);
				}

				pointOffsetCount += pointsWritten;
			}
		}
	}

	fileStream.close();

	return true;
}

unsigned int GeoWriterObj::writeGeoInstancePoints(EditableGeometryInstance* pGeoInstance, std::fstream& stream, Matrix4* pMatrix)
{
	// write out the points
	stream << "# Points\n";

	std::deque<Point>& points = pGeoInstance->getPoints();
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

unsigned int GeoWriterObj::writeGeoInstanceUVs(EditableGeometryInstance* pGeoInstance, std::fstream& stream)
{
	// write out the UVs
	stream << "# UVs\n";

	std::deque<UV>& uvs = pGeoInstance->getUVs();
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

void GeoWriterObj::writeGeoInstanceFaces(EditableGeometryInstance* pGeoInstance, std::fstream& stream, unsigned int pointOffset,
										 unsigned int uvOffset, bool writeUVs)
{
	stream << "\n\n# Faces\n";

	if (!writeUVs)
	{
		std::deque<Face>& faces = pGeoInstance->getFaces();
		std::deque<Face>::iterator itFace = faces.begin();
		for (; itFace != faces.end(); ++itFace)
		{
			const Face& face = *itFace;

			unsigned int numVertices = face.getVertexCount();

			stream << "f";
			for (unsigned int i = 0; i < numVertices; i++)
			{
				unsigned int vertexIndex = face.getVertexPosition(i);

				stream << " " << vertexIndex + 1 + pointOffset;
			}

			stream << "\n";
		}
	}
	else
	{
		std::deque<Face>& faces = pGeoInstance->getFaces();
		std::deque<Face>::iterator itFace = faces.begin();
		for (; itFace != faces.end(); ++itFace)
		{
			const Face& face = *itFace;

			unsigned int numVertices = face.getVertexCount();

			stream << "f";
			for (unsigned int i = 0; i < numVertices; i++)
			{
				unsigned int vertexIndex = face.getVertexPosition(i);
				unsigned int uvIndex = face.getVertexUV(i);

				stream << " " << vertexIndex + 1 + pointOffset << "/" << uvIndex + 1 + uvOffset << "/";
			}

			stream << "\n";
		}
	}
}

namespace
{
	GeoWriter* createGeoWriterObj()
	{
		return new GeoWriterObj();
	}

	const bool registered = FileIORegistry::instance().registerGeoWriter("obj", createGeoWriterObj);
}
