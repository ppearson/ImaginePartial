/*
 Imagine
 Copyright 2016 Peter Pearson.

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

#ifndef VARIANCE_TRACKER_H
#define VARIANCE_TRACKER_H

#include <cmath>

#include "colour/colour4f.h"

namespace Imagine
{

// Variance tracker class, based on B. P. Welford paper and Donald Knuth's implementation

class VarianceTracker
{
public:
	VarianceTracker() : m_sampleCount(0), m_lastMean(0.0f), m_lastMeanSquare(0.0f),
						m_currentMean(0.0f), m_currentMeanSquare(0.0f)
	{

	}

	void addValue(float value)
	{
		if (m_sampleCount == 0)
		{
			m_sampleCount = 1;
			m_currentMean = value;
			m_lastMean = value;
		}
		else
		{
			m_sampleCount++;

			m_currentMean = m_lastMean + (value - m_lastMean) / (float)m_sampleCount;
			m_currentMeanSquare = m_lastMeanSquare + (value - m_lastMean) * (value - m_currentMean);

			m_lastMean = m_currentMean;
			m_lastMeanSquare = m_currentMeanSquare;
		}
	}

	float getMean() const
	{
		return m_currentMean;
	}

	float getVariance() const
	{
		if (m_sampleCount <= 1)
			return 0.0f;

		return m_currentMeanSquare / (float)(m_sampleCount - 1);
	}

	float getStandardVariation() const
	{
		return std::sqrt(getVariance());
	}

protected:
	unsigned int		m_sampleCount;

	float				m_lastMean;
	float				m_lastMeanSquare;

	float				m_currentMean;
	float				m_currentMeanSquare;
};


class VarianceTrackerColour
{
public:
	VarianceTrackerColour() : m_sampleCount(0)
	{

	}

	void addValue(const Colour4f& value)
	{
		m_total += value;
		if (m_sampleCount == 0)
		{
			m_sampleCount = 1;
			m_currentMean = value;
			m_lastMean = value;
		}
		else
		{
			m_sampleCount++;

			m_currentMean = m_lastMean + (value - m_lastMean) / (float)m_sampleCount;
			m_currentMeanSquare = m_lastMeanSquare + (value - m_lastMean) * (value - m_currentMean);

			m_lastMean = m_currentMean;
			m_lastMeanSquare = m_currentMeanSquare;
		}
	}

	float getVariance() const
	{
		if (m_sampleCount <= 1)
			return 0.0f;

		Colour4f variance = m_currentMeanSquare / (float)(m_sampleCount - 1);
		return variance.max();
	}

	float getStandardVariation() const
	{
		return std::sqrt(getVariance());
	}

	float getWeightedStandardVariation() const
	{
		float weight = (float)m_sampleCount;
		Colour4f weightedColour = m_total / weight;

		return std::sqrt(getVariance()) / weightedColour.brightness();
	}


protected:
	unsigned int		m_sampleCount;

	Colour4f			m_lastMean;
	Colour4f			m_lastMeanSquare;

	Colour4f			m_currentMean;
	Colour4f			m_currentMeanSquare;

	Colour4f			m_total;
};

} // namespace Imagine

#endif // VARIANCE_TRACKER_H

