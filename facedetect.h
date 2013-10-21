#ifndef MAIN_H
#define MAIN_H
#include <cstdio>
#include <string>

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "jpeg_mem_src.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

class GetURLHandler;
class facedetectInstance : public pp::Instance {
    public:
        IplImage* frame;
        explicit facedetectInstance(PP_Instance instance) : pp::Instance(instance) {}
        virtual ~facedetectInstance() {}

        virtual void HandleMessage(const pp::Var&);
        void HandleImage(const std::string&);
        void HandleXml(const std::string&);
        void CreateMemFile(const std::string&, const std::string&);
        void RecognizeFace();
    private:
        int width, height;
        std::string url;
        GetURLHandler* handler;
        bool classifierCreated;
};
#endif
