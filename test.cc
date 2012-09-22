#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
extern "C" {
#include "jpeglib.h"
}

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include "filesys/base/Mount.h"
#include "filesys/base/MountManager.h"
#include "filesys/memory/MemMount.h"

#include "jpeg_mem_src.h"
#include "geturl_handler.h"

using namespace std;
using namespace cv;

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurence of the <embed> tag that has these
/// attributes:
///     type="application/x-nacl"
///     src="<PROJECT_NAME>.nmf"
/// To communicate with the browser, you must override HandleMessage() for
/// receiving messages from the borwser, and use PostMessage() to send messages
/// back to the browser.  Note that this interface is entirely asynchronous.

/* Read JPEG image from a memory segment */

void testInstance::HandleMessage(const pp::Var& var_message) {
	// download the url in the var_message
	stringstream convert;
	string data = var_message.AsString();
	convert << data.substr(0, data.find_first_of('|'));
	convert >> this->width;
	stringstream convert2;
	convert2 << data.substr(data.find_first_of('|')+1, data.find_last_of('|')-data.find_first_of('|')-1);
	convert2 >> this->height;
	this->url = data.substr(data.find_last_of('|')+1, data.length());
	this->handler = GetURLHandler::Create(this, this->url);
	this->handler->file = GetURLHandler::URL_IMAGE;
	if(this->handler != NULL){
		this->handler->Start();
	}
	// test core
	// create new 320x240 image
	//Mat img(3, 6, 3), img2(3,6,3);
	// create a 100x100x100 8-bit array
	//int sz[] = {100, 100, 100};
	//Mat bigCube(3, sz, CV_8U, Scalar::all(0));
	// make a 7x7 complex matrix filled with 1+3j.
	//Mat M(7,7,CV_32FC2,Scalar(1,3));
	// and now turn M to a 100x60 15-channel 8-bit matrix.
	// The old content will be deallocated
	//Mat A = Mat::eye(4, 4, CV_32F)*0.1;
	//M.create(100,60,CV_8UC(15));
	// check core dft, with out this one, the blur function of imgproc will fail to be recognized, it's a bizarre error.
/*
	dft(img, img2);
	// TODO(sdk_user): 1. Make this function handle the incoming message.
	// test imgproc module
	blur(img, img2, Size(3,3));
	// test flann
	flann::IndexParams params;
	// Test libpng
	//png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	// Test highgui
        //(1) jpeg compression
        vector<uchar> buff;//buffer for coding
        vector<int> param = vector<int>(2);
        param[0]=CV_IMWRITE_JPEG_QUALITY;
        param[1]=95;//default(95) 0-100
        imencode(".jpg",img,buff,param);
	cvLoadImage("test.jpg");
	// these function belong to imgproc module, but putting it here,
	// the objdetect module will fail
	cvtColor(img, img, CV_BGR2Luv);
	resize(img, img2, img2.size(), 0, 0, INTER_NEAREST);
	Canny(img, img2, 1.0, 1.0);
	// test objdetect
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
	CascadeClassifier face_cascade;
        face_cascade.load(face_cascade_name);
*/
}

void testInstance::HandleImage(const string& message){
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;

        // Initialize the JPEG decompression object with default error handling.
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);

        // Specify data source for decompression
        jpeg_mem_src(&cinfo, (unsigned char*)message.c_str(),(unsigned long)message.size());

        // Read file header, set default decompression parameters
        (void) jpeg_read_header(&cinfo, TRUE);


        // Start decompressor
        (void) jpeg_start_decompress(&cinfo);

	// Read the image row by row
	this->frame = cvCreateImage(cvSize(cinfo.output_width,cinfo.output_height),
                                        IPL_DEPTH_8U, cinfo.output_components);
        JSAMPARRAY imageBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, 
                                     cinfo.output_width*cinfo.output_components, 1);
        for(unsigned int y=0;y< cinfo.output_height;y++){
                jpeg_read_scanlines(&cinfo, imageBuffer, 1);
                uint8_t* dstRow = (uint8_t*) this->frame->imageData + this->frame->widthStep*y;
                memcpy(dstRow, imageBuffer[0], cinfo.output_width*cinfo.output_components);
        }

	// If the image is 3 channel convert it into BGR (OpenCV standard)
        if(cinfo.output_components == 3){
                cvCvtColor(this->frame, this->frame, CV_RGB2BGR);
        }

	// if XML classifier has already been loaded, skip it
	if(this->classifierCreated == true){
		this->RecognizeFace();
	} else {
		// get xml file
		this->handler = GetURLHandler::Create(this, "haarcascade_frontalface_alt.xml");
		this->handler->file = GetURLHandler::URL_XML;
		if(this->handler != NULL){
			this->handler->Start();
		}
	}
}

void testInstance::HandleXml(const string& content){
	const char* cascadefilename = "/haarcascade_frontalface_alt.xml";
	this->CreateMemFile(content, cascadefilename);
	this->RecognizeFace();
}

void testInstance::CreateMemFile(const string& content, const string& filename){
	// stream message for debugging
	stringstream ss;
	// test filesys module of nacl-mount to add xml file
	MountManager *mm = MountManager::MMInstance();
	KernelProxy* kp = MountManager::MMInstance()->kp();
	MemMount *mnt = new MemMount();
	mm->AddMount(mnt, "/");
	int fd = kp->open(filename, O_WRONLY | O_CREAT, 0644);
	if(fd == -1){
		ss << "--(!)Error creating cascade classifier";
		this->PostMessage(pp::Var(ss.str()));
		return;
	}
	kp->write(fd, content.c_str(), content.size());
	kp->close(fd);
	this->classifierCreated = true;

	/* TEST LOADING THE XML FILE FROM THE MEM STORAGE//
	// load xml face cascade to detect face
	FileStorage fs(cascadeFilename, FileStorage::READ);
	fs.open(cascadeFilename, FileStorage::READ);
	if(!fs.isOpened())
		this->PostMessage(pp::Var("Failed to open file"));
	fs.getFirstTopLevelNode();
	int size1 = (int)fs["test"];
	fs.release();
	*/
}

void testInstance::RecognizeFace(){
	// stream message for debugging
	stringstream ss;
	const char* cascadeFilename = "haarcascade_frontalface_alt.xml";
	CascadeClassifier face_cascade;
	face_cascade.load(cascadeFilename);
	if( !face_cascade.load(cascadeFilename) ){ 
		ss << "--(!)Error loading cascade classifier";
		this->PostMessage(pp::Var(ss.str()));
		return;
	}

        std::vector<Rect> faces;
        Mat frame_gray;

        cvtColor( Mat(this->frame), frame_gray, CV_BGR2GRAY );
        equalizeHist( frame_gray, frame_gray );

        //-- Detect faces
        face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

        for( unsigned int i = 0; i < faces.size(); i++ )
        {
		ss << "Face #" << i << " x=" << faces[i].x << " width=" << faces[i].width 
			<< " y=" << faces[i].y << " height=" << faces[i].height << "\n";
                //Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
        }

	this->PostMessage(pp::Var(ss.str()));
}

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class testModule : public pp::Module {
 public:
  testModule() : pp::Module() {}
  virtual ~testModule() {}

  /// Create and return a testInstance object.
  /// @param[in] instance The browser-side instance.
  /// @return the plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new testInstance(instance);
  }
};

namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
Module* CreateModule() {
  return new testModule();
}
}  // namespace pp
