#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;
#include "cv.h"
#include "highgui.h"
#include "sqlite3.h"
#include "math.h"

/* QTCOMMENT
#include <QApplication>
#include <QtGui>
#include <QtCore>
 */

/*
 * Class PointStorage
 * Contains information about each contour:
 * - frame Number
 * - error Flag - if the contour didn't meet the contour filtering requirements
 * - type: differentiates between PUPIL and PURKINJE REFLEX (PR)
 * - May also add the size of the contour, as well as its average pixel values
 *
 */

int frameCounter;

int minFrame;
int maxFrame;

int type;

CvRect rectEye;

FILE * centers;
sqlite3* db;
char* zErrMsg;
int rc;

IplImage* img_Eye_threshold_PR;

/* QTCOMMENT
QGraphicsView *eyeViewTemp;
QLabel *HR_Label;
QLabel *HR_SD_label;
 */

IplImage* grayscale(IplImage* originalEye);
IplImage* adaptiveThreshold(IplImage* input, int block_size, float param1);

vector<CvBox2D*>* findContours(IplImage* originalEye, IplImage* input, int minWidth, int maxWidth, float minEccentricity, float maxEccentricity);

bool minSizeFilterContour(CvSeq* contour, int );
bool maxSizeFilterContour(CvSeq* contour, int );
bool eccentricityFilterContour(CvSeq* contour, float, float );

void updateROI(int center_x, int center_y, int ROI_SIZE);

CvPoint calculateCenter(IplImage* input, CvBox2D* contourBox);

void locatePurkinje(IplImage* img_face);
void locatePupil(IplImage* img_face, float PR_x, float PR_y, float PR_width, float PR_height);
void saveImage(const char*,const char*,int, IplImage*);

static int callback(void *NotUsed, int argc, char **argv, char **azColName);

void ImageAnalysisPR();
void ImageAnalysisPupil();

float getSD(vector<float>* v,float mean);
static void sqlite_command(string sql_cmd);
void filterPRs();
void filterPupils();
double* getHR(int);
double calculateAngleOfDeviation(double* leftEye, double* rightEye);

double** sqlite_query_to_array(string query);
void sqlite_exception(int rc);
int sqlite_get_rows(string);

void getImageStatistics();

