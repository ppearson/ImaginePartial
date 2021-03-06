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

#ifndef GEO_READER_H
#define GEO_READER_H

#include <string>
#include <map>

#include "file_io_registry.h"

#include "geometry/geometry_instance.h"
#include "geometry/editable_geometry_instance.h"

namespace Imagine
{

class Object;
class Mesh;
class Material;
class StandardMaterial;
class Matrix4;
class Scene;

struct GeoMaterials
{
	std::map<std::string, StandardMaterial*> materials;

	bool hasMaterialName(const std::string& mtlName)
	{
		return materials.count(mtlName) > 0;
	}
};

struct GeoReaderOptions
{
	enum GeoReaderMeshType
	{
		eEditableMesh,
		eStandardMesh,
		eTriangleMesh,
		ePointCloud
	};

	GeoReaderOptions() : importMaterials(true), importTextures(true), importCompoundObjects(true), newMaterialBreaksObjectGroup(false),
		rotate90NegX(false), centreObject(true), scaleToFit(true), scaleToFitSize(10.0f), standObjectOnPlane(true),
		setBBoxDrawMode(false),
		importFrame(1), useInstances(false),
		pointSize(0.001f), meshType(eEditableMesh)
	{
	}

	bool				importMaterials;
	bool				importTextures;
	std::string			customTextureSearchPath;
	bool				importCompoundObjects;
	bool				newMaterialBreaksObjectGroup;

	bool				rotate90NegX;

	bool				centreObject;
	bool				scaleToFit;
	float				scaleToFitSize;
	bool				standObjectOnPlane;

	bool				setBBoxDrawMode;

	unsigned int		importFrame;
	bool				useInstances;

	float				pointSize;

	GeoReaderMeshType	meshType;
};

struct GeoInstanceOverview
{
	GeoInstanceOverview() : count(0)
	{
	}

	unsigned int					count;
	std::vector<EditableGeometryInstance*>	aGeoInstances;
	std::vector<Object*>			aOwnerObjects;

	void add(EditableGeometryInstance* pGeoInstance, Object* pObject)
	{
		aGeoInstances.emplace_back(pGeoInstance);
		aOwnerObjects.emplace_back(pObject);
		count ++;
	}
};

class GeoReader
{
public:
	// constructor handles allocation of Mesh object
	GeoReader();
	virtual ~GeoReader();

	// if this fails, it will delete the Mesh object created in the constructor
	virtual bool readFile(const std::string& path, const GeoReaderOptions& options) = 0;

	void postProcess();

	void removeDuplicatePoints(Object* pObject);
	void discardDuplicateGeometry(unsigned short precisionThreshold);

	void applyMatrixToMesh(Matrix4& matrix, Mesh* pMesh, bool recalculateNormals);

	void setScene(Scene* pScene) { m_pScene = pScene; }

	Object* getNewObject() { return m_newObject; }
	std::vector<Material*>& getNewMaterials() { return m_aNewMaterials; }

	std::deque<Face>& getObjectFaces();
	std::deque<Point>& getObjectPoints();
	std::deque<Normal>& getObjectVertexNormals();
	std::deque<UV>& getObjectVertexUVs();

	std::deque<Point>& getSubObjectPoints(Mesh* subObject);
	std::deque<Face>& getSubObjectFaces(Mesh* subObject);
	std::deque<Normal>& getSubObjectVertexNormals(Mesh* subObject);
	std::deque<UV>& getSubObjectVertexUVs(Mesh* subObject);

protected:
	Object*					m_newObject;
	std::vector<Material*>	m_aNewMaterials;
	std::string				m_originalPath;

	Scene*					m_pScene;

	GeoReaderOptions		m_readOptions;
};

} // namespace Imagine

#endif // GEO_READER_H
