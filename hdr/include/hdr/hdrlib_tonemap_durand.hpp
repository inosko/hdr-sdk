/*
 * hdrlib_tonemap_durand.hpp
 *
 *  Created on: 7. 11. 2017
 *      Author: nosko
 */

#pragma once


#include "hdrlib_tonemap_intf.hpp"

namespace hdr
{


namespace tmo
{


class TonemapDurand : public TonemapInterface
{
private:
	cv::Mat input;
	cv::Mat output;
	cv::Mat bmp;
	cv::Mat lumaOrig;
	cv::Mat lumaLog;
	cv::Mat baseLayer;
	float sigS;
	float sigR;
	float base;

public:
	TonemapDurand();

    virtual ~TonemapDurand(){};

	void prepare(float sigS, float sigR, float base);

	void apply(cv::Mat &im);

	void show();

	void writeImages(std::string path);

	cv::Mat getImage();

private:
	void getLuminance(cv::Mat &input, cv::Mat& output);
	void applyLog(cv::Mat &input, cv::Mat& output);
	void toneMapping(cv::Mat &intensitiesOrig,
						cv::Mat &intensitiesLog,
						cv::Mat &base,
						cv::Mat &colors,
						cv::Mat &dst,
						float &baseContrast);


	void findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max);
	void findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max, float percentil);

	void applyGamma(cv::Mat &input, float gamma);
};

}

}