/* QTCOMMENT
float* ImageAnalysis(QProgressBar *analysisProgressBar, QGraphicsView *view, QLabel *label, QLabel *SD_label, char eyeVal)
{
 */

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	for(i=0; i<argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int main() {

	zErrMsg = 0;
	sqlite3_stmt *stmt_left = 0;
	sqlite3_stmt *stmt_right = 0;
	sqlite3_stmt *stmt_expanded = 0;

	rc = sqlite3_open("prContours",&db);
	sqlite_exception(rc);

	// create table prContours: stores data from image processing function
	// called in ImageAnalysis()
	sqlite_command("CREATE TABLE IF NOT EXISTS prContours (type TEXT, eye TEXT, frameNumber INTEGER, x REAL, y REAL, w REAL, h REAL)");

	// create table expandedPR: stores distances between PRs that are in the same frame
	sqlite_command("CREATE TABLE IF NOT EXISTS expandedPR (frameNumber INTEGER, x1 REAL, y1 REAL, w1 REAL, h1 REAL, x2 REAL, y2 REAL, w2 REAL, h2 REAL, dist REAL)");

//	sqlite_command("SELECT count(*) FROM prContours WHERE eye = 'RIGHT' AND frameNumber = 81");
//	sqlite_command("SELECT count(*) FROM prContours WHERE eye = 'LEFT' AND frameNumber = 81");
//	sqlite_command("SELECT count(*) FROM (SELECT * FROM prContours WHERE eye = 'RIGHT') INNER JOIN (SELECT * FROM prContours WHERE eye = 'LEFT') USING (frameNumber, type) WHERE frameNumber = 81");

	bool newDB = true;
	if (newDB) {
		ImageAnalysisPR();


		// TEMPORARY DECLARATIONS
		//	minFrame = 20;
		//	maxFrame = 100;

		int rc_left;
		int rc_right;
		char* select_frame_R;
		char* insert_expanded;

		float x1, y1, w1, h1, x2, y2, w2, h2;
		float dist;

		for (int currentFrame=minFrame; currentFrame<=maxFrame; currentFrame++) {
			printf("\nCurrent Frame = %i", currentFrame);
			char* select_frame_L = 0;
			select_frame_L = (char*)calloc(255,1);
			sprintf(select_frame_L, "SELECT * FROM prContours WHERE frameNumber = %i AND eye = 'LEFT' AND type = 'PR'", currentFrame);
			rc_left = sqlite3_prepare_v2(db,select_frame_L,-1,&stmt_left,0);

			// OUTSIDE LOOP: LEFT EYES
			do{
				rc_left = sqlite3_step(stmt_left);
				switch( rc_left ){
				case SQLITE_DONE:
					break;
				case SQLITE_ROW:

					// INSIDE LOOP: RIGHT EYES
					select_frame_R = 0;
					select_frame_R = (char*)calloc(255,1);
					sprintf(select_frame_R, "SELECT * FROM prContours WHERE frameNumber = %i AND eye = 'RIGHT' AND type = 'PR'", currentFrame);
					rc_right = sqlite3_prepare_v2(db,select_frame_R,-1,&stmt_right,0);

					do{
						rc_right = sqlite3_step(stmt_right);
						switch( rc_right ){
						case SQLITE_DONE:
							break;
						case SQLITE_ROW:

							// INSERT CALCULATIONS
							x1 = atof((const char*)sqlite3_column_text(stmt_left,3));
							y1 = atof((const char*)sqlite3_column_text(stmt_left,4));
							w1 = atof((const char*)sqlite3_column_text(stmt_left,5));
							h1 = atof((const char*)sqlite3_column_text(stmt_left,6));

							x2 = atof((const char*)sqlite3_column_text(stmt_right,3));
							y2 = atof((const char*)sqlite3_column_text(stmt_right,4));
							w2 = atof((const char*)sqlite3_column_text(stmt_right,5));
							h2 = atof((const char*)sqlite3_column_text(stmt_right,6));

							dist = sqrt(pow(x2-x1,2)+pow(y2-y1,2));

							insert_expanded = 0;
							insert_expanded = (char*)calloc(255,1);

							sprintf(insert_expanded, "REPLACE into expandedPR (frameNumber, x1, y1, w1, h1, x2, y2, w2, h2, dist) values (%i,%f,%f,%f,%f,%f,%f,%f,%f,%f)",
									currentFrame, x1, y1, w1, h1, x2, y2, w2, h2, dist);

							rc = sqlite3_exec(db,insert_expanded,callback,0,&zErrMsg);

							break;
						default:
							fprintf(stderr, "Error: %d : %s\n",  rc_right, sqlite3_errmsg(db));
							break;
						}
					}while( rc_right==SQLITE_ROW );

					// END INSIDE LOOP FOR RIGHT EYES
					sqlite3_finalize(stmt_right);

					break;
				default:
					fprintf(stderr, "Error: %d : %s\n",  rc_left, sqlite3_errmsg(db));
					break;
				}
			}while( rc_left==SQLITE_ROW );

			sqlite3_finalize(stmt_left);
		}

		filterPRs();

		char* select_expanded = "SELECT * from filteredPR";

		rc = sqlite3_exec(db,select_expanded,callback,&stmt_expanded,&zErrMsg);
		sqlite_exception(rc);

		ImageAnalysisPupil();

		char* create_expanded2 = "CREATE TABLE IF NOT EXISTS expandedPupil (frameNumber INTEGER, x1 REAL, y1 REAL, w1 REAL, h1 REAL, x2 REAL, y2 REAL, w2 REAL, h2 REAL, dist REAL)";

		int rc_expanded2;
		rc_expanded2 = sqlite3_exec(db,create_expanded2,callback,0,&zErrMsg);
		if( rc_expanded2 != SQLITE_OK ){
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			exit(1);
		}

		// VERY RAW CODE:
		for (int currentFrame=minFrame; currentFrame<=maxFrame; currentFrame++) {
			printf("\nCurrent Frame = %i", currentFrame);
			char* select_frame_L = 0;
			select_frame_L = (char*)calloc(255,1);
			sprintf(select_frame_L, "SELECT * FROM prContours WHERE frameNumber = %i AND eye = 'LEFT' AND type = 'PUPIL'", currentFrame);
			rc_left = sqlite3_prepare_v2(db,select_frame_L,-1,&stmt_left,0);

			// OUTSIDE LOOP: LEFT EYES
			do{
				rc_left = sqlite3_step(stmt_left);
				switch( rc_left ){
				case SQLITE_DONE:
					break;
				case SQLITE_ROW:

					// INSIDE LOOP: RIGHT EYES
					select_frame_R = 0;
					select_frame_R = (char*)calloc(255,1);
					sprintf(select_frame_R, "SELECT * FROM prContours WHERE frameNumber = %i AND eye = 'RIGHT' AND type = 'PUPIL'", currentFrame);
					rc_right = sqlite3_prepare_v2(db,select_frame_R,-1,&stmt_right,0);

					do{
						rc_right = sqlite3_step(stmt_right);
						switch( rc_right ){
						case SQLITE_DONE:
							break;
						case SQLITE_ROW:

							// INSERT CALCULATIONS
							x1 = atof((const char*)sqlite3_column_text(stmt_left,3));
							y1 = atof((const char*)sqlite3_column_text(stmt_left,4));
							w1 = atof((const char*)sqlite3_column_text(stmt_left,5));
							h1 = atof((const char*)sqlite3_column_text(stmt_left,6));

							x2 = atof((const char*)sqlite3_column_text(stmt_right,3));
							y2 = atof((const char*)sqlite3_column_text(stmt_right,4));
							w2 = atof((const char*)sqlite3_column_text(stmt_right,5));
							h2 = atof((const char*)sqlite3_column_text(stmt_right,6));

							dist = sqrt(pow(x2-x1,2)+pow(y2-y1,2));

							insert_expanded = 0;
							insert_expanded = (char*)calloc(255,1);

							sprintf(insert_expanded, "REPLACE into expandedPupil (frameNumber, x1, y1, w1, h1, x2, y2, w2, h2, dist) values (%i,%f,%f,%f,%f,%f,%f,%f,%f,%f)",
									currentFrame, x1, y1, w1, h1, x2, y2, w2, h2, dist);
							// printf("\nx1 = %f, y1 = %f, x2 = %f, y2 = %f, dist = %f",x1,y1,x2,y1,dist);

							rc = sqlite3_exec(db,insert_expanded,callback,0,&zErrMsg);

							break;
						default:
							fprintf(stderr, "Error: %d : %s\n",  rc_right, sqlite3_errmsg(db));
							break;
						}
					}while( rc_right==SQLITE_ROW );

					// END INSIDE LOOP FOR RIGHT EYES
					sqlite3_finalize(stmt_right);

					break;
				default:
					fprintf(stderr, "Error: %d : %s\n",  rc_left, sqlite3_errmsg(db));
					break;
				}
			}while( rc_left==SQLITE_ROW );

			sqlite3_finalize(stmt_left);
		}

		filterPupils();
	}

	double* leftEyeData = getHR(0);
	double* rightEyeData = getHR(1);

	double aod = calculateAngleOfDeviation(leftEyeData,rightEyeData);

	getImageStatistics();

	// Clean up Databases

	//	rc_expanded = sqlite3_exec(db,"delete from expandedPR",callback,&stmt_expanded,&zErrMsg);
	//	if( rc_expanded != SQLITE_OK ){
	//		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	//		sqlite3_close(db);
	//		exit(1);
	//	}
	//
	//	rc = sqlite3_exec(db,"drop table expandedPR",callback,&stmt_expanded,&zErrMsg);
	//	if( rc_expanded != SQLITE_OK ){
	//		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	//		sqlite3_close(db);
	//		exit(1);
	//	}

	sqlite3_close(db);

	printf("\nEnd of Program");

	return 0;
}

// output table numbers from the database
void getImageStatistics() {

	printf("Statistical Summary:\n");

	// total frames analyzed
	int nframes = 323;
	printf("total frames : %i\n", nframes );

	double** count;
	// number of paired PRs
	string query = "SELECT count(*) from filteredPR";
	count = sqlite_query_to_array(query);
	printf("percent good PRs %f\n",(count[0][0]/nframes));

	// number of paired pupils
	query = "SELECT count(*) from filteredPupil";
	count = sqlite_query_to_array(query);
	printf("percent good Pupils %f\n",(count[0][0]/nframes));

	// number of good frames used (currently the same as the filtered pupils)
	printf("percent good Pupils%f\n",(count[0][0]/nframes));

}

void ImageAnalysisPR()
{
	// Make new directories needed for this project:
	mkdir("/home/istrab/Output/Right Eyes", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/C1 Grayscale", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/R1 Grayscale", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/C2 Thresholded", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/R2 Thresholded", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/C4 Contours", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/R4 Contours", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/C5 Center of Contours", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir("/home/istrab/Output/R5 Center of Contours", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	// Switch to relevant directory
	const char* inputDir = "/home/istrab/Output/Original Frames";
	chdir(inputDir);

	// Import number of frames from previous program
	int numFrames = 0;
	int x;

	ifstream inFile;
	inFile.open("frameCount.txt");
	if (!inFile) {
		cout << "Unable to open file";
		exit(1); // terminate with error
	}

	centers = fopen ("Center Locations.txt","w");

	while (inFile >> x) {
		numFrames = numFrames + x;
	}

	inFile.close();

	// Run analysis for each frame
	minFrame = 20;
	//maxFrame = 100;
	maxFrame = numFrames;

	type = 1;
	for (frameCounter = minFrame; frameCounter <= maxFrame; frameCounter++) {

		chdir(inputDir);

		// Generate input filename
		char *filename = 0;
		filename = (char*)calloc(255,1);
		sprintf(filename, "Image - Frame #%i.jpg", frameCounter);

		// Import original frame
		IplImage* img_face = cvLoadImage(filename);

		locatePurkinje(img_face);

		//	Release memory allocations
		cvReleaseImage(&img_face);

	}
	fclose(centers);
}


void ImageAnalysisPupil()
{
	// TEMPORARY DECLARATIONS
	//	minFrame = 20;
	//	maxFrame = 99;

	int rc_pupil;
	sqlite3_stmt *stmt_pupil;
	char *filename;
	IplImage* img_face;

	float PR_x_right, PR_y_right, PR_width_right, PR_height_right;
	float PR_x_left, PR_y_left, PR_width_left, PR_height_left;

	type = 0;
	for (frameCounter = minFrame; frameCounter <= maxFrame; frameCounter++) {

		char* select_pupil = 0;
		select_pupil = (char*)calloc(255,1);
		sprintf(select_pupil, "SELECT * FROM filteredPR WHERE frameNumber = %i", frameCounter);
		rc_pupil = sqlite3_prepare_v2(db,select_pupil,-1,&stmt_pupil,0);

		// LOOPING THROUGH ROWS
		do{
			rc_pupil = sqlite3_step(stmt_pupil);
			switch( rc_pupil ){
			case SQLITE_DONE:
				break;
			case SQLITE_ROW:
				/* Perform pupil analysis routine */

				chdir("/home/istrab/Output/Original Frames");

				// Generate input filename
				filename = 0;
				filename = (char*)calloc(255,1);
				sprintf(filename, "Image - Frame #%i.jpg", frameCounter);

				// Import original frame
				img_face = cvLoadImage(filename);

				// RIGHT EYE VARIABLES
				PR_x_right = atof((const char*)sqlite3_column_text(stmt_pupil,1));
				PR_y_right = atof((const char*)sqlite3_column_text(stmt_pupil,2));
				PR_width_right = atof((const char*)sqlite3_column_text(stmt_pupil,3));
				PR_height_right = atof((const char*)sqlite3_column_text(stmt_pupil,4));

				// LEFT EYE VARIABLES
				PR_x_left = atof((const char*)sqlite3_column_text(stmt_pupil,5));
				PR_y_left = atof((const char*)sqlite3_column_text(stmt_pupil,6));
				PR_width_left = atof((const char*)sqlite3_column_text(stmt_pupil,7));
				PR_height_left = atof((const char*)sqlite3_column_text(stmt_pupil,8));

				// RIGHT EYE
//				printf("\nFRAME #: %i, RIGHT EYE",frameCounter);
				locatePupil(img_face, PR_x_right, PR_y_right, PR_width_right, PR_height_right);

				// LEFT EYE
//				printf("\nFRAME #: %i, LEFT EYE",frameCounter);
				locatePupil(img_face, PR_x_left, PR_y_left, PR_width_left, PR_height_left);

				//	Release memory allocations
				cvReleaseImage(&img_face);

				break;
			default:
				fprintf(stderr, "Error: %d : %s\n",  rc_pupil, sqlite3_errmsg(db));
				break;
			}
		}while( rc_pupil==SQLITE_ROW );

	}
}

/*
 * Function LocatePurjinke: finds PRs of the whole image, replacing the Haar Cascade
 *
 *
 */
void locatePurkinje(IplImage* img_face) {

	// GRAYSCALE PURKINJE
	IplImage* img_face_gray = grayscale( img_face);

	// THRESHOLDING PURKINJE
	IplImage* img_face_threshold = adaptiveThreshold(img_face_gray, 399, -15); // changed from 201, -40

	// CONTOURS PURKINJE
	vector<CvBox2D*>::iterator it;
	vector<CvBox2D*>* prContours = findContours(img_face, img_face_threshold, 10, 40, 0.5, 1.8);

	for(it = prContours->begin(); it < prContours->end() ; it++) {
		// PAINT COUNTOUR ON IMAGE
		cvEllipseBox(img_face,**it, CV_RGB(255,255,255), 2, 8, 0);

		// ******************** INSERT INTO SQLITE DATABASE ************************
		char* insert = 0;
		insert = (char*)calloc(255,1);

		if ((*it)->center.x < 960) {
			sprintf(insert, "REPLACE into prContours (type, eye, frameNumber, x, y, w, h) values ('PR','RIGHT',%i,%f,%f,%f,%f)",
					frameCounter,(*it)->center.x, (*it)->center.y, (*it)->size.width, (*it)->size.height);
		}
		else {
			sprintf(insert, "REPLACE into prContours (type, eye, frameNumber, x, y, w, h) values ('PR','LEFT',%i,%f,%f,%f,%f)",
					frameCounter,(*it)->center.x, (*it)->center.y, (*it)->size.width, (*it)->size.height);
		}

		rc = sqlite3_exec(db,insert,callback,0,&zErrMsg);

		sqlite_exception(rc);

		// ********************* PRINT TO TXT FILE **************************
		fprintf(centers, "%f,%f,%f,%f,%i\n",(*it)->center.x, (*it)->center.y, (*it)->size.width, (*it)->size.height, frameCounter);
	}

	if(prContours->size() == 0 ) {
		printf("Frame %i ; found 0 contours\n",frameCounter);
	}

	// Save image
	saveImage("Face HC (Contours) - Frame #%i.jpg", "/home/istrab/Output/C4 Contours",
			type , img_face);

	// Release memory allocations
	cvReleaseImage(&img_face_gray);
	cvReleaseImage(&img_face_threshold);
}

void locatePupil(IplImage* img_face, float PR_x, float PR_y, float PR_width, float PR_height) {

//	printf("\nVALUES: PR_x = %i, PR_y = %i",(int)PR_x,(int)PR_y);

	// SAMPLING PARAMETERS
	int         RESIZE_PUPIL                    =       3;
	int			ROI_SIZE						=		399;

	IplImage* img_Eye = cvCreateImage (cvSize(img_face->width, img_face->height),
			IPL_DEPTH_8U, img_face->nChannels );
	cvCopy( img_face, img_Eye, 0 );

	updateROI((int)PR_x, (int)PR_y, ROI_SIZE);
	cvSetImageROI ( img_Eye , rectEye);

	// Generate input filename
	char *filename_eye = 0;
	filename_eye = (char*)calloc(255,1);
	sprintf(filename_eye, "Right Eye - Frame #%i.jpg", frameCounter);

	// Save image of right eye
	chdir("/home/istrab/Output/Right Eyes");
	cvSaveImage(filename_eye, img_Eye);

	img_Eye->width = ROI_SIZE;
	img_Eye->height = ROI_SIZE;

//	printf("DIAGNOSTIC: img_face width: %i, img_face height: %i", img_Eye->width, img_Eye->height);
	// GRAYSCALE PUPIL
	IplImage* img_Eye_gray = grayscale( img_Eye);

	// DOWNSAMPLE IMAGE
	IplImage* img_Eye_resized = cvCreateImage (cvSize(img_Eye_gray->width/RESIZE_PUPIL, img_Eye_gray->height/RESIZE_PUPIL), IPL_DEPTH_8U, img_Eye_gray->nChannels);
	cvResize( img_Eye_gray, img_Eye_resized, CV_INTER_LINEAR );

	int counter = 0;
	float min[3], max[3], avgPixel[3];//, max[3], avgPixel[3];
	min[0] = -1; min[1] = -1; min[2] = -1;
	max[0] = -1; max[1] = -1; max[2] = -1;
	int n, startX, startY;

	// PASS #1: CENTER SAMPLING BOX
	startX = ( (ROI_SIZE / 2) - 1.5*PR_width ) / RESIZE_PUPIL;
	startY = ( (ROI_SIZE / 2) - 1.5*PR_height ) / RESIZE_PUPIL;
	n=0;

	for (int i=startX; i<startX + (3 * PR_width / RESIZE_PUPIL); i++) {
		for (int j=startY; j<startY + (3 * PR_height / RESIZE_PUPIL); j++) {
			if ( (i < startX + (1 * PR_width / RESIZE_PUPIL) || i> startX + (2 * PR_width / RESIZE_PUPIL))
					&& (j < startY + (1 * PR_height / RESIZE_PUPIL) || j > startY + 2 * PR_height / RESIZE_PUPIL) ) { // Excludes PR values
				CvScalar s;
				s=cvGet2D(img_Eye_resized,i,j); // get the (i,j) pixel value
				avgPixel[n] = avgPixel[n] + s.val[0];
				if (s.val[0] < min[n] || min[n] == -1) {
					min[n] = s.val[0];
				}
				if (s.val[0] > max[n] || max[n] == -1) {
					max[n] = s.val[0];
				}
				counter++;
			}
		}
	}
	avgPixel[n] = avgPixel[n] / counter;

//	printf("\nAverage pixel value for pupil: %f",avgPixel[n]);
//	printf("\nMin value = %f, Max value = %f\n", min[n], max[n]);

	counter = 0;

	// PASS #2: RIGHT SAMPLING BOX
	startX = ( (ROI_SIZE / 2) + 0.5*PR_width ) / RESIZE_PUPIL;
	startY = ( (ROI_SIZE / 2) - 1.0*PR_height ) / RESIZE_PUPIL;
	n=1;

	for (int i=startX; i<startX + (2 * PR_width / RESIZE_PUPIL); i++) {
		for (int j=startY; j<startY + (2 * PR_height / RESIZE_PUPIL); j++) {
			CvScalar s;
			s=cvGet2D(img_Eye_resized,i,j); // get the (i,j) pixel value
			avgPixel[n] = avgPixel[n] + s.val[0];
			if (s.val[0] < min[n] || min[n] == -1) {
				min[n] = s.val[0];
			}
			if (s.val[0] > max[n] || max[n] == -1) {
				max[n] = s.val[0];
			}
			counter++;
		}
	}
	avgPixel[n] = avgPixel[n] / counter;

//	printf("\nAverage pixel value for pupil: %f",avgPixel[n]);
//	printf("\nMin value = %f, Max value = %f\n", min[n], max[n]);

	counter = 0;

	// PASS #3: LEFT SAMPLING BOX
	startX = ( (ROI_SIZE / 2) - 2.5*PR_width ) / RESIZE_PUPIL;
	startY = ( (ROI_SIZE / 2) - 1.0*PR_height ) / RESIZE_PUPIL;
	n=2;

	for (int i=startX; i<startX + (2 * PR_width / RESIZE_PUPIL); i++) {
		for (int j=startY; j<startY + (2 * PR_height / RESIZE_PUPIL); j++) {
			CvScalar s;
			s=cvGet2D(img_Eye_resized,i,j); // get the (i,j) pixel value
			avgPixel[n] = avgPixel[n] + s.val[0];
			if (s.val[0] < min[n] || min[n] == -1) {
				min[n] = s.val[0];
			}
			if (s.val[0] > max[n] || max[n] == -1) {
				max[n] = s.val[0];
			}
			counter++;
		}
	}
	avgPixel[n] = avgPixel[n] / counter;

//	printf("\nAverage pixel value for pupil: %f",avgPixel[n]);
//	printf("\nMin value = %f, Max value = %f\n", min[n], max[n]);

	// CHOOSING CORRECT SAMPLING BOX
	n=0;
	if (max[1] > max[0]) {
		n=1;
	}
	if (max[2] > max[n]) {
		n=2;
	}

	// NEW CODE: THRESHOLDING PUPIL
	for (int i=0; i<img_Eye_resized->width; i++) {
		for (int j=0; j<img_Eye_resized->height; j++) {
			CvScalar s;

			s=cvGet2D(img_Eye_resized,i,j); // get the (i,j) pixel value
			if (s.val[0] >= (avgPixel[n] + min[n]) / 2) {
				s.val[0]=255;
				cvSet2D(img_Eye_resized,i,j,s);
			}
			else {
				s.val[0]=0;
				cvSet2D(img_Eye_resized,i,j,s);
			}
		}
	}
//	printf("\nDone Thresholding");

	/* Noise Reduction for Pupil */
	cvMorphologyEx( img_Eye_resized, img_Eye_resized, 0, 0, CV_MOP_OPEN, 0 );

	// RESIZE THRESHOLDED IMAGE
	IplImage* img_Eye_threshold = cvCreateImage (cvSize(img_Eye->width, img_Eye->height),
			IPL_DEPTH_8U, img_Eye_gray->nChannels );

	cvResize(img_Eye_resized, img_Eye_threshold, CV_INTER_LINEAR);

	saveImage("Right Eye (Thresholded) - Frame #%i.jpg","/home/istrab/Output/%c2 Thresholded",
			type, img_Eye_threshold);

	// NEW CODE: CONTOURS PUPIL
	int 	MIN_SIZE_PUPIL 			=	75;
	int 	MAX_SIZE_PUPIL 			=	150;
	float 	MIN_ECCENTRICITY_PUPIL  	=	0.8;
	float 	MAX_ECCENTRICITY_PUPIL  	=	1.2;

	// FIND CONTOURS
	vector<CvBox2D*>::iterator it;
	vector<CvBox2D*>* pupilContours = findContours(img_Eye, img_Eye_threshold, MIN_SIZE_PUPIL, MAX_SIZE_PUPIL, MIN_ECCENTRICITY_PUPIL, MAX_ECCENTRICITY_PUPIL);

	for(it = pupilContours->begin(); it < pupilContours->end() ; it++) {
		float dist = sqrt(pow((*it)->center.x - 200,2) + pow((*it)->center.y - 200,2));
		if (dist < MAX_SIZE_PUPIL / 2) {
			// PAINT COUNTOUR ON IMAGE
			cvEllipseBox(img_Eye,**it, CV_RGB(255,255,255), 2, 8, 0);

			// ******************** INSERT INTO SQLITE DATABASE ************************
			char* insert = 0;
			insert = (char*)calloc(255,1);

//			printf("\nValues: center.x = %f, center.y = %f",((*it)->center.x - 200 + (int)PR_x), ((*it)->center.y - 200 + (int)PR_y));

			if (PR_x < 960) {
				sprintf(insert, "REPLACE into prContours (type, eye, frameNumber, x, y, w, h) values ('PUPIL','RIGHT',%i,%f,%f,%f,%f)",
						frameCounter,((*it)->center.x - 200 + (int)PR_x), ((*it)->center.y - 200 + (int)PR_y), (*it)->size.width, (*it)->size.height);
			}
			else {
				sprintf(insert, "REPLACE into prContours (type, eye, frameNumber, x, y, w, h) values ('PUPIL','LEFT',%i,%f,%f,%f,%f)",
						frameCounter,((*it)->center.x - 200 + (int)PR_x), ((*it)->center.y - 200 + (int)PR_y), (*it)->size.width, (*it)->size.height);
			}

			rc = sqlite3_exec(db,insert,callback,0,&zErrMsg);

			sqlite_exception(rc);
		}
		// ********************* PRINT TO TXT FILE **************************
		//fprintf(centers, "%f,%f,%f,%f,%i\n",(*it)->center.x, (*it)->center.y, (*it)->size.width, (*it)->size.height, frameCounter);
	}

	if(pupilContours->size() == 0 ) {
		printf("Frame %i ; found 0 contours\n",frameCounter);
	}

	// Save image
	saveImage("Right Eye (Contours) - Frame #%i.jpg", "/home/istrab/Output/C4 Contours",
			type , img_Eye);

	// FIND CENTER OF PUPIL

	cvReleaseImage(&img_Eye);
	cvReleaseImage(&img_Eye_gray);
	cvReleaseImage(&img_Eye_resized);
	cvReleaseImage(&img_Eye_threshold);
}

/*
 * Method updateROI
 * inputs: potential new centers
 * - filters in case a new ROI is bad
 */
void updateROI(int center_x, int center_y, int ROI_SIZE) {

	if ( (center_x > (ROI_SIZE / 2) ) && (center_y > (ROI_SIZE / 2) ) ) {
		rectEye.x = center_x - ROI_SIZE / 2;
		rectEye.y = center_y - ROI_SIZE / 2;
	}

	else {
		rectEye.x = 0;
		rectEye.y = 0;
	}

	rectEye.width = ROI_SIZE;
	rectEye.height = ROI_SIZE;

	return;
}

vector<CvBox2D*>* findContours(IplImage* originalEye, IplImage* input, int minWidth, int maxWidth, float minEccentricity, float maxEccentricity) {
	/* Contours for Pupil Detection */
	CvMemStorage* contour_storage = cvCreateMemStorage(0);

	CvSeq* contours;
	cvFindContours(
			input,
			contour_storage,
			&contours
	);

	vector<CvSeq*>* filtered = new vector<CvSeq*>();
	vector<CvSeq*>::iterator it;

	while(contours != NULL) {
		// number of points at this contour
		int iNumPoints = contours->total;

		// filter the contour
		// Continue if contour does not pass the filter
		if( 	minSizeFilterContour( contours, minWidth) &&
				maxSizeFilterContour( contours, maxWidth) &&
				eccentricityFilterContour( contours, minEccentricity, maxEccentricity) )
		{
			filtered->push_back(contours);

			CvPoint *PointArray;
			PointArray = new CvPoint[iNumPoints];
			cvCvtSeqToArray(contours, PointArray, CV_WHOLE_SEQ);

			// free memory
			delete PointArray;
		}

		// next contour
		contours = contours->h_next;
	}

	cvZero(input);

	vector<CvBox2D*>* contourBox = new vector<CvBox2D*>();

	for(it = filtered->begin(); it < filtered->end() ; it++) {
		CvBox2D* ellipse = new CvBox2D;
		*ellipse = cvFitEllipse2(*it) ;
		contourBox->push_back( ellipse );
	}

	// cleanup
	cvReleaseMemStorage(&contour_storage);

	// return vector of contours
	return contourBox;
}


IplImage* grayscale(IplImage* originalEye) {
	/* Grayscale of image for Pupil Detection */
	IplImage* eyeGray = cvCreateImage (cvSize(originalEye->width, originalEye->height),
			IPL_DEPTH_8U, 1 );

	cvCvtColor(
			originalEye,
			eyeGray,
			CV_RGB2GRAY
	);

	// Save image
	saveImage("Right Eye (Grayscale) - Frame #%i.jpg","/home/istrab/Output/C1 Grayscale",
			type,eyeGray);

	return eyeGray;
}

IplImage* adaptiveThreshold(IplImage* input, int block_size, float param1) {
	/* Thresholding for Pupil Detection */
	IplImage* eyeThreshold = cvCreateImage (cvSize(input->width, input->height),
			IPL_DEPTH_8U, input->nChannels );
	cvCopy (input, eyeThreshold, 0);

	cvAdaptiveThreshold(
			input,
			eyeThreshold,
			255,
			CV_ADAPTIVE_THRESH_MEAN_C,
			CV_THRESH_BINARY,
			block_size,
			param1
	);

	// Save image
	saveImage("Right Eye (Thresholded) - Frame #%i.jpg","/home/istrab/Output/R2 Thresholded",
			type, eyeThreshold);

	return eyeThreshold;
}

CvPoint calculateCenter(IplImage* input, CvBox2D* contourBox) {
	// Calculate Center of Pupil
	float center_x;
	float center_y;

	// store good eyes
	if (contourBox) {
		if(true) {
			center_x = contourBox->center.x;
			center_y = contourBox->center.y;
		}
		else {
			center_x = rectEye.x + contourBox->center.x;
			center_y = rectEye.y + contourBox->center.y;
		}
	}
	else {
		center_x = -1;
		center_y = -1;
	}

	CvPoint center;
	center.x = center_x;
	center.y = center_y;

	cvCircle(input,center,1,cvScalarAll(255));

	saveImage("Right Eye (Center) - Frame #%i.jpg", "/home/istrab/Output/%c5 Center of Contours",
			type,input);

	return center;
}


void saveImage(const char* fName, const char* dName, int type, IplImage* image) {
	char object = 'D';
	if(type == 0){
		object = 'C';
	}
	else if(type == 1){
		object = 'R';
	}

	// Save image
	char *filename = 0;
	filename = (char*)calloc(255,1);
	sprintf(filename, fName, frameCounter);

	char *directory = 0;
	directory = (char*)calloc(255,1);
	sprintf(directory, dName, object);

	chdir(directory);
	cvSaveImage(filename, image);
	free(filename);

	return;
}


bool minSizeFilterContour(CvSeq* contour, int minWidth)
{

	bool passesFilter = true;

	// get bounding rect of this contour
	CvRect rect;
	rect = cvBoundingRect( contour, 0 );

	// MINIMUM SIZE CHECK
	if ( (rect.width < minWidth) || (rect.height < minWidth) )
	{
		passesFilter = false;
	}

	return passesFilter;

}

bool maxSizeFilterContour(CvSeq* contour, int maxWidth)
{

	bool passesFilter = true;

	// get bounding rect of this contour
	CvRect rect;
	rect = cvBoundingRect( contour, 0 );

	// MAXIMUM SIZE CHECK
	if ( (rect.width > maxWidth) || (rect.height > maxWidth))
	{
		passesFilter = false;
	}

	return passesFilter;

}

bool eccentricityFilterContour(CvSeq* contour, float minEccentricity, float maxEccentricity)
{
	bool passesFilter = true;

	// get bounding rect of this contour
	CvRect rect;
	rect = cvBoundingRect( contour, 0 );

	// ECCENTRICITY CHECK
	if (((float)rect.width/(float)rect.height > maxEccentricity) || ((float)rect.width/(float)rect.height < minEccentricity)) {
		passesFilter = false;
	}

	return passesFilter;

}

float getSD(vector<float>* v,float mean)
{
	vector<float>::iterator it;
	float sumSquares = 0;
	float sd = 0;

	for( it = v->begin(); it < v->end(); it++ )
	{
		sumSquares += pow((*it-mean),2);
	}

	sd = sqrt(sumSquares/v->size());

	return sd;

}

// auxiliary function to execute command in one line
// catches sqlite exceptions
// assumes global variables exist:
// db: pointer to sqlite3 database
// rc integer return status of sqlite statement
// callback: static integer (unknown purpose)
static void sqlite_command(string sql_cmd)
{
	zErrMsg = 0;
	rc = sqlite3_exec(db,sql_cmd.c_str(),callback,0,&zErrMsg);

	sqlite_exception(rc);
}

void filterPRs() {

	// to store data
	vector<float>* d = new vector<float>();

	// SQLITE 3 variables
	sqlite3_stmt *stmt;
	const char *tail;

	string query = "SELECT * FROM expandedPR";

	rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

	// create exception if error occurs
	sqlite_exception(rc);

	float dist;
	// get all rows
	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		// get distance from db; convert to float
		dist = atof((const char*)sqlite3_column_text(stmt,9));
		// print dist in console
		// printf("%f\n",dist);
		// add dist to vector d
		d->push_back(dist);
	}

	// GET MEAN
	query = "SELECT avg(dist) FROM expandedPR";

	rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

	// create exception if error occurs
	sqlite_exception(rc);

	float mean;
	// get all rows
	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		mean = atof((const char*)sqlite3_column_text(stmt,0));
		printf("Mean: %s\n",sqlite3_column_text(stmt,0));
	}

	sqlite3_finalize(stmt);

	// calculate standard deviation of distance vector 'd'
	float sd = getSD(d,mean);
	printf("SD: %f\n",sd);

	// set min and max values for distances
	float max = mean + sd;
	float min = mean - sd;

	// create a table of filtered PRs within the given range
	char* filter = 0;
	filter = (char*)calloc(255,1);
	sprintf(filter,"CREATE TABLE IF NOT EXISTS filteredPR AS SELECT * FROM expandedPR WHERE dist BETWEEN %f AND %f",min,max);
	printf(filter);
	sqlite_command(filter);

	while (sd > 20) {
		char* delete_rows = 0;
		delete_rows = (char*)calloc(255,1);
		sprintf(delete_rows, "DELETE FROM filteredPR WHERE dist < %f OR dist > %f", min, max );

		rc = sqlite3_exec(db,delete_rows,callback,&stmt,&zErrMsg);

		sqlite3_finalize(stmt);
		free(d);
		vector<float>* d = new vector<float>();

		string query = "SELECT * FROM filteredPR";
		rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

		// get all rows
		while ( sqlite3_step(stmt) == SQLITE_ROW ) {
			// get distance from db; convert to float
			dist = atof((const char*)sqlite3_column_text(stmt,9));
			// add dist to vector d
			d->push_back(dist);
		}

		// GET MEAN
		query = "SELECT avg(dist) FROM filteredPR";

		rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

		// create exception if error occurs
		sqlite_exception(rc);

		// get all rows
		while ( sqlite3_step(stmt) == SQLITE_ROW ) {
			mean = atof((const char*)sqlite3_column_text(stmt,0));
			printf("Mean: %s\n",sqlite3_column_text(stmt,0));
		}

		sqlite3_finalize(stmt);

		// calculate standard deviation of distance vector 'd'
		sd = getSD(d,mean);
		printf("SD: %f\n",sd);

		max = mean + sd;
		min = mean - sd;
	}

	sqlite3_stmt *stmt_filtered;
	sqlite3_stmt *stmt_ordered;
	int counter, counter_ordered;
	int rc_ordered;

	//    int minFrame = 20;
	//    int maxFrame = 99;
	for (int currentFrame=minFrame; currentFrame<=maxFrame; currentFrame++) {

		char* select_frame = 0;
		select_frame = (char*)calloc(255,1);
		sprintf(select_frame, "SELECT * FROM filteredPR WHERE frameNumber = %i", currentFrame);
		rc = sqlite3_prepare_v2(db,select_frame,-1,&stmt_filtered,0);
		counter = 0;

		// LOOPING THROUGH ROWS
		do{
			rc = sqlite3_step(stmt_filtered);
			switch( rc ){
			case SQLITE_DONE:
				break;
			case SQLITE_ROW:
				counter++;
				break;
			default:
				fprintf(stderr, "Error: %d : %s\n",  rc, sqlite3_errmsg(db));
				break;
			}
		}while( rc==SQLITE_ROW );

		printf("\nCounter = %i", counter);

		if (counter > 1) {
			printf("\n Multiple PR contours found in frame %i", currentFrame);

			// DO ALTERNATIVE ROUTINE TO NARROW DOWN TO CLOSEST
			char* select_order = 0;
			select_order = (char*)calloc(255,1);
			sprintf(select_order, "SELECT * FROM filteredPR WHERE frameNumber = %i ORDER BY abs(dist-%f) ASC", currentFrame, mean);
			rc = sqlite3_prepare_v2(db,select_order,-1,&stmt_ordered,0);

			counter_ordered = 0;

			// LOOP THROUGH ROWS FOR FRAMES THAT HAVE > 1 PR CANDIDATE LEFT
			do{
				rc_ordered = sqlite3_step(stmt_ordered);
				switch( rc_ordered ){
				case SQLITE_DONE:
					break;
				case SQLITE_ROW:

					// DELETE OTHER ROWS
					// ASSUMPTION: The one closest to the mean is the correct one (not necessarily true...)
					// FUTURE IMPROVEMENTS: Add a temporal/spatial filter as well
					if (counter_ordered > 0) {

						const char* dist_text = (const char*)sqlite3_column_text(stmt_ordered,9);
						char* delete_rows = 0;
						delete_rows = (char*)calloc(255,1);
						sprintf(delete_rows, "DELETE FROM filteredPR WHERE frameNumber = '%i' AND dist = '%s'", currentFrame, dist_text);

						rc = sqlite3_exec(db,delete_rows,callback,&stmt_ordered,&zErrMsg);
					}
					counter_ordered++;
					break;
				default:
					fprintf(stderr, "Error: %d : %s\n",  rc_ordered, sqlite3_errmsg(db));
					break;
				}
			}while( rc_ordered==SQLITE_ROW );

			sqlite3_finalize(stmt_ordered);
		}

		else if (counter == 0) {
			printf("\n No PR contours found in frame %i", currentFrame);
		}

		sqlite3_finalize(stmt_filtered);
	}

	// display the filtered table of PRs within the given range
	//sqlite_command("SELECT count(*) FROM filteredPR");
	//sqlite_command("SELECT frameNumber , dist FROM filteredPR");

	return;
}

