//
//  FlipsideViewController.h
//  custom_view_test
//
//  Created by Stephan Huber on 04.05.11.
//  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include <osgViewer/Viewer>

@protocol FlipsideViewControllerDelegate;


@interface FlipsideViewController : UIViewController {
	id <FlipsideViewControllerDelegate> delegate;
    
    osg::ref_ptr<osgViewer::Viewer> _viewer;
	CADisplayLink* _displayLink;
}

- (void)initOsg;

@property (nonatomic, assign) id <FlipsideViewControllerDelegate> delegate;
- (IBAction)done:(id)sender;
@end


@protocol FlipsideViewControllerDelegate
- (void)flipsideViewControllerDidFinish:(FlipsideViewController *)controller;
@end

