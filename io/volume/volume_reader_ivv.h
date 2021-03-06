/*
 Imagine
 Copyright 2014-2016 Peter Pearson.

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

#ifndef VOLUME_READER_IVV_H
#define VOLUME_READER_IVV_H

#include "io/volume_reader.h"

namespace Imagine
{

class VolumeReaderIVV : public VolumeReader
{
public:
	VolumeReaderIVV();

	virtual bool readHeaderAndBBox(const std::string& filePath, unsigned int flags, BoundaryBox& bbox);

	virtual VolumeInstance* readVolume(const std::string& filePath, unsigned int flags, BoundaryBox& bbox);

};

} // namespace Imagine

#endif // VOLUME_READER_IVV_H
