/* 
 * File:   stasm_camera.cpp
 * Author: algunenano
 *
 * Created on 23 de octubre de 2015, 13:10
 * Based on stasm.main.cpp (Copyright (C) 2005-2013, Stephen Milborrow)
 */

#include <cstdlib>
#include <memory>
#include <opencv2/highgui/highgui.hpp>

#include "stasm.h"
#include "appmisc.h"

using namespace std;
using namespace cv;
using namespace stasm;


void myDrawShape(           // draw a shape on an image
    Mat&         img,       // io
    const Shape& shape,     // in
    unsigned     color,     // in: rrggbb, default is 0xff0000 (red)
    bool         dots,      // in: true for dots only, default is false
    int          linewidth) // in: default -1 means automatic
{
    const double width = ShapeWidth(shape);
    if (linewidth <= 0)
        linewidth = width > 700? 3: width > 300? 2: 1;
    CvScalar cvcolor(ToCvColor(color));
    int i = 0, j=0;
    do  // use do and not for loop because some points may be unused
    {
        while (i < shape.rows && !PointUsed(shape, i)) // skip unused points
            i++;
        if (i < shape.rows)
        {
            if (dots)
            {
//                const int ix = cvRound(shape(i, IX)), iy = cvRound(shape(i, IY));
//                if (ix >= 0 && ix < img.cols && iy >= 0 && iy < img.rows)
//                {
//                    img.(iy, ix)[0] = (color >>  0) & 0xff;
//                    img(iy, ix)[1] = (color >>  8) & 0xff;
//                    img(iy, ix)[2] = (color >> 16) & 0xff;
//                }
            }
            else // lines
            {
                j = i+1;
                while (j < shape.rows && !PointUsed(shape, j))
                    j++;
                if (j < shape.rows)
                    cv::line(img,
                             cv::Point(cvRound(shape(i, IX)), cvRound(shape(i, IY))),
                             cv::Point(cvRound(shape(j, IX)), cvRound(shape(j, IY))),
                             cvcolor, linewidth);
            }
        }
        i++;
    }
    while (i < shape.rows && j < shape.rows);
}

void myProcessFace (
    Mat&         _dest, //io 
    const float* landmarks, // in
    int          nfaces    // in
        )
{
    Shape shape(LandmarksAsShape(landmarks));
    Shape converted_shape(ConvertShape(shape, stasm_NLANDMARKS));
    if (converted_shape.rows == 0)
    {
        static int printed;
        PrintOnce(printed, "\nCannot convert a %d point shape to a %d point shape\n",
                  shape.rows, stasm_NLANDMARKS);
    }
    else
        shape = converted_shape;
    
    myDrawShape(_dest, shape, 0xff0000, false, -1);
    
}


void myProcessImage (CImage &_source, Mat& _dest)
{
    cvtColor(_source, _dest, CV_BGR2GRAY);
    
    if (!stasm_open_image(
            (const char*)_dest.data,
            _dest.cols,
            _dest.rows,
            "Camera", //used for err msgs and debug
            0, //ALLOW MULTIPLES FACES
            25 //MINIMUN WIDTH %
            ))
        Err("stasm_open_image failed:  %s", stasm_lasterr());
    
    

    for (;;)
    {
        float landmarks [2 * stasm_NLANDMARKS];
        
        int foundface;
        if (!stasm_search_auto(&foundface, landmarks))
            Err("stasm_search_auto failed: %s", stasm_lasterr());

        if (!foundface)
            break; // note break
        stasm_printf("Face found\n");
        
        myProcessFace(_dest, landmarks, 1);
        
    }
    
    

}


void main1(int argc, const char** argv)
{
    if (!stasm_init("../data", 1 /*trace*/))
        Err("stasm_init failed %s", stasm_lasterr());
    
    VideoCapture camera(0);
    if (!camera.isOpened())
    {
        stasm::Err("Could not open the camera!");
    }
    
    const char* const camName = "Test";
    namedWindow(camName,CV_WINDOW_OPENGL);
    
    CImage frame;
    Mat dest;
    camera >> frame;
    
    for(;;)
    {
        camera >> frame; // get a new frame from camera
        
        myProcessImage(frame, dest);
                
        imshow(camName, dest);
        
        if(waitKey(30) >= 0) break;
    }
    
    
    destroyWindow(camName);
    camera.release();
    
}



/* 
 * This application calls Stasm's internal routines.  Thus we need to catch a
 * potential throw from Stasm's error handlers.  Hence the try/catch code below.
 */


int main (int argc, const char** argv)
{
    stasm::CatchOpenCvErrs();
    try
    {
        main1(argc, argv);
    }
    catch(...)
    {
        // a call was made to Err or a CV_Assert failed
        printf("\n%s\n", stasm_lasterr());
        exit(1);
    }
    return 0;       // success
}

