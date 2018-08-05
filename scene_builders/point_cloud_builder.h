#ifndef POINT_CLOUD_BUILDER_H
#define POINT_CLOUD_BUILDER_H

#include "scene_builder.h"

namespace Imagine
{

class PointCloudBuilder : public SceneBuilder
{
public:
	PointCloudBuilder();

	virtual unsigned char getSceneBuilderTypeID();
	virtual std::string getSceneBuilderDescription();

	virtual void buildParameters(Parameters& parameters, unsigned int flags);

	virtual void createScene(Scene& scene);

protected:
	unsigned int	m_distribution;
	unsigned int	m_boundShape;
	
	unsigned int	m_numberOfPoints;
	
	float			m_radius;
};

} // namespace Imagine

#endif // POINT_CLOUD_BUILDER_H
