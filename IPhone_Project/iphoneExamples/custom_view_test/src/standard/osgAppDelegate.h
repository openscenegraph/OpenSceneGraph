//Created by Thomas Hogarth 2009

//force the link to our desired osgPlugins
#include "osgPlugins.h"

#include "Sketch.h"


#import <UIKit/UIKit.h>


@interface osgAppDelegate : NSObject <UIApplicationDelegate> {

	osg::ref_ptr<cefix::Sketch> _sketch;

}

- (void)updateScene;

@end

