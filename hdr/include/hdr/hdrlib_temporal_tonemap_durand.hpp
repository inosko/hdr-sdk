/*
 * hdrlib_temporal_tonemap_durand.hpp
 *
 *  Created on: 7. 11. 2017
 *      Author: nosko
 */

#pragma once


#include <fstream>
#include "hdrlib_tonemap_intf.hpp"

namespace hdr::tmo
{


class TemporalTonemapDurand : public TonemapInterface
{

public:
	TemporalTonemapDurand(double d, double baseContrast);

    virtual ~TemporalTonemapDurand(){};

	void prepare(float sigS, float sigR, float base);

	void apply(cv::Mat &im);

	void show();

	void writeImages(std::string path);

	cv::Mat getImage();

	float getMin();

private:
	void getLuminance(cv::Mat &input, cv::Mat& output);
	void applyLog(cv::Mat &input, cv::Mat& output);
	void toneMapping(cv::Mat &intensitiesOrig,
						cv::Mat &intensitiesLog,
						cv::Mat &base,
						cv::Mat &colors,
						cv::Mat &dst,
						cv::Mat &dstMap,
						float &baseContrast);


	void findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max);
	void findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max, float percentil);

	static void applyGamma(cv::Mat &input, cv::Mat &map, float gammaR , float gammaG , float gammaB);
	static void applyGammaB(cv::Mat &input, cv::Mat &map,  float gamma);


private:
	cv::Mat input;
	cv::Mat output;
	cv::Mat bmp;
	cv::Mat lumaOrig;
	cv::Mat lumaLog;
	cv::Mat baseLayer;
	cv::Mat baseLayerG;
	float sigS;
	float sigR;
	float base;
	float temporalAdaptation;
	int processed;

	std::vector<cv::Vec3f> mins;
	std::vector<cv::Vec3f> maxs;
	std::vector<cv::Vec3f> minsMap;
	std::vector<cv::Vec3f> maxsMap;
};

}