void filterPupils() {
	// to store data
	vector<float>* d = new vector<float>();

	// SQLITE 3 variables
	sqlite3_stmt *stmt;
	const char *tail;

	string query = "SELECT * FROM expandedPupil";

	rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

	// create exception if error occurs
	sqlite_exception(rc);

	float dist;
	// get all rows
	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		// get distance from db; convert to float
		dist = atof((const char*)sqlite3_column_text(stmt,9));
		// print dist in console
		// printf("%f\n",dist);
		// add dist to vector d
		d->push_back(dist);
	}

	// GET MEAN
	query = "SELECT avg(dist) FROM expandedPR";

	rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

	// create exception if error occurs
	sqlite_exception(rc);

	float mean;
	// get all rows
	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		mean = atof((const char*)sqlite3_column_text(stmt,0));
		printf("Mean: %s\n",sqlite3_column_text(stmt,0));
	}

	sqlite3_finalize(stmt);

	// calculate standard deviation of distance vector 'd'
	float sd = getSD(d,mean);
	printf("SD: %f\n",sd);

	// set min and max values for distances
	float max = mean + sd;
	float min = mean - sd;

	// create a table of filtered PRs within the given range
	char* filter = 0;
	filter = (char*)calloc(255,1);
	sprintf(filter,"CREATE TABLE IF NOT EXISTS filteredPupil AS SELECT * FROM expandedPupil WHERE dist BETWEEN %f AND %f",min,max);
	printf(filter);
	sqlite_command(filter);

	while (sd > 20) {
		char* delete_rows = 0;
		delete_rows = (char*)calloc(255,1);
		sprintf(delete_rows, "DELETE FROM filteredPupil WHERE dist < %f OR dist > %f", min, max );

		rc = sqlite3_exec(db,delete_rows,callback,&stmt,&zErrMsg);

		sqlite3_finalize(stmt);
		free(d);
		vector<float>* d = new vector<float>();

		string query = "SELECT * FROM filteredPupil";
		rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

		// get all rows
		while ( sqlite3_step(stmt) == SQLITE_ROW ) {
			// get distance from db; convert to float
			dist = atof((const char*)sqlite3_column_text(stmt,9));
			// add dist to vector d
			d->push_back(dist);
		}

		// GET MEAN
		query = "SELECT avg(dist) FROM filteredPupil";

		rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

		// create exception if error occurs
		sqlite_exception(rc);

		// get all rows
		while ( sqlite3_step(stmt) == SQLITE_ROW ) {
			mean = atof((const char*)sqlite3_column_text(stmt,0));
			printf("Mean: %s\n",sqlite3_column_text(stmt,0));
		}

		sqlite3_finalize(stmt);

		// calculate standard deviation of distance vector 'd'
		sd = getSD(d,mean);
		printf("SD: %f\n",sd);

		max = mean + 3*sd;
		min = mean - 3*sd;
	}

	sqlite3_stmt *stmt_filtered;
	sqlite3_stmt *stmt_ordered;
	int counter, counter_ordered;
	int rc_ordered;

	//    int minFrame = 20;
	//    int maxFrame = 99;
	for (int currentFrame=minFrame; currentFrame<=maxFrame; currentFrame++) {

		char* select_frame = 0;
		select_frame = (char*)calloc(255,1);
		sprintf(select_frame, "SELECT * FROM filteredPupil WHERE frameNumber = %i", currentFrame);
		rc = sqlite3_prepare_v2(db,select_frame,-1,&stmt_filtered,0);
		counter = 0;

		// LOOPING THROUGH ROWS
		do{
			rc = sqlite3_step(stmt_filtered);
			switch( rc ){
			case SQLITE_DONE:
				break;
			case SQLITE_ROW:
				counter++;
				break;
			default:
				fprintf(stderr, "Error: %d : %s\n",  rc, sqlite3_errmsg(db));
				break;
			}
		}while( rc==SQLITE_ROW );

		printf("\nCounter = %i", counter);

		if (counter > 1) {
			printf("\n Multiple Pupil contours found in frame %i", currentFrame);

			// DO ALTERNATIVE ROUTINE TO NARROW DOWN TO CLOSEST
			char* select_order = 0;
			select_order = (char*)calloc(255,1);
			sprintf(select_order, "SELECT * FROM filteredPupil WHERE frameNumber = %i ORDER BY abs(dist-%f) ASC", currentFrame, mean);
			rc = sqlite3_prepare_v2(db,select_order,-1,&stmt_ordered,0);

			counter_ordered = 0;

			// LOOP THROUGH ROWS FOR FRAMES THAT HAVE > 1 PUPIL CANDIDATE LEFT
			do{
				rc_ordered = sqlite3_step(stmt_ordered);
				switch( rc_ordered ){
				case SQLITE_DONE:
					break;
				case SQLITE_ROW:

					// DELETE OTHER ROWS
					// ASSUMPTION: The one closest to the mean is the correct one (not necessarily true...)
					// FUTURE IMPROVEMENTS: Add a temporal/spatial filter as well
					if (counter_ordered > 0) {

						const char* dist_text = (const char*)sqlite3_column_text(stmt_ordered,9);
						char* delete_rows = 0;
						delete_rows = (char*)calloc(255,1);
						sprintf(delete_rows, "DELETE FROM filteredPupil WHERE frameNumber = '%i' AND dist = '%s'", currentFrame, dist_text);

						rc = sqlite3_exec(db,delete_rows,callback,&stmt_ordered,&zErrMsg);
					}
					counter_ordered++;
					break;
				default:
					fprintf(stderr, "Error: %d : %s\n",  rc_ordered, sqlite3_errmsg(db));
					break;
				}
			}while( rc_ordered==SQLITE_ROW );

			sqlite3_finalize(stmt_ordered);
		}

		else if (counter == 0) {
			printf("\n No Pupil contours found in frame %i", currentFrame);
		}

		sqlite3_finalize(stmt_filtered);
	}
	return;
}

