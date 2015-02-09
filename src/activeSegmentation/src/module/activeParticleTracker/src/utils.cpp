/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:  vadim.tikhanoff@iit.it
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

#include <yarp/os/Time.h>

#include "iCub/utils.h"
#include "iCub/activeParticle.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

/**********************************************************/
FixationPoint::FixationPoint()
{
    manager=NULL;
    useCallback();
}

/**********************************************************/
void FixationPoint::setManager(TRACKERManager *manager)
{
    this->manager=manager;
}

/**********************************************************/
void FixationPoint::onRead(Bottle &target)
{
    if (target.size()>1)
        manager->processFixationPoint(target);
}

/**********************************************************/
ParticleThread::ParticleThread ( unsigned int id, ResourceFinder &rf, SegInfo info, int group, CvScalar *colour) : container(TargetObjectRecord::getInstance())
{
    
    mutexThread.lock();
    if (colour)
        ParticleThread::colour = *colour;
    else
    {
        int R = rand() % 255;
        int G = rand() % 255;
        int B = rand() % 255;
        ParticleThread::colour = CV_RGB(R,G,B);
    }
    ParticleThread::id = id;
    ParticleThread::info = info;
    ParticleThread::group = group;
    regions = (CvRect **) malloc(sizeof(CvRect*));
    ref_histos = NULL;
    particles = NULL;
    sort_particles = NULL;
    rng = gsl_rng_alloc( gsl_rng_mt19937 );
    gsl_rng_set( rng, (unsigned long)time(NULL) );
    activeSeg.configure(rf);
}

/**********************************************************/
ParticleThread::~ParticleThread()
{
    if ( object )
    {
        container.lock();
        //if ( object->tpl != NULL )
        //    cvReleaseImage(&object->tpl);

        //if ( object->seg != NULL  )
         //   cvReleaseImage(&object->seg);

        container.remove(id);
        container.unlock();

        delete object;

        if(regions != NULL)
            free_regions ( regions, num_objects );

        if (ref_histos != NULL)
            free_histos ( ref_histos, num_objects );

        if(particles != NULL)
            free ( particles );

        gsl_rng_free ( rng );
    }
}

/**********************************************************/
bool ParticleThread::threadInit()
{
    object = NULL;
    shouldStop = false;
    num_objects = 0;
    image = NULL;
    num_particles = 1000;
    return true;
}

/**********************************************************/
void ParticleThread::isInitialized()
{
    mutexThread.lock();
    mutexThread.unlock();
}

/**********************************************************/
void ParticleThread::run()
{
    while (isStopping() != true)
    {
        event.wait();
        mutex.wait();
        if (shouldStop)
        {
            mutex.post();
            return;
        }
        IplImage *img_hsv = bgr2hsv( image );

        if (object==NULL)
        {
            object = new TargetObject(id);
            object->group = group;
            object->colour = ParticleThread::colour;

            activeSeg.getSegWithFixation(image, object->seg, info);
            
            activeSeg.getTemplateFromSeg(image, object->seg, object->tpl, info);
            
            container.lock();
            container.add(object);
            container.unlock();

            if (num_objects>0)
                free(*regions);
            num_objects = 0;
            cvCvtColor(image, image, CV_BGR2RGB);
            num_objects = get_regionsImage( image, regions );

            image = bgr2hsv( image );
            if (ref_histos!=NULL)
                free_histos ( ref_histos, num_objects);

            ref_histos = compute_ref_histos( img_hsv, *regions, num_objects );

            if (particles != NULL)
                free (particles);

            particles= init_distribution( *regions, ref_histos, num_objects, num_particles );
            
            mutexThread.unlock();
        }
        else
        {
            // perform prediction and measurement for each particle
            for( int j = 0; j < num_particles; j++ )
            {
                particles[j] = transition( particles[j], image->width, image->height, rng );
                float s = particles[j].s;
                particles[j].w = likelihood( img_hsv, cvRound(particles[j].y),
                    cvRound( particles[j].x ),
                    cvRound( particles[j].width * s ),
                    cvRound( particles[j].height * s ),
                    particles[j].histo );
            }
            // normalize weights and resample a set of unweighted particles
            normalize_weights( particles, num_particles );
            sort_particles = resample( particles, num_particles );
            free( particles );
            particles = sort_particles;
        }
        qsort( particles, num_particles, sizeof( ParticleThread::particle ), &particle_cmp );

        retreive_particle( particles );

        cvReleaseImage(&img_hsv);
        cvReleaseImage(&image);
        mutex.post();
    }
}

