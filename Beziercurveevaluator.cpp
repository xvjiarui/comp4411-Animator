#include "BezierCurveEvaluator.h"
#include <assert.h>
#include "mat.h"
#include "vec.h"
#include "modelerapp.h"

#define SEGMENT 30

void BezierCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts, 
										 std::vector<Point>& ptvEvaluatedCurvePts, 
										 const float& fAniLength, 
										 const bool& bWrap) const
{
	ptvEvaluatedCurvePts.clear();

	// Wrapping version remark:
	// we insert one value at the end of the animation coordinate
	// case1: if this node can be part of the 4-node group (to generate curve)
	//		  the intersection y-value will determined by two close point in the curve
	// case2: then we will determine the intersection by using linear interpolation

	std::vector<Point> controlPointsCopy(ptvCtrlPts);
	if (bWrap)
	{
		controlPointsCopy.push_back(Point(ptvCtrlPts.front().x + fAniLength,
			ptvCtrlPts.front().y));
	}

	float x_start = 0.0;
	float y_start = ptvCtrlPts[0].y;

	size_t cnt;
	const Mat4d basis(
		-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0);

	bool is_wrapped = false;

	for (cnt = 0; cnt + 3 < controlPointsCopy.size(); cnt += 3)
	{
		// push starting and ending points first
		ptvEvaluatedCurvePts.push_back(controlPointsCopy[cnt]);
		ptvEvaluatedCurvePts.push_back(controlPointsCopy[cnt + 3]);
		const Vec4d px(controlPointsCopy[cnt].x, controlPointsCopy[cnt + 1].x,
			controlPointsCopy[cnt + 2].x, controlPointsCopy[cnt + 3].x);
		const Vec4d py(controlPointsCopy[cnt].y, controlPointsCopy[cnt + 1].y,
			controlPointsCopy[cnt + 2].y, controlPointsCopy[cnt + 3].y);
				
		for (int i = 0; i < SEGMENT; ++i)
		{
			const double t = i / (double)SEGMENT;
			const Vec4d param_time(t*t*t, t*t, t, 1);
			Point eval_point(param_time*basis*px, param_time*basis*py);

			if (eval_point.x > fAniLength && bWrap)
			{
				const float x_mod = fmod(eval_point.x, fAniLength);
				if (!is_wrapped)
				{
					const Point prev_point(ptvEvaluatedCurvePts.back());
					const float x_neg1 = prev_point.x - fAniLength;
					// push fake starting
					ptvEvaluatedCurvePts.push_back(Point(x_neg1, prev_point.y));
					// push fake ending, eval_point.x > fAniLength
					ptvEvaluatedCurvePts.push_back(eval_point);
					is_wrapped = true;
				}
				eval_point.x = x_mod;
			}
			ptvEvaluatedCurvePts.push_back(eval_point);
		}
	
	}

	for (; cnt < ptvCtrlPts.size(); cnt++)
	{
		ptvEvaluatedCurvePts.push_back(ptvCtrlPts[cnt]);
	}

	if (!bWrap)
	{
		ptvEvaluatedCurvePts.push_back(Point(0, ptvCtrlPts.front().y));
		ptvEvaluatedCurvePts.push_back(Point(fAniLength, ptvCtrlPts.back().y));
	}
	else if (!is_wrapped)
	{
		const float interval_length = ptvCtrlPts.front().x + fAniLength - ptvCtrlPts.back().x;
		const float percent = 1.0f - ptvCtrlPts.front().x / interval_length;
		const float y = ptvCtrlPts.back().y + (ptvCtrlPts.front().y - ptvCtrlPts.back().y) * percent;
		ptvEvaluatedCurvePts.push_back(Point(fAniLength, y));
		ptvEvaluatedCurvePts.push_back(Point(0.0f, y));
	}
}