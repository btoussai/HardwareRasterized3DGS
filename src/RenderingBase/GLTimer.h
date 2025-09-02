//
// Created by Briac on 28/08/2025.
//

#ifndef HARDWARERASTERIZED3DGS_GLTIMER_H
#define HARDWARERASTERIZED3DGS_GLTIMER_H

#include <cstdint>
#include <list>
#include <vector>
#include "../glad/gl.h"

class Query{
private:
    GLuint ID;
    GLenum type;
    GLsync s = {};
public:
    Query& operator=(const Query&) = delete;
    Query(const Query&) = delete;
    Query(Query&&) = delete;
    void operator=(Query&&) = delete;

    explicit Query(GLenum type) : ID(0), type(type){
        glGenQueries(1, &ID);
    }

    ~Query(){
        glDeleteQueries(1, &ID);
        glDeleteSync(s);
    }

    void begin() const{
        glBeginQuery(type, ID);
    }

    void end(){
        glEndQuery(type);
        s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    /**
     * Only if the query is GL_TIMESTAMP
     */
    void queryCounter(){
        glQueryCounter(ID, type);
        s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    bool resultAvailable(){
        return glClientWaitSync(s, 0, 0) == GL_ALREADY_SIGNALED;
    }

    bool getResultNoWait(int64_t& res){
        if(!resultAvailable()){
            return false;
        }

        glGetQueryObjecti64v(ID, GL_QUERY_RESULT, &res);
        return true;
    }

    void getResult(int64_t& res){
        glGetQueryObjecti64v(ID, GL_QUERY_RESULT, &res);
    }
};

class QueryBuffer{
private:
    GLenum type=GL_TIME_ELAPSED;
    std::list<Query> buffer;
    int64_t lastResult = 0;
    int64_t total = 0;
    int max_size=100;
public:
    QueryBuffer(QueryBuffer&&)  noexcept = default;
    QueryBuffer& operator=(QueryBuffer&&) = default;

    QueryBuffer& operator=(const QueryBuffer&) = delete;
    QueryBuffer(const QueryBuffer&) = delete;

    QueryBuffer()= default;

    explicit QueryBuffer(GLenum type, int max_size = 100) : type(type), max_size(max_size){

    };
    ~QueryBuffer() = default;
    Query& push_back(){
        if(size() > max_size){
            getLastResult();
            if(!buffer.empty())buffer.pop_front();
        }
        return buffer.emplace_back(type);
    }
    Query* last(){
        if(buffer.size() > 0){
            return &buffer.back();
        }
        return nullptr;
    }

    int size() const{
        return (int)buffer.size();
    }

    bool resultAvailable(){
        if(size() > 0){
            return buffer.front().resultAvailable();
        }
        return false;
    }

    int64_t getTotal(bool update=true) {
        getLastResult(update);
        return total;
    }

    int64_t getLastResult(bool update=true) {
        if(update){
            while(getResultAndPopFrontIfAvailable(lastResult));
        }
        return lastResult;
    }
    int64_t getNLastResults(const int N, bool update=true) {
        if(update){
            std::vector<int64_t> results;
            results.reserve(max_size);
            int64_t res = 0;
            while(getResultAndPopFrontIfAvailable(res)){
                results.push_back(res);
            }
            res = 0;
            for(int i=0; i<std::min(N, (int)results.size()); i++){
                res += results[(int)results.size()-1-i];
            }
            if(res != 0){
                lastResult = res;
            }
        }
        return lastResult;
    }
    bool getResultAndPopFrontIfAvailable(int64_t& res){
        if(buffer.empty()){
            return false;
        }
        if(buffer.front().getResultNoWait(res)){
            buffer.pop_front();
            total += res;
            return true;
        }
        return false;
    }
};

#endif //HARDWARERASTERIZED3DGS_GLTIMER_H
