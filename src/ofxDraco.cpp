//
//  ofxDraco.cpp
//  test_draco
//
//  Created by satcy on 2017/01/20.
//
//

#include "ofxDraco.h"


namespace ofxDraco {
    bool decode(string path, ofMesh & m) {
        
        ofFile input_file(path, ofFile::ReadOnly, true);
        
        if ( !input_file.exists() ) {
            printf("Failed opening the input file.\n");
            return false;
        }
        
        // Read the file stream into a buffer.
        std::streampos file_size = 0;
        input_file.seekg(0, std::ios::end);
        file_size = input_file.tellg() - file_size;
        input_file.seekg(0, std::ios::beg);
        std::vector<char> data(file_size);
        input_file.read(data.data(), file_size);
        
        if (data.empty()) {
            printf("Empty input file.\n");
            return false;
        }
        
        // Create a draco decoding buffer. Note that no data is copied in this step.
        draco::DecoderBuffer buffer;
        buffer.Init(data.data(), data.size());
        
        // Decode the input data into a geometry.
        std::unique_ptr<draco::PointCloud> pc;
        draco::Mesh *mesh = nullptr;
        const draco::EncodedGeometryType geom_type = draco::GetEncodedGeometryType(&buffer);
        
        if (geom_type == draco::TRIANGULAR_MESH) {
            std::unique_ptr<draco::Mesh> in_mesh = draco::DecodeMeshFromBuffer(&buffer);
            if (in_mesh) {
                mesh = in_mesh.get();
                pc = std::move(in_mesh);
                
                const draco::PointAttribute *const att =
                pc->GetNamedAttribute(draco::GeometryAttribute::POSITION);
                if (att == nullptr || att->size() == 0)
                    return false;  // Position attribute must be valid.
                std::array<float, 3> value;
                
                vector<ofVec3f> verts;
                for (draco::AttributeValueIndex i(0); i < att->size(); ++i) {
                    if (att->ConvertValue<float, 3>(i, &value[0]) ) {
                        ofVec3f pt(value[0], value[1], value[2]);
                        verts.emplace_back(pt);
                    }
                }
                vector<ofIndexType> inds;
                for (draco::FaceIndex i(0); i < mesh->num_faces(); ++i) {
                    for (int j = 0; j < 3; ++j) {
                        const draco::PointIndex vert_index = mesh->face(i)[j];
                        ofIndexType index = att->mapped_index(vert_index).value();
                        inds.emplace_back(index);
                    }
                }
                m.addVertices(verts);
                m.addIndices(inds);
            }
        } else if (geom_type == draco::POINT_CLOUD) {
            // Failed to decode it as mesh, so let's try to decode it as a point cloud.
            pc = draco::DecodePointCloudFromBuffer(&buffer);
            
            const draco::PointAttribute *const att =
            pc->GetNamedAttribute(draco::GeometryAttribute::POSITION);
            if (att == nullptr || att->size() == 0)
                return false;  // Position attribute must be valid.
            std::array<float, 3> value;
            
            vector<ofVec3f> verts;
            for (draco::AttributeValueIndex i(0); i < att->size(); ++i) {
                if (!att->ConvertValue<float, 3>(i, &value[0]) ) {
                    return false;
                } else {
                    ofVec3f pt(value[0], value[1], value[2]);
                    verts.emplace_back(pt);
                }
            }
            m.addVertices(verts);
        }

        mesh = nullptr;
        
        return true;
    }
    
    bool encode(const ofMesh & m, ofBuffer & buff) {
        return true;
    }
} // namespace ofxDraco
