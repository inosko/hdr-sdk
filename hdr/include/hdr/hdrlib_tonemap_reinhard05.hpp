/*
 * hdrlib_tonemap_durand.hpp
 *
 *  Created on: 7. 11. 2017
 *      Author: nosko
 */

#pragma once


#include "hdrlib_tonemap_intf.hpp"

namespace hdr::tmo
{


class TonemapReinhard : public TonemapInterface
{


public:
	TonemapReinhard();

    virtual ~TonemapReinhard(){};



	void apply(cv::Mat &im);

	void show();

	void writeImages(std::string path);

	cv::Mat getImage();

private:
	cv::Mat output;
	cv::Mat bmp;
	float f;
	float a;
	float c;
};


}

