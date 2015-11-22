/*
 Imagine
 Copyright 2013 Peter Pearson.

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

#ifndef SCENE_READER_OBJ_H
#define SCENE_READER_OBJ_H

#include "io/scene_reader.h"

#include <set>

#include "io/geo_reader.h"

class SceneReaderObj : public SceneReader
{
public:
	SceneReaderObj();

	virtual bool readFile(const std::string& path, const SceneReaderOptions& options, SceneReaderResults& results);
};

#endif // SCENE_READER_OBJ_H