/**********************************************************/
void ParticleThread::onStop()
{
    shouldStop = true;
    mutexThread.unlock();
    event.signal();
}

/**********************************************************/
void ParticleThread::threadRelease()
{
    if (image)
        cvReleaseImage(&image);

}

/**********************************************************/
void ParticleThread::update(IplImage *img)
{
    mutex.wait();
    image = cvCloneImage(img);
    mutex.post();
    event.signal();
}

/**********************************************************/
IplImage* ParticleThread::bgr2hsv( IplImage* bgr )
{
    IplImage* bgr32f, * hsv;

    bgr32f = cvCreateImage( cvGetSize(bgr), IPL_DEPTH_32F, 3 );
    hsv = cvCreateImage( cvGetSize(bgr), IPL_DEPTH_32F, 3 );
    cvConvertScale( bgr, bgr32f, 1.0 / 255.0, 0 );
    cvCvtColor( bgr32f, hsv, CV_BGR2HSV );
    cvReleaseImage( &bgr32f );
    return hsv;
}

/**********************************************************/
void ParticleThread::free_regions( CvRect** regions, int n)
{
    for (int i = 0; i < n; i++)
        free (regions[i]);

    free(regions);
}

/**********************************************************/
void ParticleThread::free_histos( ParticleThread::histogram** histo, int n)
{
    for (int i = 0; i < n; i++)
        free (histo[i]);

    free(histo);
}

/**********************************************************/
int ParticleThread::get_regionsImage( IplImage* frame, CvRect** regions )
{
    int param;
    CvRect* r;
    int tplWidth  = object->tpl->width;
    int tplHeight = object->tpl->height;

    param = 0;

    int res_width  = frame->width - tplWidth + 1;
    int res_height = frame->height - tplHeight + 1;

    IplImage* res = cvCreateImage( cvSize( res_width, res_height ), IPL_DEPTH_32F, 1 );

    CvPoint	minloc, maxloc;
	double	minval, maxval;

    cvMatchTemplate( frame, object->tpl, res, CV_TM_SQDIFF );

    cvMinMaxLoc( res, &minval, &maxval, &minloc, &maxloc, 0 );

    cvReleaseImage( &res );

    param = 1;
    // extract regions defined by user; store as an array of rectangles
    r = (CvRect*) malloc ( param * sizeof( CvRect ) );

    for(int i = 0; i < param; i++ )
        r[i] = cvRect( minloc.x, minloc.y, tplWidth, tplHeight );

    /* draw red rectangle */
    cvRectangle( frame,
                cvPoint( minloc.x, minloc.y ),
                cvPoint( minloc.x + tplWidth, minloc.y + tplHeight ),
                cvScalar( 0, 0, 255, 0 ), 1, 0, 0 );

    *regions = r;
    return param;
}

/**********************************************************/
ParticleThread::histogram** ParticleThread::compute_ref_histos( IplImage* frame, CvRect* regions, int n )
{
    histogram** histos = (histogram**) malloc( n * sizeof( histogram* ) );
    IplImage* tmp;
    int i;

    // extract each region from frame and compute its histogram
    for( i = 0; i < n; i++ )
    {
        cvSetImageROI( frame, regions[i]);
        tmp = cvCreateImage( cvGetSize(frame), IPL_DEPTH_32F, 3 );
        cvCopy( frame, tmp, NULL );
        cvResetImageROI( frame );
        histos[i] = calc_histogram( &tmp, 1 );
        normalize_histogram( histos[i] );
        cvReleaseImage( &tmp );
    }
    return histos;
}

/**********************************************************/
ParticleThread::histogram* ParticleThread::calc_histogram( IplImage** imgs, int n )
{
    IplImage* img;
    histogram* histo;
    IplImage* h, * s, * v;
    float* hist;
    int i, r, c, bin;
    histo = (histogram*) malloc( sizeof(histogram) );
    histo->n = NH*NS + NV;
    hist = histo->histo;
    memset( hist, 0, histo->n * sizeof(float) );
    for( i = 0; i < n; i++ )
    {
        // extract individual HSV planes from image
        img = imgs[i];
        h = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 1 );
        s = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 1 );
        v = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 1 );
        cvCvtPixToPlane( img, h, s, v, NULL );

        // increment appropriate histogram bin for each pixel
        for( r = 0; r < img->height; r++ )
            for( c = 0; c < img->width; c++ )
            {
                bin = histo_bin( pixval32f( h, r, c ),
                                pixval32f( s, r, c ),
                                pixval32f( v, r, c ) );
                hist[bin] += 1;
            }
        cvReleaseImage( &h );
        cvReleaseImage( &s );
        cvReleaseImage( &v );
    }
    return histo;
}

