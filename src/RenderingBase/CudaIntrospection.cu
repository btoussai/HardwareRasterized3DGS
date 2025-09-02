//
// Created by Briac on 05/08/2025.
//

#include "CudaIntrospection.cuh"

#include "../imgui/imgui.h"
#include <mutex>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

static std::mutex mut;

std::unordered_map<void*, CudaIntrospection::Buff> CudaIntrospection::buffers;


std::string prettyPrintMem(size_t bytes) {
    std::stringstream ss;
    ss <<std::fixed <<std::setprecision(2);

    int n = (int)log10(bytes) / 3;

    if(n > 0 && n <= 3){
        ss <<bytes * pow(10, -n * 3);
    }else{
        ss <<bytes;
    }

    switch(n){
        case 1:
            ss <<" kB";
            break;
        case 2:
            ss <<" MB";
            break;
        case 3:
            ss <<" GB";
            break;
        default:
            ss <<" bytes";
    }

    return ss.str();
}

void CudaIntrospection::addBuffer(void *ptr, size_t size, const std::string& name) {
    if(ptr == nullptr || size == 0){
        throw std::string("Error, ptr==0 or size==0.");
    }
    std::lock_guard<std::mutex> lock(mut);
    if(buffers.contains(ptr)){
        throw std::string("Error, buffer is already in use.");
    }
//    std::cout <<"Adding cuda buffer " << ptr <<" " << prettyPrintMem(size) <<std::endl;
    buffers[ptr] = {ptr, size, name};
}

void CudaIntrospection::removeBuffer(void *ptr, size_t size) {
    if(ptr){
        std::lock_guard<std::mutex> lock(mut);
        if(!buffers.contains(ptr)){
            throw std::string("Error, buffer has already been removed.");
        }
//        std::cout <<"Removing cuda buffer " << ptr <<" " <<prettyPrintMem(size) <<std::endl;
        buffers.erase(buffers.find(ptr));
    }
}



void CudaIntrospection::inspectBuffers() {
    std::lock_guard<std::mutex> lock(mut);

    if(ImGui::BeginMenu("Cuda Allocations")){

        std::vector<Buff> sortedBuffers;
        sortedBuffers.reserve(buffers.size());

        size_t totalBufSize = 0;

        for(auto [ptr, buff] : buffers){
            totalBufSize += buff.size;
            sortedBuffers.push_back(buff);
        }

        ImGui::Text("Total size: %s", prettyPrintMem(totalBufSize).c_str());

        if(ImGui::TreeNode("Cuda Buffers", "%d Buffers: %s", (int)buffers.size(), prettyPrintMem(totalBufSize).c_str())){

            std::sort(sortedBuffers.begin(), sortedBuffers.end(), [](const auto& a, const auto& b){ return a.size > b.size; });
            for(const auto& buff : sortedBuffers){
                ImGui::Text( "%s : %s", buff.name.c_str(), prettyPrintMem(buff.size).c_str());
            }
            ImGui::TreePop();
        }

        ImGui::EndMenu();
    }


}
