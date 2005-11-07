/*
 *  MovieData.h
 *  encapsulates movie-related stuff
 *
 *  Created by Stephan Huber on Wed Mar 10 2004.
 *  Copyright (c) 2004 digital mind. All rights reserved.
 *
 */

#ifndef _MOVIEDATA_HEADER_
#define _MOVIEDATA_HEADER_

#include <osg/Notify>
#include <osg/Image>
#include <string>
#include <QuickTime/QuickTime.h>

namespace osg {
    
    class MovieData {
    
        public:
            MovieData();
            ~MovieData();
            
            void load(osg::Image* image, std::string fileName, float startTime = 0.0f);
            
            float getMovieDuration() { return GetMovieDuration(_movie)/(float)_timescale;}
            float getMovieTime()  {return GetMovieTime(_movie,NULL)/(float)_timescale; }
                    
            Movie &getMovie() { return _movie; }
            
        protected:
            Movie           _movie;
            GWorldPtr       _gw;
            
            unsigned int    _movieWidth, _movieHeight, _textureWidth, _textureHeight;
            float           _timescale;
            bool            _fError;
            
            void _initImage(osg::Image* image);
            void _initGWorldStuff(osg::Image * image);
            void _initTexture();
            
            inline void _checkMovieError(std::string msg) {
                if (GetMoviesError()) {
                    _fError = true;
                    osg::notify(osg::ALWAYS) << "MovieData: GetMoviesError fails at " << msg << std::endl;
                }
            }
            
    };


} // namespace

#endif
