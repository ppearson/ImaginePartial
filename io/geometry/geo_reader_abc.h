/*
 Imagine
 Copyright 2011-2013 Peter Pearson.

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

#ifndef GEO_READER_ABC_H
#define GEO_READER_ABC_H

#include "io/geo_reader.h"

#include <map>

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Util/All.h>

#include "core/hash.h"

namespace Imagine
{

using namespace Alembic::AbcGeom;

class GeoReaderAbc : public GeoReader
{
public:
	GeoReaderAbc();
	virtual ~GeoReaderAbc()
	{
	}

	virtual bool readFile(const std::string& path, const GeoReaderOptions& options);

	// bakes down point positions to the objects transforms - no instancing is done, so scenes can be heavy in
	// memory
	void processObjectsBaked(IObject& object, std::vector<Object*>& objects, chrono_t time);

	// GeoInstance points are stored in object space, and the objects themselves have transforms, allowing instancing
	void processObjectsInstances(IObject& object, std::vector<Object*>& objects, chrono_t time);

	// add the mesh vertex positions in final world space within mesh by transforming them
	void addTransformedPolyPoints(IPolyMeshSchema& meshSchema, IPolyMeshSchema::Sample& meshSample, Mesh* pMesh, chrono_t time);
	void addTransformedSubDPoints(ISubDSchema& subDSchema, ISubDSchema::Sample& subDSample, Mesh* pMesh, chrono_t time);

	// add the mesh vertex positions in original object space
	void addPolyPoints(IPolyMeshSchema& meshSchema, IPolyMeshSchema::Sample& meshSample, EditableGeometryInstance* pGeoInstace, chrono_t time);
	void addSubDPoints(ISubDSchema& subDSchema, ISubDSchema::Sample& subDSample, EditableGeometryInstance* pGeoInstace, chrono_t time);

	void getObjectTransform(IObject& object, chrono_t time, Vector& translate, Vector& rotation);

	void concatenateTransform(Imath::M44d& transform, IObject& object, chrono_t time);

	Imath::M44d getOverallTransform(IObject& object, chrono_t time);

	void getMatrixTransformComponents(const Imath::M44d& matrix, Vector& translate, Vector& rotation);

	static Vector quaternionToEuler(const Imath::Quatd& quat);

protected:
	typedef std::map<HashValue, EditableGeometryInstance*> GeoInstanceMap;

	GeoInstanceMap		m_aGeoInstances;
};

} // namespace Imagine

#endif // GEO_READER_ABC_H