/**********************************************************/
void ParticleThread::normalize_histogram( ParticleThread::histogram* histo )
{
    float* hist;
    float sum = 0, inv_sum;
    int i, n;

    hist = histo->histo;
    n = histo->n;

    // compute sum of all bins and multiply each bin by the sum's inverse
    for( i = 0; i < n; i++ )
        sum += hist[i];
    inv_sum = 1.0f / sum;
    for( i = 0; i < n; i++ )
        hist[i] *= inv_sum;
}

/**********************************************************/
int ParticleThread::histo_bin( float h, float s, float v )
{
    int hd, sd, vd;

    // if S or V is less than its threshold, return a "colorless" bin
    vd = MIN( (int)(v * NV / V_MAX), NV-1 );
    if( s < S_THRESH  ||  v < V_THRESH )
        return NH * NS + vd;

    // otherwise determine "colorful" bin
    hd = MIN( (int)(h * NH / H_MAX), NH-1 );
    sd = MIN( (int)(s * NS / S_MAX), NS-1 );
    return sd * NH + hd;
}

/**********************************************************/
float ParticleThread::pixval32f(IplImage* img, int r, int c)
{
    return ( (float*)(img->imageData + img->widthStep*r) )[c];
}

/**********************************************************/
ParticleThread::particle* ParticleThread::init_distribution( CvRect* regions, histogram** histos, int n, int p)
{
    particle* particles;
    int np;
    float x, y;
    int i, j, width, height, k = 0;

    particles = (particle* )malloc( p * sizeof( particle ) );
    np = p / n;

    // create particles at the centers of each of n regions
    for( i = 0; i < n; i++ ) {
        width = regions[i].width;
        height = regions[i].height;
        x = regions[i].x + (float)(width / 2);
        y = regions[i].y + (float) (height / 2);
        for( j = 0; j < np; j++ )
        {
            particles[k].x0 = particles[k].xp = particles[k].x = x;
            particles[k].y0 = particles[k].yp = particles[k].y = y;
            particles[k].sp = particles[k].s = 1.0;
            particles[k].width = width;
            particles[k].height = height;
            particles[k].histo = histos[i];
            particles[k++].w = 0;
        }
    }
    // make sure to create exactly p particles
    i = 0;
    while( k < p )
    {
        width = regions[i].width;
        height = regions[i].height;
        x = regions[i].x + (float) (width / 2);
        y = regions[i].y + (float) (height / 2);
        particles[k].x0 = particles[k].xp = particles[k].x = x;
        particles[k].y0 = particles[k].yp = particles[k].y = y;
        particles[k].sp = particles[k].s = 1.0;
        particles[k].width = width;
        particles[k].height = height;
        particles[k].histo = histos[i];
        particles[k++].w = 0;
        i = ( i + 1 ) % n;
    }
    return particles;
}

/**********************************************************/
ParticleThread::particle ParticleThread::transition( const ParticleThread::particle &p, int w, int h, gsl_rng* rng )
{
    float x, y, s;
    ParticleThread::particle pn;

    // sample new state using second-order autoregressive dynamics
    x = pfot_A1 * ( p.x - p.x0 ) + pfot_A2 * ( p.xp - p.x0 ) +
    pfot_B0 * (float)(gsl_ran_gaussian( rng, TRANS_X_STD )) + p.x0;
    pn.x = MAX( 0.0f, MIN( (float)w - 1.0f, x ) );
    y = pfot_A1 * ( p.y - p.y0 ) + pfot_A2 * ( p.yp - p.y0 ) +
    pfot_B0 * (float)(gsl_ran_gaussian( rng, TRANS_Y_STD )) + p.y0;
    pn.y = MAX( 0.0f, MIN( (float)h - 1.0f, y ) );
    s = pfot_A1 * ( p.s - 1.0f ) + pfot_A2 * ( p.sp - 1.0f ) +
    pfot_B0 * (float)(gsl_ran_gaussian( rng, TRANS_S_STD )) + 1.0f;
    pn.s = MAX( 0.1f, s );

    pn.xp = p.x;
    pn.yp = p.y;
    pn.sp = p.s;
    pn.x0 = p.x0;
    pn.y0 = p.y0;
    pn.width = p.width;
    pn.height = p.height;
    pn.histo = p.histo;
    pn.w = 0;

    return pn;
}