// returns the Hirschberg ratio of the data
// input: eye ( 0 = left 1 = right)
double* getHR(int eye) {

	// SQLITE 3 variables
	sqlite3_stmt *stmt;
	const char *tail;

	// create a table of distances between pupils and purkinje reflexes
	//sqlite_command("CREATE TABLE IF NOT EXISTS eyeDiff as SELECT (( (filteredPR.x1-filteredPupil.x1)*(filteredPR.x1-filteredPupil.x1) + (filteredPR.y1-filteredPupil.y1)*(filteredPR.y1-filteredPupil.y1) AS distSqr, filteredPR.frameNumber AS frameNumber ) FROM filteredPR, filteredPupil WHERE filteredPR.frameNumber = filteredPupil.frameNumber)");
	if(eye == 0){
		sqlite_command("CREATE TABLE IF NOT EXISTS eyeDiff as SELECT ( (filteredPR.x1-filteredPupil.x1)*(filteredPR.x1-filteredPupil.x1) + (filteredPR.y1-filteredPupil.y1)*(filteredPR.y1-filteredPupil.y1)) AS distSqr FROM filteredPR, filteredPupil WHERE filteredPR.frameNumber = filteredPupil.frameNumber AND filteredPR.x1 >= filteredPupil.x1");
		sqlite_command("REPLACE INTO eyeDiff (distSqr) SELECT ( -((filteredPR.x1-filteredPupil.x1)*(filteredPR.x1-filteredPupil.x1) + (filteredPR.y1-filteredPupil.y1)*(filteredPR.y1-filteredPupil.y1)) ) AS distSqr FROM filteredPR, filteredPupil WHERE filteredPR.frameNumber = filteredPupil.frameNumber AND filteredPR.x1 < filteredPupil.x1");
	}
	else if( eye == 1) {
		sqlite_command("CREATE TABLE IF NOT EXISTS eyeDiff as SELECT ( (filteredPR.x2-filteredPupil.x2)*(filteredPR.x2-filteredPupil.x2) + (filteredPR.y2-filteredPupil.y2)*(filteredPR.y2-filteredPupil.y2)) AS distSqr FROM filteredPR, filteredPupil WHERE filteredPR.frameNumber = filteredPupil.frameNumber AND filteredPR.x2 >= filteredPupil.x2");
		sqlite_command("REPLACE INTO eyeDiff (distSqr) SELECT ( -((filteredPR.x2-filteredPupil.x2)*(filteredPR.x2-filteredPupil.x2) + (filteredPR.y2-filteredPupil.y2)*(filteredPR.y2-filteredPupil.y2)) ) AS distSqr FROM filteredPR, filteredPupil WHERE filteredPR.frameNumber = filteredPupil.frameNumber AND filteredPR.x2 < filteredPupil.x2");
	}
	else {
		printf("bad input for eye. Must set eye = 0 for left eye and eye = 1 for right eye\n");
		return NULL;
	}



	// GET MAX
	string query = "SELECT max(distSqr) FROM eyeDiff";

	rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

	// create exception if error occurs
	sqlite_exception(rc);

	float max;
	// get all rows
	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		max = atof((const char*)sqlite3_column_text(stmt,0));
		printf("Max: %s\n",sqlite3_column_text(stmt,0));
	}

	// GET MIN
	query = "SELECT min(distSqr) FROM eyeDiff";

	rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

	// create exception if error occurs
	sqlite_exception(rc);

	float min;
	// get all rows
	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		min = atof((const char*)sqlite3_column_text(stmt,0));
		printf("Min: %s\n",sqlite3_column_text(stmt,0));
	}

	printf("\n DIAGNOSTIC: max = %f, min = %f\n",max,min);

	// select from both tables all frames where PR and PUPIL distances both exist
	// and they are between a certain value
	// add these values to a vector
	char* cQuery = 0;
	cQuery = (char*)calloc(255,1);
	sprintf(cQuery,"SELECT avg(distSqr) FROM eyeDiff WHERE distSqr BETWEEN %f AND %f",min,min+(max-min)/3);

	double mean1 = 0.0;
	double** mean1sqr_array = sqlite_query_to_array(cQuery);
	// get mean value
	double mean1Sqr = mean1sqr_array[0][0];
	if(mean1Sqr < 0.0 ) {
		mean1 = -sqrt(abs(mean1Sqr));
	}
	else if(mean1Sqr > 0.0){
		mean1 = sqrt(abs(mean1Sqr));
	}
	printf("Mean #1: %f\n",mean1);

	// select from both tables all frames where PR and PUPIL distances both exist
	// and they are between a certain value
	// add these values to a vector
	cQuery = 0;
	cQuery = (char*)calloc(255,1);
	sprintf(cQuery,"SELECT avg(distSqr) FROM eyeDiff WHERE distSqr BETWEEN %f AND %f",min+(max-min)/3,min+2*(max-min)/3);

	double mean2 = 0.0;
	double** mean2sqr_array = sqlite_query_to_array(cQuery);
	// get mean value
	double mean2Sqr = mean2sqr_array[0][0];
	if(mean2Sqr < 0.0 ) {
		mean2 = -sqrt(abs(mean2Sqr));
	}
	else if(mean2Sqr > 0.0){
		mean2 = sqrt(abs(mean2Sqr));
	}
	printf("Mean #2: %f\n",mean2);

	// select from both tables all frames where PR and PUPIL distances both exist
	// and they are between a certain value
	// add these values to a vector
	cQuery = 0;
	cQuery = (char*)calloc(255,1);
	sprintf(cQuery,"SELECT avg(distSqr) FROM eyeDiff WHERE distSqr BETWEEN %f AND %f",min+2*(max-min)/3,max);

	double mean3 = 0.0;
	double** mean3sqr_array = sqlite_query_to_array(cQuery);
	// get mean value
	double mean3Sqr = mean3sqr_array[0][0];
	if(mean3Sqr < 0.0 ) {
		mean3 = -sqrt(abs(mean3Sqr));
	}
	else if(mean3Sqr > 0.0){
		mean3 = sqrt(abs(mean3Sqr));
	}
	printf("Mean #3: %f\n",mean3);

	sqlite3_finalize(stmt);

	sqlite_command("DELETE FROM eyeDiff");
	sqlite_command("DROP TABLE eyeDiff");

	// calculate HR From mean values
	const double PIXELS = 1920;     					// pixels
	const double WIDTH = 112.5; 						// mm
	const double ANGLE = 30;        					// degrees
	const double PRISM_DIOPTER_PER_DEGREE = 21/12.5;	// pd/degree

	int meanCount = 0;
	double* meanArray = new double[3];

	if(!(mean1==0.0)){
		meanArray[meanCount] = mean1;
		meanCount++;
	}
	if(!(mean2==0.0)){
		meanArray[meanCount] = mean2;
		meanCount++;
	}
	if(!(mean3==0.0)){
		meanArray[meanCount] = mean3;
		meanCount++;
	}

	double HR1 = 0;
	double HR2 = 0;
	double HRTotal = 0;
	// Need at least 2 means to calculate HR
	if(meanCount == 2) {
		printf("Found only 2 means\n");
		HR1 = abs(ANGLE*PRISM_DIOPTER_PER_DEGREE / (WIDTH/PIXELS * (meanArray[1]-meanArray[0])));
		// determine whether the 1 of the means is from the center target
		// if not, double the calculated HR above (since the ANGLE doubles)
		int diffMeans = abs(meanArray[0] - meanArray[1]);
		// chose 40 pixels as a heuristic/kludge; should really be a function of frame size
		if( diffMeans > 40 ) {
			printf("Determined that means were not from center target. Doubling HR\n");
			HR1 *= 2;
		}
		HRTotal = HR1;
		printf("HR Calculations: HR1 %f HRTotal %f\n",HR1,HRTotal);
	}
	// Case: all 3 targets have data
	// this implies that the center target is meanArray[1]
	// since its mean is from the center third of all distSqr values
	// and that ANGLE = 30 degrees for both HR calculations
	else if(meanCount == 3) {
		printf("Found 3 means\n");
		HR1 = abs(ANGLE*PRISM_DIOPTER_PER_DEGREE / (WIDTH/PIXELS * (meanArray[1]-meanArray[0])));
		HR2 = abs(ANGLE*PRISM_DIOPTER_PER_DEGREE / (WIDTH/PIXELS * (meanArray[2]-meanArray[1])));
		HRTotal = (HR1 + HR2) / 2;
		printf("HR Calculations: HR1 %f HR2 %f HRTotal %f\n",HR1,HR2,HRTotal);
	}
	// could not find targets
	else {
		printf("Error; only found %i means. Cannot continue.\n",meanCount);
	}

	double* HRData = new double[4];

	HRData[0] = HRTotal;
	HRData[1] = mean1;
	HRData[2] = mean2;
	HRData[3] = mean3;

	return HRData;

}

