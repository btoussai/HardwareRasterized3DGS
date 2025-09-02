//
// Created by Briac on 05/08/2025.
//

#ifndef SPARSEVOXRECON_CUDAINTROSPECTION_CUH
#define SPARSEVOXRECON_CUDAINTROSPECTION_CUH

#include <unordered_map>
#include <string>

class CudaIntrospection {
public:
    static void addBuffer(void* ptr, size_t size, const std::string& name);
    static void removeBuffer(void* ptr, size_t size);
    static void inspectBuffers();
private:
    struct Buff{
        void* ptr;
        size_t size;
        std::string name;
    };
    static std::unordered_map<void*, Buff> buffers;
};


#endif //SPARSEVOXRECON_CUDAINTROSPECTION_CUH