/**********************************************************/
float ParticleThread::likelihood( IplImage* img, int r, int c, int w, int h, histogram* ref_histo )
{
    IplImage* tmp;
    histogram* histo;
    float d_sq;

    // extract region around (r,c) and compute and normalize its histogram
    cvSetImageROI( img, cvRect( c - w / 2, r - h / 2, w, h ) );
    tmp = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 3 );
    cvCopy( img, tmp, NULL );
    cvResetImageROI( img );
    histo = calc_histogram( &tmp, 1 );
    cvReleaseImage( &tmp );
    normalize_histogram( histo );

    // compute likelihood as e^{\lambda D^2(h, h^*)}
    d_sq = histo_dist_sq( histo, ref_histo );
    free( histo );
    return exp( -LAMBDA * d_sq );
}

/**********************************************************/
float ParticleThread::histo_dist_sq( histogram* h1, histogram* h2 )
{
    float* hist1, * hist2;
    float sum = 0;
    int i, n;

    n = h1->n;
    hist1 = h1->histo;
    hist2 = h2->histo;
    //  According the the Bhattacharyya similarity coefficient,
    //  D = \sqrt{ 1 - \sum_1^n{ \sqrt{ h_1(i) * h_2(i) } } }

    for( i = 0; i < n; i++ )
        sum += sqrt( hist1[i]*hist2[i] );

    if(sum<1.0f)
        return sqrt(1.0f - sum);
    else
    {
        // should be impossible to get here...
        fprintf(stdout,"Error in similarity computation!\n");
        return 1.0f-sum;
    }
}

/**********************************************************/
void ParticleThread::normalize_weights( particle* particles, int n )
{
    float sum = 0;
    int i;

    for( i = 0; i < n; i++ )
        sum += particles[i].w;
    for( i = 0; i < n; i++ )
        particles[i].w /= sum;
}

/**********************************************************/
ParticleThread::particle* ParticleThread::resample( particle* particles, int n )
{
    particle* _new_particles;
    int i, j, np, k = 0;

    //new_particles = (particle* )malloc( sizeof( particle ) );
    qsort( particles, n, sizeof( particle ), &particle_cmp );
    _new_particles = (particle* ) malloc( n * sizeof( particle ) );

    for( i = 0; i < n; i++ )
    {
        np = cvRound( particles[i].w * n );
        for( j = 0; j < np; j++ )
        {
            _new_particles[k++] = particles[i];
            if( k == n )
                goto exit;
        }
    }
    while( k < n )
        _new_particles[k++] = particles[0];

exit:
    return _new_particles;
}

/**********************************************************/
int particle_cmp( const void* p1, const void* p2 )
{
    ParticleThread::particle* _p1 = (ParticleThread::particle*)p1;
    ParticleThread::particle* _p2 = (ParticleThread::particle*)p2;

    if( _p1->w > _p2->w )
        return -1;
    if( _p1->w < _p2->w )
        return 1;
    return 0;
}

/**********************************************************/
void ParticleThread::retreive_particle( particle* p )
{
    int x0, y0, x1, y1;
    x0 = cvRound( p[0].x - 0.5 * p[0].s * p[0].width );
    y0 = cvRound( p[0].y - 0.5 * p[0].s * p[0].height );
    x1 = x0 + cvRound( p[0].s * p[0].width );
    y1 = y0 + cvRound( p[0].s * p[0].height );

    container.lock();
    object->boundingBox.clear();
    object->boundingBox.push_back((x0+x1)/2);
    object->boundingBox.push_back((y0+y1)/2);
    object->boundingBox.push_back(x0);
    object->boundingBox.push_back(y0);
    object->boundingBox.push_back(x1);
    object->boundingBox.push_back(y1);

    object->particlePoints.clear();
    for (int i=0; i<num_particles; i++)
    {
        CvPoint point;
        point.x = p[i].x;
        point.y = p[i].y;
        object->particlePoints.push_back( point );
    }
    container.unlock();
}
//empty line to make gcc happy
