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

#ifndef PREVIEW_RENDERER_H
#define PREVIEW_RENDERER_H

#include "full_renderer.h"

namespace Imagine
{

//! preview progressive renderer

template <typename Integrator, typename Accumulator, typename TimeCounter>
class PreviewRenderer : public FullRenderer<Integrator, Accumulator, TimeCounter>
{
public:
	PreviewRenderer(Raytracer& rt, const Params& settings, const Accumulator& acc, unsigned int threads)
		: FullRenderer<Integrator, Accumulator, TimeCounter>(rt, settings, acc, threads)
	{
	}

	virtual ~PreviewRenderer()
	{
	}

	virtual bool processTask(RenderTask* pRTask, unsigned int threadID);

protected:

};

} // namespace Imagine

#endif // PREVIEW_RENDERER_H
