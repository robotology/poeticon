#ifndef ICUB_BLOB_DESC_SUPPORT_H
#define ICUB_BLOB_DESC_SUPPORT_H

/* helper classes and functions - most are taken from Ivana Cingovska's 2008 work */

#include <cmath>
#include <cstdio>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iCub/BlobDescriptorModule.h>

/* objectDescriptor.h */

// TODO: create a class with proper constructors and destructors
class ObjectDescriptor
{
public: //everything public for a start
    ObjectDescriptor();
    bool Create(int width, int height);
    ~ObjectDescriptor();
    int no; // number of objects in the image ??? It looks like it is more the index of the object in the array
    int label;
    int area;
    CvPoint center;

    bool valid;
    //Color Histogram
    CvHistogram *objHist;
    //New fields added for color histogram computations - Alex 19/12/2009
    int _w;
    int _h;
    CvSize _sz;
    int _hist_size[2];
    float _h_ranges[2], _s_ranges[2], _v_ranges[2];
    int	h_bins;
    int	s_bins;
    int v_bins;

    IplImage* mask_image; // object mask
    unsigned char *mask_data;

    CvScalar color;

    //New fields added for tracker initialization - Alex 20/12/2009
    int roi_x;
    int roi_y;
    int roi_width;
    int roi_height;
    int v_min;
    int v_max;
    int s_min;
    int s_max;

    //New fields added for contour processing - Alex 13/12/2009
    CvMemStorage *storage; // = cvCreateMemStorage(0);
    CvSeq *contours; // raw blob contours
    CvSeq *affcontours; // processed contours for the description of affordances
    CvSeq *convexhull;

    // new fields added for moments and centre of mass - Giovanni 2014
    cv::Moments moments;

    /* variables to define the shape */
    double contour_area;
    double contour_perimeter;
    double convex_perimeter;
    double major_axis;
    double minor_axis;
    double rect_area;
    CvBox2D enclosing_rect;
    CvRect bounding_rect;

    /* shape descriptors for the affordances */
    double convexity;
    double eccentricity;
    double compactness;
    double circleness;
    double squareness;
    /* for POETICON++ */
    double elongatedness;
    double length;
};

/* calcdistances.h */
class Distance
{
private:
    float bhattacharyya(CvHistogram *hist1, CvHistogram *hist2, int h_bins, int s_bins);
    double euclidian(CvPoint c1, CvPoint c2);
    double weightB, weightE, weightA; // weight coefficients of the different distances that contribute to the overall distance

public:
    Distance(double wB = 1, double wE = 1, double wA = 1);
    double overallDistance(ObjectDescriptor *d1, ObjectDescriptor *d2);
};

/* selectObjects.h */
int selectObjects(IplImage *labeledImage, IplImage *out, int numLabels, int areaThres);
void whiteBalance(IplImage *maskImage, IplImage *originalImage, IplImage *whiteBalancedImg);
int indexOfL(int label, ObjectDescriptor *odTable, int length);
void extractObj(IplImage *labeledImage, int numObjects, ObjectDescriptor *objDescTable);
void int32ToInt8Image(IplImage *labeledIntImage, IplImage *labeledImage);
bool existsInList(int x, int a[], int numEl);
void printImgLabels(IplImage *img, int numObjects);

/* for POETICON++ */
void drawBox( CvArr* img, CvBox2D box, CvScalar color );
void drawBoxContour( CvArr* img, CvBox2D box, CvScalar color, int thickness );

#endif // ICUB_BLOB_DESC_SUPPORT_H