double calculateAngleOfDeviation(double* l, double* r){

	if(l == NULL || r == NULL) {
		printf("ERROR: one or both of the HR calculations returned bad values. Exiting\n");
		return 0 ;
	}

	double HR_left = l[0];
	double mean1_left = l[1];
	double mean2_left = l[2];
	double mean3_left = l[3];

	double HR_right = r[0];
	double mean1_right = r[1];
	double mean2_right = r[2];
	double mean3_right = r[3];

	double HR = 0;
	// Assume dominant eye is right ( TODO ) : allow user to set dominant eye
	if(HR_right != 0){
		printf("setting HR from right eye\n");
		HR = HR_right;
	}
	else if(HR_left != 0){
		printf("Right eye HR was invalid. setting HR from left eye\n");
		HR = HR_left;
	}
	else {
		printf("ERROR: both of the HR calculations are 0. Exiting\n");
		return 0;
	}

	// try to set the differences between both eyes
	// try to use middle targets, using left and right targets as backups
	double diff = 0;
	if ( mean2_left != 0 && mean2_right != 0) {
		diff = mean2_left-mean2_right;
		printf("using middle target mean differences to calculate angle of deviation\n.");
	}
	else if ( mean1_left != 0 && mean1_right != 0) {
		printf("at least one of the middle target  mean differences was 0.\n");
		diff = mean1_left-mean1_right;
		printf("using left target mean differences to calculate angle of deviation\n.");
	}
	else if ( mean3_left != 0 && mean3_right != 0) {
		printf("at least one of the left target  mean differences was 0.\n");
		diff = mean3_left-mean3_right;
		printf("using right target mean differences to calculate angle of deviation\n.");
	}
	else {
		printf("ERROR: none of the targets could succesfully find mean differences from both eyes. Exiting.\n");
		return 0;
	}

	// calcuate angle of deviation
	const double PIXELS = 1920;     					// pixels
	const double WIDTH = 112.5; 						// mm

	double angle_of_deviation = diff * WIDTH/PIXELS * HR ;

	printf("Angle of deviation: %f\n",angle_of_deviation);

	return angle_of_deviation;
}

