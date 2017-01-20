//
//  ofxDraco.h
//  test_draco
//
//  Created by satcy on 2017/01/20.
//
//

#pragma once
#include "ofMain.h"

#include "compression/decode.h"
#include "compression/encode.h"

namespace ofxDraco {
    bool decode(string path, ofMesh & m);
    bool encode(const ofMesh & m, ofBuffer & buff);
} // namespace ofxDraco
