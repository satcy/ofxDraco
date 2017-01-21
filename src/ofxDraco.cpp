//
//  ofxDraco.cpp
//  test_draco
//
//  Created by satcy on 2017/01/20.
//
//

#include "ofxDraco.h"


namespace ofxDraco {
    //ofMesh toOf(const draco::Mesh & m) {}
    
    //draco::Mesh toDraco(const ofMesh & m) {}
    
    ofMesh load(const string & path) {
        ofMesh m;
        decode(path, m);
        return m;
    }
    
    bool decode(const string & input_path, ofMesh & dst) {
        
        ofFile input_file(input_path, ofFile::ReadOnly, true);
        
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
                dst.addVertices(verts);
                dst.addIndices(inds);
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
            dst.addVertices(verts);
        }

        mesh = nullptr;
        
        return true;
    }
    
    // private
    int EncodePointCloudToFile(const draco::PointCloud &pc,
                               const draco::EncoderOptions &options,
                               const std::string &file) {
        
        // Encode the geometry.
        draco::EncoderBuffer buffer;
        
        if (!draco::EncodePointCloudToBuffer(pc, options, &buffer)) {
            printf("Failed to encode the point cloud.\n");
            return -1;
        }
        
        // Save the encoded geometry into a file.
        std::ofstream out_file(file, std::ios::binary);
        if (!out_file) {
            printf("Failed to create the output file.\n");
            return -1;
        }
        out_file.write(buffer.data(), buffer.size());
        //printf("\nEncoded size = %zu bytes\n\n", buffer.size());
        return 0;
    }
    
    // private
    int EncodeMeshToFile(const draco::Mesh &mesh,
                         const draco::EncoderOptions &options,
                         const std::string &file) {
        
        // Encode the geometry.
        draco::EncoderBuffer buffer;
        
        if (!draco::EncodeMeshToBuffer(mesh, options, &buffer)) {
            printf("Failed to encode the mesh.\n");
            return -1;
        }
        
        // Save the encoded geometry into a file.
        std::ofstream out_file(file, std::ios::binary);
        if (!out_file) {
            printf("Failed to create the output file.\n");
            return -1;
        }
        out_file.write(buffer.data(), buffer.size());
        //printf("\nEncoded size = %zu bytes\n\n", buffer.size());
        return 0;
    }
    
    bool encodeFromFile(const string & input_path,
                        const string & output_path,
                        bool is_absolute_path,
                        bool is_point_cloud,
                        int pos_quantization_bits,
                        int tex_coords_quantization_bits,
                        int normals_quantization_bits,
                        int compression_level) {
        
        std::unique_ptr<draco::PointCloud> pc;
        draco::Mesh *mesh = nullptr;
        if ( !is_point_cloud ) {
            std::unique_ptr<draco::Mesh> in_mesh = draco::ReadMeshFromFile(is_absolute_path ? input_path : ofToDataPath(input_path, true));
            
            if (!in_mesh) {
                printf("Failed loading the input mesh.\n");
                return -1;
            }
            mesh = in_mesh.get();
            pc = std::move(in_mesh);
        } else {
            pc = draco::ReadPointCloudFromFile(is_absolute_path ? input_path : ofToDataPath(input_path, true));
            if (!pc) {
                printf("Failed loading the input point cloud.\n");
                return -1;
            }
        }
        
        // Setup encoder options.
        draco::EncoderOptions encoder_options = draco::CreateDefaultEncoderOptions();
        if (pos_quantization_bits > 0) {
            draco::SetNamedAttributeQuantization(&encoder_options, *pc.get(),
                                                 draco::GeometryAttribute::POSITION,
                                                 pos_quantization_bits);
        }
        if (tex_coords_quantization_bits > 0) {
            draco::SetNamedAttributeQuantization(&encoder_options, *pc.get(),
                                                 draco::GeometryAttribute::TEX_COORD,
                                                 tex_coords_quantization_bits);
        }
        if (normals_quantization_bits > 0) {
            draco::SetNamedAttributeQuantization(&encoder_options, *pc.get(),
                                                 draco::GeometryAttribute::NORMAL,
                                                 normals_quantization_bits);
        }
        // Convert compression level to speed (that 0 = slowest, 10 = fastest).
        const int speed = 10 - compression_level;
        draco::SetSpeedOptions(&encoder_options, speed, speed);
        
        if (mesh && mesh->num_faces() > 0)
            return EncodeMeshToFile(*mesh, encoder_options, is_absolute_path ? output_path : ofToDataPath(output_path, true));
        return EncodePointCloudToFile(*pc.get(), encoder_options, is_absolute_path ? output_path : ofToDataPath(output_path, true));
        
        return true;
    }
    
} // namespace ofxDraco