// returns the results of a sqlite statement (a query)
// as a 2d double array. The query must expect integer or
// double type values exclusively as a return type
// i.e. no strings should be expected in the result
// assumes global database pointer db exists
double** sqlite_query_to_array(string query) {

	// SQL variables
	sqlite3_stmt *stmt;
	const char *tail;

	// get the number of rows in the query
	int nrows = sqlite_get_rows(query);

	// execute the main query
	rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

	// create exception if error occurs
	sqlite_exception(rc);

	// get the number of columns in the query
	const int ncolumns = sqlite3_column_count(stmt);

	// put the results of the query into an array
	if (nrows == 0) {
		printf("Query returns 0 rows\n");
		return NULL;
	}
	else if (ncolumns == 0) {
		printf("Query returns 0 columns\n");
		return NULL;
	}
	else {
		double** data = new double*[nrows];
		for(int i = 0; i < nrows; i++) {
			data[i] = new double[ncolumns];
		}

		int i = 0;
		while ( sqlite3_step(stmt) == SQLITE_ROW ) {
			for( int j = 0 ; j < ncolumns; j++) {
				const char* columnValue = (const char*) sqlite3_column_text(stmt,0);
				// handle null pointers by setting value to 0.0
				if (columnValue == NULL ){
					data[i][j] = 0.0;
				}
				else {
					data[i][j] = strtod(columnValue,NULL);
				}
			}
			i++;
		}
		return data;
	}
}

// assumes db is a global variable
int sqlite_get_rows(string query) {

	// SQL variables
	sqlite3_stmt *stmt;

	// get the number of rows in the query
	char* rowsQuery = 0;// get the number of rows in the query
	rowsQuery = (char*)calloc(255,1);

	strcpy(rowsQuery,"SELECT count(*) FROM ( ");
	strcat(rowsQuery,query.c_str());
	strcat(rowsQuery," )");

	rc = sqlite3_prepare_v2(db,rowsQuery,-1,&stmt,0);

	// create exception if error occurs
	sqlite_exception(rc);

	int nrows = 0;
	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		nrows = atoi((const char*)sqlite3_column_text(stmt,0));
		// printf("Rows: %i\n",nrows);
	}

	return nrows;
}

// assumes db is a global variable
void sqlite_exception(int rc) {

	// create exception if error occurs
	if( rc != SQLITE_OK ) {
		fprintf(stderr, "sqlite error: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	return;

}
