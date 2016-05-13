#pragma once

#include <vector> 
#include <utility>

namespace example_WW_llvv {

    template<class T>
    struct Subprocess {
        public:
            using Callback = std::function<double(T&)>;
    
            Subprocess(const Callback& callback, bool mirror, const std::vector<std::pair<int, int>>& iniStates, int ncomb, int denom):
                callback(callback), 
                hasMirrorProcess(mirror), 
                initialStates(iniStates), 
                goodHel(ncomb, true), 
                denominator(denom) {
                    // Empty
                }
    
            Callback callback;
            bool hasMirrorProcess; 
            std::vector<std::pair<int, int>> initialStates; 
            std::vector<bool> goodHel; 
            int denominator;
    
        private:
            Subprocess() = delete;
    }; 

}
