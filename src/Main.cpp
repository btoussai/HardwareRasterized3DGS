//
// Created by Briac on 18/06/2025.
//

#include <iostream>

#include <string>
#include <iostream>

#include <exception>
#include <csignal>
#include <cassert>

#include "Window.cuh"

void my_terminate_handler() {
    std::cout << "Unhandled exception" << std::endl;
    std::abort();
}


extern "C" void handle_aborts(int signal_number)
{
    switch(signal_number){
        case SIGABRT:
            std::cout <<"SIGABRT";
            break;
        case SIGSEGV:
            std::cout <<"SIGSEGV";
            break;
        case SIGTERM:
            std::cout <<"SIGTERM";
            break;
        case SIGFPE:
            std::cout <<"SIGFPE";
            break;
        default:
            std::cout <<signal_number;
            break;
    }
    std::cout << " signal received, shutting down." <<std::endl;
    std::flush(std::cout);

    signal(signal_number, SIG_DFL);
}

int main(int argc, char* argv[]){

    std::set_terminate(my_terminate_handler);
    signal(SIGABRT, &handle_aborts);
    signal(SIGSEGV, &handle_aborts);
    signal(SIGTERM, &handle_aborts);
    signal(SIGFPE, &handle_aborts);

    bool error = false;

    int samples = 1;
    Window w(std::string("HardwareRasterized3DGS"), samples);
    try {
        w.mainloop(argc, argv);
    } catch (const char* msg) {
        std::cout << msg << std::endl;
        error = true;
    } catch (const std::string& msg) {
        std::cout << msg << std::endl;
        error = true;
    } catch (const std::bad_cast& e) {
        std::cout << e.what() << std::endl;
        error = true;
    }catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        error = true;
    }
    std::flush(std::cout);

    return error ? 1 : 0;

}