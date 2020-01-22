/*
 Imagine
 Copyright 2011-2014 Peter Pearson.

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

#ifndef FLOAT_SLIDER_WIDGET_H
#define FLOAT_SLIDER_WIDGET_H

#include <QWidget>

class QSlider;

namespace Imagine
{

class DoubleSpinBoxEx;

class FloatSliderWidget : public QWidget
{
	Q_OBJECT
public:
	FloatSliderWidget(float min, float max, bool editControl, bool logScaleSlider, bool highPrecision, QWidget* parent = nullptr);
	virtual ~FloatSliderWidget();

	void setValue(float value);

	float getValue() const;

signals:
	void sliderMoved(int);
	void editingFinished();
	void valueChanged();

public slots:
	void spinChanged();
	void sliderChanged(int position);
	void sliderActionTriggered(int actionTriggered);

protected:
	float getLinearValue(float x) const;
	float getExpValue(float x) const;

protected:
	DoubleSpinBoxEx*	m_pDoubleSpin;
	QSlider*			m_pSlider;

	int					m_min;
	int					m_max;

	bool				m_logScale;
	float				m_offset; // offset to cope with min = 0
	float				m_scale;
	float				m_fMin;
	float				m_fMax;
	float				m_logMin;
	float				m_logMax;

	// used to get floating point value from an int Widget
	double				m_converter;
};

} // namespace Imagine

#endif // FLOAT_SLIDER_WIDGET_H
