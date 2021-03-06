#include "videoprocessor.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
VideoProcessor::VideoProcessor()
{
    callIt= true;
    delay= 0;
    fnumber= 0;
    stop= false;
    frameToStop= -1;
}
*/
// open a video file
bool VideoProcessor::setInput(std::string filename){
    fnumber= 0;
    //in case a resource was already
    //   associated with the VideoCapture instance
    capture.release();
    images.clear();
    //open the video file
    return capture.open(filename);
}

// set the vector of input images(*.jpg,*.png...)
bool VideoProcessor::setInput(const std::vector<std::string>& imgs) {
        fnumber= 0;
        // In case a resource was already
        // associated with the VideoCapture instance
        capture.release();
        // the input will be this vector of images
        images= imgs;
        itImg= images.begin();
        return true;
}

void VideoProcessor::displayInput(std::string wn){
    windowNameInput= wn;
    cv::namedWindow(windowNameInput);
}

void VideoProcessor::displayOutput(std::string wn){
    windowNameOutput= wn;
    cv::namedWindow(windowNameOutput);
}

void VideoProcessor::dontDisplay(){
    cv::destroyWindow(windowNameInput);
    cv::destroyWindow(windowNameOutput);
    windowNameInput.clear();
    windowNameOutput.clear();
}

//to grab(and process)the frames of the sequence
void VideoProcessor::run(){
    //current frame
    cv::Mat frame;
    //output frame
    cv::Mat output;

    //if no capture device has been set
    if (!isOpened())
        return;
    stop= false;
    while (!isStopped()) {
        //read next frame if any
        if (!readNextFrame(frame))
            break;
        if (windowNameInput.length()!=0)
            cv::imshow(windowNameInput,frame);

        // ** calling the process function or method **
        if (callIt) {
            // process the frame
            if (process) // if call back function
                  process(frame, output);
            else if (frameProcessor) // if class interface instance
                 frameProcessor->process(frame,output);
            // increment frame number
            fnumber++;
        } else {
        output= frame;
        }
        // ** write output sequence **
        if (outputFile.length()!=0)
        writeNextFrame(output);

        //display output frame
        if (windowNameOutput.length()!=0)
            cv::imshow(windowNameOutput,output);
        //introduce delay
        if(delay>=0 && cv::waitKey(delay)==27)
            stopIt();
        //check if we should stop
        if(frameToStop>=0 && getFrameNumber()==frameToStop)
            stopIt();
        }
    destroyAllWindows();
}

//stop the processing
void VideoProcessor::stopIt() {
    stop=true;
}

//is the processing stopped
bool VideoProcessor::isStopped() {
    return stop;
}

//is a capture device opened
bool VideoProcessor::isOpened() {
    return capture.isOpened()|| !images.empty();
}

//set a delay between each frame
//0 means wait at each frame
void VideoProcessor::setDelay(int d) {
    delay=d;
}

//to get the next frame
//could be:video file or camera
bool VideoProcessor::readNextFrame(cv::Mat& frame) {
    if (images.size()==0)
         return capture.read(frame);
    else {
         if (itImg != images.end()) {
         frame= cv::imread(*itImg);
         itImg++;
         return frame.data != 0;
         } else {
         return false;
         }
     }
}


//process callback to called
void VideoProcessor::callProcess() {
    callIt= true;
}

//do not call process callback
void VideoProcessor::dontCallProcess() {
    callIt= false;
}

//to stop at a certain frame number
void VideoProcessor::stopAtFrameNo(long frame) {
    frameToStop= frame;
}

//return the frame number of the next frame
long VideoProcessor::getFrameNumber() {
    //get info of from the capture device
    long fnumber=static_cast<long>(capture.get(CV_CAP_PROP_POS_FRAMES));
    return fnumber;
}

//return the frame rate of the next frame
long VideoProcessor::getFrameRate() {
    //get info of from the capture device
    long frate=static_cast<long>(capture.get(CV_CAP_PROP_FPS));
    return frate;
}

Size VideoProcessor::getFrameSize() {
    //get info of from the capture device
    int w= static_cast<int>(capture.get(CV_CAP_PROP_FRAME_WIDTH));
    int h= static_cast<int>(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    return Size(w,h);
   }
// set the instance of the class that
// implements the FrameProcessor interface
void VideoProcessor::setFrameProcessor(FrameProcessor* frameProcessorPtr){
        // invalidate callback function
        process= 0;
        // this is the frame processor instance
        // that will be called
        frameProcessor= frameProcessorPtr;
        callProcess();
}

// set the callback function that will
// be called for each frame
void VideoProcessor::setFrameProcessor(
                 void (*frameProcessingCallback)(cv::Mat&, cv::Mat&)) {
        // invalidate frame processor class instance
        frameProcessor= 0;
        // this is the frame processor function that
        // will be called
        process= frameProcessingCallback;
        callProcess();
}

// set the output video file
// by default the same parameters than
// input video will be used
bool VideoProcessor::setOutput(const std::string &filename,
            int codec, double framerate,
            bool isColor) {
       outputFile= filename;
       extension.clear();
       if (framerate==0.0)
       framerate= getFrameRate(); // same as input
       char c[4];
       // use same codec as input
       if (codec==0) {
            codec= getCodec(c);
       }
       // Open output video,this can save to mp4
  /*     return writer.open(outputFile, // filename
       0x00000020, // codec to be used
       framerate, // frame rate of the video
       getFrameSize(), // frame size
       isColor); // color video?
   */
       // another format use FOURCC
       return writer.open(outputFile, // filename
       CV_FOURCC('X','V','I','D'), // codec to be used
       framerate, // frame rate of the video
       getFrameSize(), // frame size
       isColor); // color video?
}
// set the output as a series of image files
// extension must be ".jpg", ".bmp" ...
bool VideoProcessor::setOutput(const std::string &filename, // prefix
            const std::string &ext, // image file extension
            int numberOfDigits, // number of digits
            int startIndex) { // start index
        // number of digits must be positive
        if (numberOfDigits<0)
        return false;
        // filenames and their common extension
        outputFile= filename;
        extension= ext;
        // number of digits in the file numbering scheme
        digits= numberOfDigits;
        // start numbering at this index
        currentIndex= startIndex;
        return true;
}


// to write the output frame
// could be: video file or images
void VideoProcessor::writeNextFrame(cv::Mat& frame) {
        if (extension.length()) { // then we write images
                std::stringstream ss;
                // compose the output filename
                ss << outputFile << std::setfill('0')
                << std::setw(digits)
                << currentIndex++ << extension;
                cv::imwrite(ss.str(),frame);
        } else { // then write to video file
       // std::cout <<"write video";
        writer.write(frame);
        }
}



// get the codec of input video
int VideoProcessor::getCodec(char codec[4]) {
        // undefined for vector of images
        if (images.size()!=0) return -1;
        union { // data structure for the 4-char code
            int value;
            char code[4]; } returned;
        // get the code
        returned.value= static_cast<int>(
            capture.get(CV_CAP_PROP_FOURCC));
        // get the 4 characters
        codec[0]= returned.code[0];
        codec[1]= returned.code[1];
        codec[2]= returned.code[2];
        codec[3]= returned.code[3];
        // return the int value corresponding to the code
        return returned.value;
}






