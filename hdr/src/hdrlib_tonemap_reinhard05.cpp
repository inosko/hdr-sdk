/*
 * hdrlib_tonemap_durnad.cpp
 *
 *  Created on: 7. 11. 2017
 *      Author: nosko
 */



#include <iomanip>
#include <hdr/hdrlib_tonemap_reinhard05.hpp>


static const float TMO_RGB2Y[3] =
{
	0.21263903f,
	0.71516865f,
	0.07219231f
};

static float rgbfToY( cv::Vec3f pix )
{
	return pix[ 2 ] * TMO_RGB2Y[ 0 ] + pix[ 1 ] * TMO_RGB2Y[ 1 ] + pix[ 0 ] * TMO_RGB2Y[ 2 ];
}

hdr::tmo::TonemapReinhard::TonemapReinhard()
{
	f = 9.0f,  a = 0.75f, c = 0.75f;
}

void hdr::tmo::TonemapReinhard::apply(cv::Mat &im)
{
	float max = -1e10f;
	float min = 1e10f;
	float sum = 0.0f;
	float logSum = 0.0f;
	float sumB = 0.0f;
	float sumG = 0.0f;
	float sumR = 0.0f;

	assert( im.data != NULL );
	assert( im.depth( ) == CV_32F );

	assert( im.channels( ) == 3 );



	output = cv::Mat( im.rows, im.cols, CV_32FC3 );
	cv::Mat imageTmp( im.rows, im.cols, CV_32FC1 );

	/* convert */
	for ( int32_t i = 0; i < im.rows; i++ )
	{
		for ( int32_t j = 0; j < im.cols; j++ )
		{
			cv::Vec3f pix;
			pix = im.at< cv::Vec3f >( i, j );


			float Y = rgbfToY( pix );
			imageTmp.at< float >( i, j )= Y;

			sumR += pix[ 2 ];
			sumG += pix[ 1 ];
			sumB += pix[ 0 ];

			logSum += logf(2.3e-5F + Y );
			sum += Y;
			max = MAX( max, Y );
			min = MIN( min, Y );

		}
	}

	uint32_t imSize = im.rows * im.cols;
	float luminanceAvg =  ( sum / imSize );
	float luminanceLogAvg =  expf( logSum / imSize  );

	cv::Vec3f chanAvgs;
	chanAvgs[ 2 ] = sumR / imSize;
	chanAvgs[ 1 ] = sumG / imSize;
	chanAvgs[ 0 ] = sumB / imSize;

	float lumLogMax = logf( max );
	float lumLogMin = logf( min );


	/* compute image key */
	float imageKey = ( lumLogMax - luminanceLogAvg ) / ( lumLogMax - lumLogMin );
	float m = 0.0;
	if ( imageKey < 0.0f )
	{
		imageKey = ( lumLogMax - logf( luminanceLogAvg ) ) / ( lumLogMax - lumLogMin );
		if( imageKey < 0.0f)
			m = 0.3f;
	}

	m = (m > 0) ? m :  ( 0.3 + 0.7 * powf( imageKey ,1.4F ) );

	float colorMin =  1e10;
	float colorMax = -1e10;
	f = expf( -f );

	std::cout << "Luminance  Max:" << std::setprecision( 10 ) << max << std::endl;
	std::cout << "Luminance  Min:" << std::setprecision( 10 ) << min << std::endl;
	std::cout << "Luminance  Avg:" << std::setprecision( 10 ) << luminanceAvg << std::endl;
	std::cout << "Lum. Log.  Avg:" << std::setprecision( 10 ) << luminanceLogAvg << std::endl;
	std::cout << "Chan Blue  Avg:" << std::setprecision( 10 ) << chanAvgs[ 0 ] << std::endl;
	std::cout << "Chan Green Avg:" << std::setprecision( 10 ) << chanAvgs[ 1 ] << std::endl;
	std::cout << "Chan Red   Avg:" << std::setprecision( 10 ) << chanAvgs[ 2 ] << std::endl;
	std::cout << "Image Key     :" << std::setprecision( 10 ) << imageKey << std::endl;
	std::cout << "m             :" << std::setprecision( 10 ) << m        << std::endl;

	/* tone map */
	for ( int32_t i = 0; i < im.rows; i++ )
	{
		for ( int32_t j = 0; j < im.cols; j++ )
		{

			cv::Vec3f pix;
			pix = im.at< cv::Vec3f >( i, j );

			for( int32_t k = 0; k < 3; k++ )
			{
				float lightLocal = c * pix[ k ] + ( 1 - c ) * luminanceAvg;
				float lightGlobal = c * chanAvgs[ k ] + ( 1 - c ) * luminanceLogAvg;
				float lightInterpol = a * lightLocal + ( 1 - a ) * lightGlobal;
				float coef = pow( lightInterpol * f,  m );
				pix[ k ] = pix[ k ]  / ( pix[ k ]  + coef );

				colorMax = MAX( colorMax, pix[ k ] );
				colorMin = MIN( colorMin, pix[ k ] );
			}

			output.at< cv::Vec3f >( i, j ) = pix;

		}
	}


	const float range = colorMax -  colorMin;

	/* normalize */
	for ( int32_t i = 0; i < im.rows; i++ )
	{
		for ( int32_t j = 0; j < im.cols; j++ )
		{
			cv::Vec3f pix;
			pix = output.at< cv::Vec3f >( i, j );
			for( int32_t k = 0; k < 3; k++ )
			{

				pix[ k ] = ( pix[ k ] - colorMin ) / range;
			}
			output.at< cv::Vec3f >( i, j ) = pix;
		}
	}

	bmp = cv::Mat(this->output.size(), CV_8UC3);


	for (int32_t i = 0; i < this->output.rows; ++i)
	{
		for (int32_t j = 0; j < this->output.cols; ++j)
		{
			cv::Vec3f pix = this->output.at<cv::Vec3f>(i, j);

			bmp.at<cv::Vec3b>(i, j)[0] = pix[0] * 255 > 255 ? 255 : pix[0] * 255;
			bmp.at<cv::Vec3b>(i, j)[1] = pix[1] * 255 > 255 ? 255 : pix[1] * 255;
			bmp.at<cv::Vec3b>(i, j)[2] = pix[2] * 255 > 255 ? 255 : pix[2] * 255;
		}
	}


}

void hdr::tmo::TonemapReinhard::show()
{

}

void hdr::tmo::TonemapReinhard::writeImages(std::string path)
{

	cv::imwrite(path, bmp);
	//cv::imwrite(path + "durand.exr", output);
}

cv::Mat hdr::tmo::TonemapReinhard::getImage()
{
	return bmp;
}




