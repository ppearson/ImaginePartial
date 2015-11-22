/*
 Imagine
 Copyright 2012 Peter Pearson.

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

#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

enum ImageComponentFlags
{
	COMPONENT_RGBA				= 1 << 0,
	COMPONENT_DEPTH_NORMALISED	= 1 << 1,
	COMPONENT_DEPTH_REAL		= 1 << 2,
	COMPONENT_NORMAL			= 1 << 3,
	COMPONENT_WPP				= 1 << 4,
	COMPONENT_SHADOWS			= 1 << 5,

	COMPONENT_SAMPLES			= 1 << 6,

	COMPONENT_DEEP				= 1 << 7,

	COMPONENT_DEPTH				= COMPONENT_DEPTH_NORMALISED | COMPONENT_DEPTH_REAL
};

#endif // IMAGE_COMMON_H
