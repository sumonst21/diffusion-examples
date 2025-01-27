//  Diffusion Client Library for iOS, tvOS and OS X / macOS - Examples
//
//  Copyright (C) 2022 Push Technology Ltd.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#import "InitialSessionEstablishmentRetryStrategy.h"

@import Diffusion;

@implementation InitialSessionEstablishmentRetryStrategy {
    PTDiffusionSession* _session;
}

-(void)startWithURL:(NSURL*)url {

    NSLog(@"Connecting...");

    PTDiffusionMutableSessionConfiguration *const configuration = [[PTDiffusionMutableSessionConfiguration alloc] init];

    // Create an initial session establishment retry strategy.
    // It will attempt 10 times to connect to the Diffusion server, with 500 milliseconds interval between attempts.
    configuration.initialRetryStrategy = [[PTDiffusionRetryStrategy alloc] initWithInterval:500 andAttempts:10];

    [PTDiffusionSession openWithURL:url
                      configuration:configuration
                  completionHandler:^(PTDiffusionSession *session, NSError *error)
    {
        if (!session) {
            NSLog(@"Failed to open session: %@", error);
            return;
        }

        // At this point we now have a connected session.
        NSLog(@"Connected. Session Identifier: %@", session.sessionId);

        // Set ivar to maintain a strong reference to the session.
        self->_session = session;
    }];
}

@end
