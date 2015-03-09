/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:   vadim.tikhanoff@iit.it
 * website: www.robotcub.org
 * Algorithm taken from R. Hess, A. Fern, "Discriminatively trained particle filters for complex multi-object tracking," 
 * cvpr, pp.240-247, 2009 IEEE Conference on Computer Vision and Pattern Recognition, 2009
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */


#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/PortReport.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/LockGuard.h>
#include <yarp/sig/Vector.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Event.h>
#include "iCub/targetContainer.h"
#include "iCub/activeSeg.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <cv.h>
#include <highgui.h>

/* default number of particles */
#define PARTICLES 750
/* number of bins of HSV in histogram */
#define NH 10
#define NS 10
#define NV 10
/* low thresholds on saturation and value for histogramming */
#define S_THRESH 0.1
#define V_THRESH 0.2
/* max HSV values */
#define H_MAX 360.0
#define S_MAX 1.0
#define V_MAX 1.0
/* standard deviations for gaussian sampling in transition model */
#define TRANS_X_STD 10.0f    //12      best 10
#define TRANS_Y_STD 7.5f     //5.5     best 7.5
#define TRANS_S_STD 0.001f   //0.0005f best 0.001
/* autoregressive dynamics parameters for transition model */
#define pfot_A1  2.0f
#define pfot_A2 -1.0f
#define pfot_B0  1.0000f
/* distribution parameter */
#define LAMBDA 15            //25 best 15

/* templates list parameters */
#define TEMP_LIST_SIZE                  10
#define TEMP_LIST_PARTICLE_THRES_HIGH   0.9
#define TEMP_LIST_PARTICLE_THRES_LOW    0.5

typedef struct TemplateStruct
{
    float                                       w;
    yarp::sig::ImageOf<yarp::sig::PixelRgb>     *templ;
} TemplateStruct;

int particle_cmp( const void* p1, const void* p2 );

class TRACKERManager;  //forward declaration

/**********************************************************/
class FixationPoint : public yarp::os::BufferedPort<yarp::os::Bottle>
{
protected:
    TRACKERManager *manager;
    void onRead(yarp::os::Bottle &b);
public:
    FixationPoint();
    void setManager(TRACKERManager *manager);
};

/**********************************************************/
class ParticleThread : public yarp::os::RateThread
{
public:

    /* class methods */
    ParticleThread( unsigned int id, yarp::os::ResourceFinder &rf, SegInfo info, int group=-1, CvScalar *colour=NULL);
    ~ParticleThread();

    typedef struct histogram
    {
        float histo[NH*NS + NV];  /* histogram array */
        int n;                    /* length of histogram array */
    } histogram;

    typedef struct particle
    {
        float x;                  /* current x coordinate */
        float y;                  /* current y coordinate */
        float s;                  /* scale */
        float xp;                 /* previous x coordinate */
        float yp;                 /* previous y coordinate */
        float sp;                 /* previous scale */
        float x0;                 /* original x coordinate */
        float y0;                 /* original y coordinate */
        int width;                /* original width of region described by particle */
        int height;               /* original height of region described by particle */
        histogram* histo;         /* reference histogram describing region being tracked */
        float w;                  /* weight */
    } particle;

    bool threadInit();
    void threadRelease();
    void run();
    void update(IplImage * img );
    void onStop();
    yarp::sig::ImageOf<yarp::sig::PixelRgb> getTemplate ();
    void isInitialized();

    TemplateStruct getBestTemplate();
    yarp::os::Event event;

    particle                *particles;

private:

    SegInfo                 info;
    int                     group;
    CvScalar                colour;
    IplImage                *image;
    bool                    shouldStop;
    unsigned int            id;
    TargetObjectRecord      &container;
    ActiveSeg               activeSeg;
    TargetObject            *object;
    
    yarp::os::Mutex         mutexThread;

    int                     num_objects;
    int                     num_particles;
    CvRect                  **regions;
    histogram               **ref_histos;
    yarp::os::Semaphore     mutex;
    
    bool                    doneUpdating;

    gsl_rng                 *rng;

    particle                *sort_particles;

    IplImage* bgr2hsv( IplImage* bgr );
    void free_regions( CvRect** regions, int n );
    void free_histos( histogram** histo, int n );
    int get_regionsImage( IplImage* frame, CvRect** regions );
    histogram** compute_ref_histos( IplImage* img, CvRect* rect, int n );
    histogram* calc_histogram( IplImage** imgs, int n );
    void normalize_histogram( histogram* histo );
    int histo_bin( float h, float s, float v );
    float pixval32f( IplImage* img, int r, int c );
    particle* init_distribution( CvRect* regions, histogram** histos, int n, int p );
    particle transition( const particle &p, int w, int h, gsl_rng* rng );
    float likelihood( IplImage* img, int r, int c, int w, int h, histogram* ref_histo );
    float histo_dist_sq( histogram* h1, histogram* h2 );
    void normalize_weights( particle* particles, int n );
    particle* resample( particle* particles, int n );
    void retreive_particle( particle* particles );
};
#endif
//empty line to make gcc happy
