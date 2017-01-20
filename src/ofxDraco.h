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
#include "io/mesh_io.h"
#include "io/point_cloud_io.h"


namespace ofxDraco {
    //ofMesh toOf(const draco::Mesh & m);
    //draco::Mesh toDraco(const ofMesh & m);
    
    ofMesh load(const string & path);
    bool decode(const string & input_path, ofMesh & dst);
    
    bool encodeFromFile(const string & input_path, const string & out_path, bool is_point_cloud = false, int pos_quantization_bits = 14, int tex_coords_quantization_bits = 12, int normals_quantization_bits = 10, int compression_level = 5);
    
} // namespace ofxDraco
