#ifndef __CVERROR_HPP__
#define __CVERROR_HPP__

#include <iostream>

namespace shun {

    #define SHOW_ERROR_AND_RETURN(x) \
        do { \
            if ((x) != CVError::NOERROR) { \
                CVErrorMessage::Show((x), __FILE__, __FUNCTION__, __LINE__); \
                return (x); \
            } \
        } while (0)

    enum class CVError : int {
        NOERROR = 0,
        MEMORY,
        INPUT,
        FILEACCESS,
        UNKNOWN,
    };

    class CVErrorMessage {
        public:
            static void Show(CVError error, const char *file, const char *function, int line) 
            {
                switch (error)
                {
                case CVError::NOERROR:
                    break;

                case CVError::MEMORY:
                    std::cerr << "[Error] No memory: " << file << ", " << function << ", " << line << std::endl;
                    break;
                
                case CVError::INPUT:
                    std::cerr << "[Error] Input error: " << file << ", " << function << ", " << line << std::endl;
                    break;

                case CVError::FILEACCESS:
                    std::cerr << "[Error] Access file error: " << file << ", " << function << ", " << line << std::endl;
                    break;

                case CVError::UNKNOWN:
                default:
                    std::cerr << "Unknown error: " << file << ", " << function << ", " << line << std::endl;
                    break;
                }
            }
    };
    
}

#endif // __CVERROR_HPP__