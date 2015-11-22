/*
 Imagine
 Copyright 2012-2013 Peter Pearson.

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

#ifndef SCENE_READER_ABC_H
#define SCENE_READER_ABC_H

#include "io/scene_reader.h"

#include <map>

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Util/All.h>

#include "core/hash.h"

using namespace Alembic::AbcGeom;

class GeometryInstanceGathered;
class EditableGeometryInstance;
class Vector;

class SceneReaderAbc : public SceneReader
{
public:
	SceneReaderAbc();

	virtual bool readFile(const std::string& path, const SceneReaderOptions& options, SceneReaderResults& results);

	// GeoInstance points are stored in object space, and the objects themselves have transforms, allowing instancing
	void processObjectsInstances(IObject& object, std::vector<Object*>& objects, std::vector<Material*>& materials, chrono_t time, unsigned int currentDepth);

	// add the mesh vertex positions in original object space
	void addPolyPoints(IPolyMeshSchema& meshSchema, IPolyMeshSchema::Sample& meshSample, EditableGeometryInstance* pGeoInstance, chrono_t time);
	void addSubDPoints(ISubDSchema& subDSchema, ISubDSchema::Sample& subDSample, EditableGeometryInstance* pGeoInstance, chrono_t time);

	void addFacesAndUVs(P3fArraySamplePtr pPoints, Int32ArraySamplePtr pFaceIndices, Int32ArraySamplePtr pFaceCounts, IV2fGeomParam uvParams,
						EditableGeometryInstance* pGeoInstance);

	void addFacesAndUVsWithFacesets(P3fArraySamplePtr pPoints, Int32ArraySamplePtr pFaceIndices, Int32ArraySamplePtr pFaceCounts, IV2fGeomParam uvParams,
						Int32ArraySamplePtr pFaceSetIndices, EditableGeometryInstance* pGeoInstance);

	void getObjectTransform(IObject& object, chrono_t time, Vector& translate, Vector& rotation, Vector& scale);

	void concatenateTransform(Imath::M44d& transform, IObject& object, chrono_t time);

	Imath::M44d getOverallTransform(IObject& object, chrono_t time);

	void getMatrixTransformComponents(const Imath::M44d& matrix, Vector& translate, Vector& rotation, Vector& scale);

	static Vector quaternionToEuler(const Imath::Quatd& quat);


protected:
	typedef std::map<HashValue, GeometryInstanceGathered*> GeoInstanceMap;

	GeoInstanceMap		m_aGeoInstances;
	SceneReaderOptions	m_options;
};

#endif // SCENE_READER_ABC_H
