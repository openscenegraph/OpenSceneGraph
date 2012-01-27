//
//  MainViewController.h
//  custom_view_test
//
//  Created by Stephan Huber on 04.05.11.
//  Copyright 2011 Stephan Maximilian Huber, digital mind. All rights reserved.
//

#import "FlipsideViewController.h"

@interface MainViewController : UIViewController <FlipsideViewControllerDelegate> {
}

- (IBAction)showInfo:(id)sender;

@end
