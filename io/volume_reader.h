/*
 Imagine
 Copyright 2015 Peter Pearson.

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

#ifndef VOLUME_READER_H
#define VOLUME_READER_H

#include "io/file_io_registry.h"

#include "core/boundary_box.h"

#include "volumes/volume_grid_dense.h"
#include "volumes/volume_instance.h"

namespace Imagine
{

class VolumeReader
{
public:
	VolumeReader();
	virtual ~VolumeReader();

	enum VolumeReadFlags
	{
		VOLUME_DATA_DENSITY			= 1 << 0,
		VOLUME_DATA_TEMPERATURE		= 1 << 1,
		VOLUME_DATA_COLOUR			= 1 << 2,

		VOLUME_TYPE_FLOAT			= 1 << 6,
		VOLUME_TYPE_HALF			= 1 << 7
	};

	virtual bool readHeaderAndBBox(const std::string& filePath, unsigned int flags, BoundaryBox& bbox) = 0;

	virtual VolumeInstance* readVolume(const std::string& filePath, unsigned int flags, BoundaryBox& bbox) = 0;
};

} // namespace Imagine

#endif // VOLUME_READER_H
