#include <sys/stat.h>
#include <fstream>
#include <iostream>
using namespace std;

#include "cv.h"
#include "highgui.h"
#include "sqlite3.h"
#include "math.h"
#define _USE_MATH_DEFINES
using namespace cv;

#define ELEM(type,start,step,size,xpos,ypos) *((type*)(start+step*(ypos)+(xpos)*size))

#include <QApplication>
#include <QtGui>
#include <QtCore>
#include "camera.h"

int MIN_FRAME, MAX_FRAME;

CvRect rectEye;

// FILE DECLARATIONS
FILE * error_log;
FILE * pr_contours;
FILE * pr_contours_expanded;
FILE * clustered_pr_contours;
FILE * clustered_pupil_contours;
FILE * pupil_contours_right;
FILE * pupil_contours_left;
FILE * dist_contours_0;
FILE * dist_contours_1;
FILE * dist_contours_0_filtered;
FILE * dist_contours_1_filtered;
FILE * statistics;
FILE * TEMP_expanded_pr;

double IMAGE_WIDTH; // pixels
double IMAGE_HEIGHT; // pixels
const double VIEW_WIDTH = 243.3; // mm
const double ratio = 25/12.5;    // PD / deg
const double VIEW_DIST = 1.05; // meters

QProgressBar *progressBar;

sqlite3* db;
char* zErrMsg;
int rc = 0;
float ROI_offset_X, ROI_offset_Y;

int NO_EDGE = 0;
int POSSIBLE_EDGE = 100;
int EDGE = 255;

int MIN_RADIUS, MAX_RADIUS, MEAN_RADIUS, COUNT_RADIUS;

float MIN_THRESH, MAX_THRESH, MEAN_THRESH;
int COUNT_THRESH;

Mat edges;

// These values will be passed in from the GUI
char* patientID;
char* techID;
int vidID;

/*
// Temporary Eclipse Declarations
char* patientID = (char*)"Patient";
char* techID = (char*)"Operator";
int vidID = 0;
*/

// List of directories for saving images
char *inputDir, *rightDir, *leftDir, *logDir, *prDir;

vector<RotatedRect> getContours( Mat input, float minWidth, float maxWidth, float minEccentricity, float maxEccentricity);

bool minSizeFilterContour(Mat , float );
bool maxSizeFilterContour(Mat , float );
bool eccentricityFilterContour(Mat , float, float );

Mat getROI(Mat img,float center_x,float center_y, int ROI_SIZE);
void printMat(Mat);

void locatePurkinje(Mat img_face, int frameNum);
void locatePupil(Mat img_face, float PR_x, float PR_y, int frameNum, int cluster);
void saveImage(const char*,const char*,int,Mat);

void ImageAnalysisPR();
void ImageAnalysisPupil();

void filterContours(char* table);

void recordLogs();

// SQLITE AUXILARY FUNCTIONS
double sqlite_query_value( string query );
int sqlite_get_columns(string query);
static int print_query(void *fstream, int argc, char **argv, char **azColName);
static int print_query2(void *fstream, int argc, char **argv, char **azColName);
void sqlite_exception(int rc);
static void sqlite_command(string sql_cmd, FILE * file = stdout );
static void sqlite_output_table(string sql_cmd, FILE * file);
int sqlite_get_rows(string);
double** sqlite_query_to_array(string query);
void sqlite_print_table_info(FILE *);
Mat sqlite_query_to_mat(string query);

// CUSTOM SQLITE FUNCTIONS
void sqlite_register_functions(sqlite3 * db);
static void sqrtFunc(sqlite3_context *context, int argc, sqlite3_value **argv);
static void varianceStep(sqlite3_context *context, int argc, sqlite3_value **argv);
static void stdevFinalize(sqlite3_context *context);
static void atanFunc(sqlite3_context *context, int argc, sqlite3_value **argv);

double* linReg(double** values, double x_mean, double y_mean, double count);

// Hough Transform Functions
void followEdges(Mat magnitude, int i, int j, float lowThreshold);
void accum_circle(Mat houghImg, int i, int j, int rows, int cols, int radius);
void accum_pixel(Mat houghImg, int i, int j, int rows, int cols);
void draw_circle(Mat drawPupil, int i, int j, int rows, int cols, int radius);
void draw_pixel(Mat drawPupil, int i, int j, int rows, int cols);

// CUSTOM SQLITE DATATYPE
typedef struct StdevCtx StdevCtx;
struct StdevCtx {
  double rM;
  double rS;
  int cnt;          /* number of elements */
};

// CLUSTERING ALGORITHMS
int cluster_table(string query, char* table, int nclusters = 2);
Mat cluster(string query, int nclusters);

Mat cluster(string query, int nclusters)
{
        Mat m = sqlite_query_to_mat(query);
        // transpose(m,m);

        Mat labels, centers;

        if (m.data != NULL) {
                kmeans( m, nclusters, labels, cv::TermCriteria(), 1 ,KMEANS_PP_CENTERS, &centers ); // centers in OpenCV 2.3, &centers in OpenCV 2.1-2
        }
        return labels;
}

// function cluster_table
// @query: the sql query to pass to the clustering function
// @table: the name of the table to
// updates the cluster values in the table
int cluster_table(string query, char* table, int nclusters) {

                char* cmd;
        // get the total number of current clusters in the contours table
        // so that each cluster has a unique value
        cmd = new char[255];
        sprintf(cmd,"SELECT max(cluster) FROM %s",table);
        int clusterCount = (int)(sqlite_query_value(cmd));
        // workaround for initial clusters
        if( clusterCount != 0 ) {
                clusterCount++;
        }
        delete[] cmd;

        int NUM_RETRIES = 3;
        int counter = 0;

        while(counter < NUM_RETRIES) {
                        // get the cluster number for each contour in the query
                        Mat clusterid = cluster(query,nclusters);
                        if (clusterid.data == NULL) {
                                return -1;
                        }

                        // get the total number of contours in the query
                        int count = sqlite_get_rows(query);
                        // get the x and y locations of the contours in the query
                        cmd = new char[255];
                        sprintf(cmd,"SELECT ROWID FROM %s",table);
                        double** rowIds = sqlite_query_to_array(cmd);
                        delete[] cmd;

                        // begin transaction
                        sqlite_command("BEGIN TRANSACTION");

                        // update the table with the clusters
                        for ( int i = 0; i < count ; i++ ) {
                                char* cmd = new char[255];
                                sprintf(cmd, "UPDATE %s SET cluster = '%i' WHERE ROWID = %i", table, clusterid.at<int>(i,0)+clusterCount,(int)rowIds[i][0]);
                                sqlite_command(cmd);
                                delete[] cmd;
                        }

                        double* clusterLocations = new double[nclusters];

                        for (int i = 0; i < nclusters; i++) {
                                char* cmd = new char[255];
                                sprintf(cmd, "SELECT avg(dist) FROM %s WHERE cluster = %i", table, i);
                                clusterLocations[i] = sqlite_query_value(cmd);
                                delete[] cmd;
                        }

                        int* clusterIndex = new int[nclusters];
                        for (int i = 0; i < nclusters; i++) {
                                clusterIndex[i] = i;
                        }
                        // two-pass bubble sort index values
                        for (int i = 0; i < nclusters-1; i++) {
                                for (int j = 0; j < nclusters-1; j++) {
                                        if (abs(clusterLocations[j]) > abs(clusterLocations[j+1])) {
                                                int tempIndex = clusterLocations[j];
                                                clusterLocations[j] = clusterLocations[j+1];
                                                clusterLocations[j+1] = tempIndex;
                                        }
                                }
                        }

                        double diff1 = abs(clusterLocations[1] - clusterLocations[0]);
                        double diff2 = abs(clusterLocations[2] - clusterLocations[1]);

                        if (diff2 >= 2*diff1) {
                                // delete cluster values from SQLite database and cluster again
                                char* cmd = new char[255];
                                sprintf(cmd, "DELETE FROM %s WHERE cluster = %i", table, clusterIndex[2]);
                                delete[] cmd;

                                clusterid.release();
                                counter++;
                        }
                        else {
                                // commit transaction
                                sqlite_command("COMMIT TRANSACTION");
                                break;
                        }

                        // commit transaction
                        sqlite_command("COMMIT TRANSACTION");

                // clean up data
                delete[] rowIds;
        }

        // get the new value for the largest cluster
        cmd = new char[255];
        sprintf(cmd,"SELECT max(cluster) FROM %s",table);
        clusterCount = (int)(sqlite_query_value(cmd));
        delete[] cmd;

        // return
        return clusterCount;
}

// callback function for sqlite3
// prints all rows to specified file stream
// assumes first input is of type FILE *
static int print_query(void * fstream, int argc, char **argv, char **azColName)
{
        int i;

        // assume fstream is a FILE*
        FILE * file = (FILE *)fstream;

        for(i=0; i<argc; i++)
        {
                fprintf(file,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        }
        fprintf(file,"\n");
        return 0;
}

// callback function for sqlite3
// prints all rows to specified file stream
// assumes first input is of type FILE *
// formatted for tab-delimited text file
static int print_query2(void * fstream, int argc, char **argv, char **azColName)
{
        int i;

        // assume fstream is a FILE*
        FILE * file = (FILE *)fstream;

        for(i=0; i<argc; i++)
        {
                char* output_value;
                if (argv[i] != NULL) {
                        float temp_value = atof((const char*)argv[i]);
                        output_value = new char[255];
                        sprintf(output_value,"%4.2f",temp_value);
                }
                else {
                        output_value = NULL;
                }
                fprintf(file,"%s\t", output_value);
                delete[] output_value;
        }
        fprintf(file,"\n");
        return 0;
}

// testing options
bool prAnalysis = true;
bool prFilter = true;
bool pupilAnalysis = true;
bool pupilFilter = true;
bool HRCalculation = true;
bool AODCalculation = true;

double* ImageAnalysis(QString patient, QString tech, int vid, QProgressBar *analysisProgressBar, QLabel *progressLabel) {

    double* aod_results = new double[3];
    patientID = new char[255];
    sprintf(patientID,"%s",(const char*)patient.toLatin1());

    techID = new char[255];
    sprintf(techID,"%s",(const char*)tech.toLatin1());

    vidID = vid;

    // Initialize radius parameters
    MIN_RADIUS = 15;
    MAX_RADIUS = 40;
    MEAN_RADIUS = (MIN_RADIUS + MAX_RADIUS) / 2;
    COUNT_RADIUS = 2;

    // Initialize PR binary threshold parameters
    MIN_THRESH = -1;
    MAX_THRESH = -1;
    MEAN_THRESH = 0;
    COUNT_THRESH = 0;

    // Initialize sensor parameters
    double* SENSOR_SIZE = camera::getSensorSize();
    IMAGE_WIDTH = SENSOR_SIZE[0];
    IMAGE_HEIGHT = SENSOR_SIZE[1];

    /*
    // ECLIPSE COMMENT
    IMAGE_WIDTH = 1280;
    IMAGE_HEIGHT = 1024;
    */

    fprintf(stderr,"\n %s, %s, %i",patientID,techID,vidID);

    progressBar = analysisProgressBar;

    // Begin timer for entire program
    time_t t_begin = time(NULL);

        // Make new directories needed for this project:
    rightDir = new char[255];
        sprintf(rightDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Right Eye", patientID, techID, vidID);
        mkdir(rightDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    leftDir = new char[255];
        sprintf(leftDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Left Eye", patientID, techID, vidID);
        mkdir(leftDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    prDir = new char[255];
        sprintf(prDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/PR Thresholding", patientID, techID, vidID);
        mkdir(prDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    logDir = new char[255];
        sprintf(logDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Logs", patientID, techID, vidID);
        mkdir(logDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        chdir(logDir);

        // Initialize Log files : (TODO) Update and consolidate log files
        error_log = fopen ("error_log.txt","w");
        pr_contours = fopen ("pr_contours.txt","w");
        pr_contours_expanded = fopen ("pr_contours_expanded.txt","w");
        clustered_pr_contours = fopen ("clustered_pr_contours.txt","w");
        clustered_pupil_contours = fopen ("clustered_pupil_contours.txt","w");
        pupil_contours_right = fopen ("pupil_contours_right.txt","w");
        pupil_contours_left = fopen ("pupil_contours_left.txt","w");
        dist_contours_0 = fopen ("dist_contours_0.txt","w");
        dist_contours_1 = fopen ("dist_contours_1.txt","w");
        dist_contours_0_filtered = fopen ("dist_contours_0_filtered.txt","w");
        dist_contours_1_filtered = fopen ("dist_contours_1_filtered.txt","w");
        statistics = fopen ("statistics.txt","w");
        TEMP_expanded_pr = fopen ("tempExpandedPR.txt","w");

        fprintf(error_log,"Starting Main\n");

        // SQLITE3 variables
        zErrMsg = 0;

        // Switch to input frame directory
        inputDir = new char[255];
        sprintf(inputDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames", patientID, techID, vidID);
        chdir(inputDir);

        // Import number of frames from txt file
        int numFrames = 0;
        int x;

        ifstream inFile;
        inFile.open("frameCount.txt");
        if (!inFile) {
                cout << "Unable to open file";
                exit(1); // terminate with error
        }

        while (inFile >> x) {
                numFrames = numFrames + x;
        }

        inFile.close();

        // Global declarations for MIN_FRAME and MAX_FRAME
        MIN_FRAME = 0;
        MAX_FRAME = numFrames-1;

        fprintf(statistics,"Total # of Frames: %i\n\n",MAX_FRAME-MIN_FRAME);

        fprintf(error_log,"Initializing database\n");
        rc = sqlite3_open("contours",&db);
        sqlite_exception(rc);

        // add extension functions
        fprintf(error_log,"Registering Sqlite Extension functions \n");
        sqlite_register_functions(db);

        fprintf(error_log,"Creating Database tables\n");

        if ( prAnalysis ) {
                // create table pr_contours: stores data from image processing function
                // called in ImageAnalysis()
                sqlite_command("CREATE TABLE IF NOT EXISTS pr_contours (frameNumber INTEGER, x REAL, y REAL, w REAL, h REAL, cluster INTEGER)");
                // reset the table
                sqlite_command("DELETE from pr_contours");
        }

        if ( prFilter ) {
                // create table clustered_pr_contours: stores filtered data from pr_contours
                // called in filterContours()
                sqlite_command("CREATE TABLE IF NOT EXISTS clustered_pr_contours (frameNumber INTEGER, x REAL, y REAL, w REAL, h REAL, cluster INTEGER)");
                // reset the table
                sqlite_command("DELETE from clustered_pr_contours");
        }

        if ( pupilAnalysis ) {
                // create table pupil_contours: stores data from image processing function
                // called in ImageAnalysis()
                sqlite_command("CREATE TABLE IF NOT EXISTS pupil_contours (frameNumber INTEGER, x REAL, y REAL, w REAL, h REAL, cluster INTEGER)");
                // reset the table
                sqlite_command("DELETE from pupil_contours");
        }

        if ( pupilFilter ) {
                // create table clustered_pr_contours: stores filtered data from pr_contours
                // called in filterPRs()
                sqlite_command("CREATE TABLE IF NOT EXISTS clustered_pupil_contours (frameNumber INTEGER, x REAL, y REAL, w REAL, h REAL, cluster INTEGER)");
                // reset the table
                sqlite_command("DELETE from clustered_pupil_contours");
        }

        sqlite_print_table_info(stdout);

        progressBar->setValue(0);
        qApp->processEvents();

        time_t t1;
        time_t t2;

        // Find Purkinje Reflexes (PRs) in images
        // And add them to a database table
        if ( prAnalysis ) {
                fprintf(error_log,"Executing PR ImageAnalysisPR()\n");
                fprintf(stderr,"Executing PR ImageAnalysisPR()\n");

                progressLabel->setText("PR Analysis");

                t1 = time(NULL);

                // begin transaction
                sqlite_command("BEGIN TRANSACTION");

                ImageAnalysisPR();

                // commit transaction
                sqlite_command("COMMIT TRANSACTION");

                t2 = time(NULL);
                fprintf(statistics,"PR ImageAnalysis: %f sec\n",(double)(t2-t1));
                fprintf(statistics,"Average time per frame: %f sec\n",((double)(t2-t1))/(MAX_FRAME-MIN_FRAME));
                fprintf(statistics,"FPS: %f\n\n",((MAX_FRAME-MIN_FRAME)/(double)(t2-t1)));
        }

        if ( prFilter ) {
                fprintf(error_log,"Executing filterContours()\n");
                fprintf(stderr,"Executing filterContours()\n");

                progressLabel->setText("PR Filtering");

                t1 = time(NULL);

                filterContours((char*)"pr_contours");

                sqlite_print_table_info(stdout);
        }

        // Find Pupil Contours in images
        // and add them to a table in a database
        if ( pupilAnalysis ) {
                fprintf(error_log,"Executing ImageAnalysisPupiling()\n");
                fprintf(stderr,"Executing ImageAnalysisPupiling()\n");

                progressLabel->setText("Pupil Analysis");

                t1 = time(NULL);

                // begin transaction
                sqlite_command("BEGIN TRANSACTION");

                ImageAnalysisPupil();

                // commit transaction
                sqlite_command("COMMIT TRANSACTION");

                t2 = time(NULL);
                fprintf(statistics,"Pupil ImageAnalysis: %f sec\n",(double)(t2-t1));
                fprintf(statistics,"Average time per frame: %f sec\n",((double)(t2-t1))/(MAX_FRAME-MIN_FRAME));
                fprintf(statistics,"FPS: %f\n\n",((MAX_FRAME-MIN_FRAME)/(double)(t2-t1)));
        }

        if ( pupilFilter ) {
                fprintf(error_log,"Executing filterPRs()\n");

                t1 = time(NULL);

                filterContours((char*)"pupil_contours");

                t2 = time(NULL);
                fprintf(statistics,"pupil_contours filtering: %f sec\n\n",(double)(t2-t1));

                sqlite_print_table_info(stdout);
        }


        // (TODO): Split some of this into a separate method to clean up the code
        if ( HRCalculation ) {
                fprintf(error_log,"Clustering dist_contours\n");

                progressLabel->setText("Calculating Statistics");

                time_t t1 = time(NULL);

                // Only analyze pr contour values where it found a successful pair of pupils
                sqlite_command("DELETE FROM clustered_pr_contours WHERE frameNumber NOT in (SELECT clustered_pupil_contours.frameNumber FROM clustered_pupil_contours INNER JOIN clustered_pr_contours WHERE clustered_pupil_contours.frameNumber = clustered_pr_contours.frameNumber)");

                double mean_pr_0 = sqlite_query_value("SELECT avg(x) from clustered_pr_contours WHERE cluster = 0");
                double mean_pr_1 = sqlite_query_value("SELECT avg(x) from clustered_pr_contours WHERE cluster = 1");

                // cluster the contours for each eye
                // DEFINITION: cluster 0 = RIGHT eye, cluster 1 = LEFT eye (all definitions are from patient's perspective)
                sqlite_command("CREATE TABLE IF NOT EXISTS dist_contours_0 (frameNumber INTEGER, dist REAL, cluster INTEGER, angle REAL)");
                sqlite_command("DELETE FROM dist_contours_0");

                sqlite_command("CREATE TABLE IF NOT EXISTS dist_contours_1 (frameNumber INTEGER, dist REAL, cluster INTEGER, angle REAL)");
                sqlite_command("DELETE FROM dist_contours_1");

                fprintf(stderr,"Clustering Values: %f, %f\n",mean_pr_0,mean_pr_1);

                if (mean_pr_0 < mean_pr_1) { // same clustering order used
                                fprintf(stderr,"PR_0 < PR_1");
                        sqlite_command("REPLACE INTO dist_contours_0 (frameNumber, dist) SELECT p.FrameNumber,c.x-p.x AS dist FROM (SELECT * FROM clustered_pr_contours WHERE cluster = 0 ) AS p , (SELECT * FROM clustered_pupil_contours WHERE cluster = 0 ) AS c WHERE c.frameNumber = p.frameNumber AND abs(dist) < 200");
                        sqlite_command("REPLACE INTO dist_contours_1 (frameNumber, dist) SELECT p.FrameNumber,c.x-p.x AS dist FROM (SELECT * FROM clustered_pr_contours WHERE cluster = 1 ) AS p , (SELECT * FROM clustered_pupil_contours WHERE cluster = 1 ) AS c WHERE c.frameNumber = p.frameNumber AND abs(dist) < 200");
                }
                else { // switch clustering order to match definition
                                fprintf(stderr,"PR_0 > PR_1");
                        sqlite_command("REPLACE INTO dist_contours_0 (frameNumber, dist) SELECT p.FrameNumber,c.x-p.x AS dist FROM (SELECT * FROM clustered_pr_contours WHERE cluster = 1 ) AS p , (SELECT * FROM clustered_pupil_contours WHERE cluster = 1 ) AS c WHERE c.frameNumber = p.frameNumber AND abs(dist) < 200");
                        sqlite_command("REPLACE INTO dist_contours_1 (frameNumber, dist) SELECT p.FrameNumber,c.x-p.x AS dist FROM (SELECT * FROM clustered_pr_contours WHERE cluster = 0 ) AS p , (SELECT * FROM clustered_pupil_contours WHERE cluster = 0 ) AS c WHERE c.frameNumber = p.frameNumber AND abs(dist) < 200");
                }

                sqlite_output_table("SELECT * FROM dist_contours_0",dist_contours_0);

                int num_targets = 3;

                if(cluster_table("SELECT dist from dist_contours_0",(char*)"dist_contours_0",num_targets) == -1) {
                        fprintf(stderr,"Early termination of program due to invalid clustering for dist_contours_0\n");
                        fprintf(error_log,"Early termination of program due to invalid clustering for dist_contours_0\n");
                        recordLogs();
                        return NULL;
                }
                if(cluster_table("SELECT dist from dist_contours_1",(char*)"dist_contours_1",num_targets) == -1) {
                        fprintf(stderr,"Early termination of program due to invalid clustering for dist_contours_1\n");
                        fprintf(error_log,"Early termination of program due to invalid clustering for dist_contours_1\n");
                        recordLogs();
                        return NULL;
                }

                progressBar->setValue(99);
                qApp->processEvents();

                // Print out clustered dist_contours tables to txt file
                // before any filtering or angle correction is done
                fprintf(dist_contours_0,"fNum\tdist\tcluster\tangle\n");
                sqlite_output_table("SELECT * FROM dist_contours_0",dist_contours_0);

                fprintf(dist_contours_1,"fNum\tdist\tcluster\tangle\n");
                sqlite_output_table("SELECT * FROM dist_contours_1",dist_contours_1);

                t1 = time(NULL);

                fprintf(error_log,"Filtering dist_contours\n");

                /*
                float* meanCluster = new float[num_targets];
                float* sdCluster = new float[num_targets];
                */

                char* cmd;
                char* dist_contours;

                for (int k=0; k<2; k++) {
                        dist_contours = new char[255];

                        if (k==0) {
                                sprintf(dist_contours,"dist_contours_0");
                        }
                        else if (k==1) {
                                sprintf(dist_contours,"dist_contours_1");
                        }
                        printf("\n %s",dist_contours);

                        /*
                        for (int i=0; i<num_targets; i++) {
                                cmd = new char[255];
                                sprintf(cmd,"SELECT avg(dist) from %s WHERE cluster = %i",dist_contours,i);
                                meanCluster[i] = sqlite_query_value(cmd);
                                delete[] cmd;

                                cmd = new char[255];
                                sprintf(cmd,"SELECT stdev(dist) from %s WHERE cluster = %i",dist_contours,i);
                                sdCluster[i] = sqlite_query_value(cmd);
                                delete[] cmd;
                        }

                        for (int i=0; i<num_targets; i++) {
                                cmd = new char[255];
                                sprintf(cmd, "DELETE FROM %s WHERE cluster = %i AND (dist < %f OR dist > %f)", dist_contours, i, meanCluster[i] - sdCluster[i], meanCluster[i] + sdCluster[i] );
                                sqlite_command(cmd);
                                delete[] cmd;
                        }
                        */

                        // Derivative Data Filtering
                        cmd = new char[255];
                        sprintf(cmd, "SELECT frameNumber from %s", dist_contours);
                        double** frameNumArray = sqlite_query_to_array(cmd);
                        delete[] cmd;

                        cmd = new char[255];
                        sprintf(cmd, "SELECT count(frameNumber) from %s", dist_contours);
                        double frameCount = sqlite_query_value(cmd);
                        delete[] cmd;

                        cmd = new char[255];
                        sprintf(cmd,"SELECT dist from %s WHERE frameNumber = %i", dist_contours, (int)frameNumArray[0][0]);
                        double backVal = sqlite_query_value(cmd);
                        delete[] cmd;

                        cmd = new char[255];
                        sprintf(cmd,"SELECT dist from %s WHERE frameNumber = %i", dist_contours, (int)frameNumArray[1][0]);
                        double centerVal = sqlite_query_value(cmd);
                        delete[] cmd;

                        double forwardVal;
                        double backAvg, forwardAvg;
                        double centralDiff;

                        // TODO: Find better way to define these thresholds
                        double CENTRAL_DIFF_THRESHOLD		=		2.5;
                        double DIRECT_DIFF_THRESHOLD		=		6;

                        printf("\n frameCount = %i",(int)frameCount);

                        cmd = new char[255];
                        sprintf(cmd,"SELECT ROWID FROM %s",dist_contours);
                        double** rowIds = sqlite_query_to_array(cmd);
                        delete[] cmd;

                        // begin transaction
                        sqlite_command("BEGIN TRANSACTION");

                        int j = 1;
                        while (j < (int)frameCount - 1) {

                            cmd = new char[255];
                            sprintf(cmd,"SELECT dist from %s WHERE frameNumber = %i", dist_contours, (int)frameNumArray[j+1][0]);
                            forwardVal = sqlite_query_value(cmd);
                            delete[] cmd;

                            backAvg = (backVal + centerVal) / 2;
                            forwardAvg = (centerVal + forwardVal) / 2;
                            centralDiff = forwardAvg - backAvg;

                                if (frameNumArray[j+1][0] - frameNumArray[j][0] < 5) {
                                // forward and backward difference
                                if (abs(forwardVal - centerVal) > DIRECT_DIFF_THRESHOLD && abs(centerVal - backVal) > DIRECT_DIFF_THRESHOLD) {
                                        cmd = new char[255];
                                        sprintf(cmd, "DELETE FROM %s WHERE ROWID = %i", dist_contours, (int)rowIds[j][0]);
                                        sqlite_command(cmd);
                                        delete[] cmd;

                                        backVal = forwardVal;

                                        cmd = new char[255];
                                        sprintf(cmd,"SELECT dist from %s WHERE frameNumber = %i", dist_contours, (int)frameNumArray[j+2][0]);
                                        centerVal = sqlite_query_value(cmd);
                                        delete[] cmd;
                                        j = j+2;
                                }

                            // central difference
                                else if (abs(centralDiff) > CENTRAL_DIFF_THRESHOLD) {
                                        cmd = new char[255];
                                        sprintf(cmd, "DELETE FROM %s WHERE ROWID = %i", dist_contours, (int)rowIds[j][0]);
                                        sqlite_command(cmd);
                                        delete[] cmd;

                                        backVal = centerVal;
                                        centerVal = forwardVal;
                                        j++;
                                }
                                else {
                                        backVal = centerVal;
                                        centerVal = forwardVal;
                                        j++;
                                }
                                }
                                else {
                                backVal = centerVal;
                                centerVal = forwardVal;
                                j++;
                                }
                        }
                        free(rowIds);

                        // commit transaction
                        sqlite_command("COMMIT TRANSACTION");
                }

                // Determine which cluster is which angle
                int nrows = 2; // 2 eyes
                int ncolumns = 3; // 3 locations

                int** clusterSort = new int*[nrows];
                for(int i = 0; i < nrows; i++) {
                        clusterSort[i] = new int[ncolumns];
                }

                double** tempSort = new double*[nrows];
                for(int i = 0; i < nrows; i++) {
                        tempSort[i] = new double[ncolumns];
                }

                for (int i = 0; i < nrows; i++) {
                        for (int j = 0; j < ncolumns; j++) {
                                cmd = new char[255];
                                sprintf(cmd,"SELECT avg(dist) from dist_contours_%i WHERE cluster = %i", i, j);
                                tempSort[i][j] = sqlite_query_value(cmd);
                                clusterSort[i][j] = j;
                                delete[] cmd;
                        }
                }

                double tempVal;
                int tempCluster;

                // Perform bubble sort (3 passes)
                for (int k = 0; k < 3; k++) {
                        for (int i = 0; i < nrows; i++) {
                                for (int j = 1; j < ncolumns; j++) {
                                        if (tempSort[i][j] < tempSort[i][j-1]) {
                                                tempVal = tempSort[i][j];
                                                tempCluster = clusterSort[i][j];

                                                tempSort[i][j] = tempSort[i][j-1];
                                                tempSort[i][j-1] = tempVal;

                                                clusterSort[i][j] = clusterSort[i][j-1];
                                                clusterSort[i][j-1] = tempCluster;
                                        }
                                }
                        }
                }

                t1 = time(NULL);

                for (int i = 0; i < nrows; i++) {
                        fprintf(statistics,"dist_contours_%i:\n",i);
                        for (int j = 0; j < ncolumns; j++) {
                                printf("\n Temp values: %f, %i for i=%i, j=%i",tempSort[i][j], clusterSort[i][j], i, j);

                                // Find the number of frames for a given cluster
                                cmd = new char[255];
                                sprintf(cmd, "SELECT * FROM dist_contours_%i WHERE cluster = %i", i, clusterSort[i][j]);
                                int numRows = sqlite_get_rows(cmd);

                                fprintf(statistics,"Cluster = %i\t Mean = %f\t # Frames = %i\n",clusterSort[i][j],tempSort[i][j],numRows);
                        }
                        fprintf(statistics,"\n");
                }

                delete[] tempSort;

                int* angles = new int[ncolumns];
                angles[0] = 30;
                angles[1] = 0;
                angles[2] = -30;

                // begin transaction
                sqlite_command("BEGIN TRANSACTION");

                for (int i=0; i<nrows; i++) {
                        for (int j=0; j<ncolumns; j++) {
                                cmd = new char[255];
                                sprintf(cmd,"UPDATE OR REPLACE dist_contours_%i SET angle = %i WHERE dist_contours_%i.cluster = %i", i, angles[j], i, clusterSort[i][j]);
                                sqlite_command(cmd);
                                delete[] cmd;
                        }
                }

                // commit transaction
                sqlite_command("COMMIT TRANSACTION");

                delete[] angles;

                // Sort right and left pupil values into sorted arrays for later retrieval

                double** pupil_right = sqlite_query_to_array("SELECT * from clustered_pupil_contours WHERE cluster = 0");
                int pupil_right_rows = sqlite_get_rows("SELECT * from clustered_pupil_contours WHERE cluster = 0");

                double** pupil_left = sqlite_query_to_array("SELECT * from clustered_pupil_contours WHERE cluster = 1");
                int pupil_left_rows = sqlite_get_rows("SELECT * from clustered_pupil_contours WHERE cluster = 1");

                int rightCounter = 0;
                int leftCounter = 0;

                int rightFrameNum = 0;
                int leftFrameNum = 0;

                double* pupilLocations_0 = new double[MAX_FRAME+1];
                double* pupilLocations_1 = new double[MAX_FRAME+1];

                // TODO: Replace with SQLite statement
                for (rightCounter = 0; rightCounter < pupil_right_rows; rightCounter++) {
                        rightFrameNum = pupil_right[rightCounter][0];
                        double pupil_right_val = pupil_right[rightCounter][1];
                        pupilLocations_0[rightFrameNum] = pupil_right_val;
                }

                for (leftCounter = 0; leftCounter < pupil_left_rows; leftCounter++) {
                        leftFrameNum = pupil_left[leftCounter][0];
                        double pupil_left_val = pupil_left[leftCounter][1];
                        pupilLocations_1[leftFrameNum] = pupil_left_val;
                }

                delete[] pupil_right;
                delete[] pupil_left;

                // Update angles for each position of dist_contours
                fprintf(error_log,"Correcting angles for dist_contours\n");

                for (int i=0; i<2; i++) {
                        cmd = new char[255];
                        sprintf(cmd,"SELECT * from dist_contours_%i",i);
                        double** dist_array = sqlite_query_to_array(cmd);
                        int dist_array_rows = sqlite_get_rows(cmd);
                        delete[] cmd;

                        // reset the table
                        cmd = new char[255];
                        sprintf(cmd,"DELETE FROM dist_contours_%i",i);
                        sqlite_command(cmd);
                        delete[] cmd;

                        // begin transaction
                        sqlite_command("BEGIN TRANSACTION");

                        // dist_array: frameNumber, dist, cluster, angle
                        for (int j=0; j<dist_array_rows; j++) {

                                int frameNum = (int)dist_array[j][0];
                                double currentAngle = dist_array[j][3];

                                double pupil_right_val = pupilLocations_0[frameNum];
                                double pupil_left_val = pupilLocations_1[frameNum];
                                double pupil_dist = pupil_left_val - pupil_right_val;

                                double newAngle;

                                /* New approximated angles
                                        Dm = distance offset from center of camera x_r - (1280 / 2)
                                        De = half the distance between eyes (x_r - x_l) / 2
                                        Dp = 1m (distance person from camera)
                                        Dt = 0.577m (distance between OGLE targets)
                                 */

                                double pupil_val;
                                if (i == 0) { // right eye
                                        pupil_val = pupil_right_val;
                                }
                                else { // left eye
                                        pupil_val = pupil_left_val;
                                }

                                double Dm = (pupil_val - (IMAGE_WIDTH / 2)); // pixels
                                Dm = Dm * (double)(VIEW_WIDTH / (IMAGE_WIDTH * 1000)); // meters

                                double De = (pupil_dist / 2) * (double)(VIEW_WIDTH / (IMAGE_WIDTH * 1000)); // meters
                                double Dp = VIEW_DIST; // VIEW_DIST meters away from camera
                                double Dt = 0.577; // distance between OGLE targets (meters)

                                // Case 1: Right Eye
                                if (i == 0) {
                                        if (currentAngle == -30) {
                                                newAngle = -atan((Dt - Dm - De/2)/Dp) - (abs(Dm)/Dm)*atan(abs(Dm)/Dp);
                                                newAngle = newAngle * 180 / M_PI; // convert from radians to degrees
                                                //fprintf(stderr,"Right Eye -30, frame %i: Dm = %f, De = %f, Dp = %f, Dt = %f, newAngle = %f\n",frameNum,Dm,De,Dp,Dt,newAngle);
                                        }

                                        else if (currentAngle == 0) {
                                                newAngle = (abs(Dm)/Dm)*(atan(abs(Dm)/Dp) - atan((abs(Dm) + (abs(Dm)/Dm)*De/2)/Dp));
                                                newAngle = newAngle * 180 / M_PI; // convert from radians to degrees
                                                //fprintf(stderr,"Right Eye 0, frame %i: Dm = %f, De = %f, Dp = %f, Dt = %f, newAngle = %f\n",frameNum,Dm,De,Dp,Dt,newAngle);
                                        }

                                        else if (currentAngle == 30) {
                                                newAngle = atan((Dt + Dm + De/2)/Dp) - (abs(Dm)/Dm)*atan(abs(Dm)/Dp);
                                                newAngle = newAngle * 180 / M_PI; // convert from radians to degrees
                                                //fprintf(stderr,"Right Eye 30, frame %i: Dm = %f, De = %f, Dp = %f, Dt = %f, newAngle = %f\n",frameNum,Dm,De,Dp,Dt,newAngle);
                                        }

                                        else {
                                                fprintf(error_log,"\n ERROR: not expected input angle");
                                                fprintf(stderr,"\n ERROR: not expected input angle");
                                                newAngle = currentAngle;
                                        }
                                }

                                // Case 2: Left Eye
                                else if (i == 1) {
                                        if (currentAngle == -30) {
                                                newAngle = -atan((Dt - Dm + De/2)/Dp) - (abs(Dm)/Dm)*atan(abs(Dm)/Dp);
                                                newAngle = newAngle * 180 / M_PI; // convert from radians to degrees
                                                //fprintf(stderr,"Left Eye -30, frame %i: Dm = %f, De = %f, Dp = %f, Dt = %f, newAngle = %f\n",frameNum,Dm,De,Dp,Dt,newAngle);
                                        }

                                        else if (currentAngle == 0) {
                                                newAngle = (abs(Dm)/Dm)*(atan(abs(Dm)/Dp) - atan((abs(Dm) - (abs(Dm)/Dm)*De/2)/Dp));
                                                newAngle = newAngle * 180 / M_PI; // convert from radians to degrees
                                                //fprintf(stderr,"Left Eye 0, frame %i: Dm = %f, De = %f, Dp = %f, Dt = %f, newAngle = %f\n",frameNum,Dm,De,Dp,Dt,newAngle);
                                        }

                                        else if (currentAngle == 30) {
                                                newAngle = atan((Dt + Dm - De/2)/Dp) - (abs(Dm)/Dm)*atan(abs(Dm)/Dp);
                                                newAngle = newAngle * 180 / M_PI; // convert from radians to degrees
                                                //fprintf(stderr,"Left Eye 30, frame %i: Dm = %f, De = %f, Dp = %f, Dt = %f, newAngle = %f\n",frameNum,Dm,De,Dp,Dt,newAngle);
                                        }

                                        else {
                                                fprintf(error_log,"\n ERROR: not expected input angle");
                                                fprintf(stderr,"\n ERROR: not expected input angle");
                                                newAngle = currentAngle;
                                        }
                                }

                                else {
                                        fprintf(stderr, "Error updating angle, using default value\n");
                                        fprintf(error_log, "Error updating angle, using default value\n");
                                        newAngle = currentAngle;
                                }

                                cmd = new char[255];
                                sprintf(cmd,"INSERT INTO dist_contours_%i (frameNumber,dist,cluster,angle) VALUES (%i,%f,%i,%f)", i, (int)dist_array[j][0],dist_array[j][1],(int)dist_array[j][2],newAngle);
                                sqlite_command(cmd);
                                delete[] cmd;
                        }

                        // end transaction
                        sqlite_command("COMMIT TRANSACTION");

                        delete[] dist_array;
                }

                delete[] pupilLocations_0;
                delete[] pupilLocations_1;

                progressBar->setValue(99);
                qApp->processEvents();

                t1 = time(NULL);

                fprintf(error_log,"Calculating HR values\n");

                // angle = x_val, dist = y_val
                double x_mean_0 = sqlite_query_value("SELECT avg(angle) from dist_contours_0");
                double y_mean_0 = sqlite_query_value("SELECT avg(dist) from dist_contours_0");
                double count_0 = sqlite_query_value("SELECT count(dist) from dist_contours_0");

                double x_mean_1 = sqlite_query_value("SELECT avg(angle) from dist_contours_1");
                double y_mean_1 = sqlite_query_value("SELECT avg(dist) from dist_contours_1");
                double count_1 = sqlite_query_value("SELECT count(dist) from dist_contours_1");

                double** contourValues_0 = sqlite_query_to_array("SELECT * FROM dist_contours_0");
                double** contourValues_1 = sqlite_query_to_array("SELECT * FROM dist_contours_1");

                double* linRegVals_0 = linReg(contourValues_0, x_mean_0, y_mean_0, count_0);
                double* linRegVals_1 = linReg(contourValues_1, x_mean_1, y_mean_1, count_1);

                printf("\n#0: b = %f, a = %f, SE = %f\n",linRegVals_0[0],linRegVals_0[1],linRegVals_0[2]);
                printf("#1: b = %f, a = %f, SE = %f\n",linRegVals_1[0],linRegVals_1[1],linRegVals_1[2]);

                // Calculate HR in PD/mm
                double HR_0 = abs((1/linRegVals_0[0]) * ratio * (IMAGE_WIDTH / VIEW_WIDTH));
                double HR_1 = abs((1/linRegVals_1[0]) * ratio * (IMAGE_WIDTH / VIEW_WIDTH));

                printf("HR_0 = %f\n", HR_0);
                printf("HR_1 = %f\n", HR_1);

                // Determine which HR value to use to calculate angle of deviation
                double HR = 0;
                // Assume dominant eye is right ( TODO ) : allow user to set dominant eye
                if(HR_0 != 0 && HR_1 != 0 && abs(HR_1-HR_0) < 3) {
                        fprintf(stdout,"Averaging HR from both eyes\n");
                        HR = (HR_0 + HR_1) / 2;
                }
                else if(HR_0 != 0 && (abs(HR_0 - 21) < abs(HR_1 - 21))) {
                        fprintf(stderr,"Setting HR from right eye\n");
                        HR = HR_0;
                }
                else if(HR_1 != 0 && (abs(HR_1 - 21) < abs(HR_0 - 21))){
                        fprintf(stderr,"Setting HR from left eye\n");
                        HR = HR_1;
                }
                else {
                        fprintf(stderr,"ERROR: both of the HR calculations are invalid. Exiting\n");
                        return NULL;
                }

                // Calculate angle of deviation calcuation method:
                // Looks linear fit at each location between the two eyes, y-intercept is the AOD
                double aod_left, aod_center, aod_right;
                if ( AODCalculation ) {
                    // -30 deg target
                    double tempHR = (HR / ratio) * (VIEW_WIDTH / IMAGE_WIDTH); // deg/pixels
                    double y_mean_0_adjusted = 0; // = y_mean_t0_0 + (x_mean_t0_0 - (-30))/tempHR;
                    double y_mean_1_adjusted = 0; // = y_mean_t0_1 + (x_mean_t0_1 - (-30))/tempHR;

                    double** dist_array_0 = sqlite_query_to_array("SELECT * from dist_contours_0 WHERE angle < -25 AND angle > -35");
                    int dist_array_rows_0 = sqlite_get_rows("SELECT * from dist_contours_0 WHERE angle < -25 AND angle > -35");
                    for (int i=0; i<dist_array_rows_0; i++) {
                        y_mean_0_adjusted = y_mean_0_adjusted + dist_array_0[i][1] + (dist_array_0[i][3] - (-30))/tempHR;
                    }
                    y_mean_0_adjusted = y_mean_0_adjusted / dist_array_rows_0;

                    double** dist_array_1 = sqlite_query_to_array("SELECT * from dist_contours_1 WHERE angle < -25 AND angle > -35");
                    int dist_array_rows_1 = sqlite_get_rows("SELECT * from dist_contours_1 WHERE angle < -25 AND angle > -35");
                    for (int i=0; i<dist_array_rows_1; i++) {
                        y_mean_1_adjusted = y_mean_1_adjusted + dist_array_1[i][1] + (dist_array_1[i][3] - (-30))/tempHR;
                    }
                    y_mean_1_adjusted = y_mean_1_adjusted / dist_array_rows_1;

                    aod_left = VIEW_WIDTH/IMAGE_WIDTH * (y_mean_0_adjusted - y_mean_1_adjusted) * HR;
                    aod_results[0] = aod_left;

                    free(dist_array_1);

                    // 0 deg target
                    double x_mean_t1_0 = sqlite_query_value("SELECT avg(angle) from dist_contours_0 WHERE angle < 10 AND angle > -10");
                    double y_mean_t1_0 = sqlite_query_value("SELECT avg(dist) from dist_contours_0 WHERE angle < 10 AND angle > -10");
                    double count_t1_0 = sqlite_query_value("SELECT count(dist) from dist_contours_0 WHERE angle < 10 AND angle > -10");

                    double x_mean_t1_1 = sqlite_query_value("SELECT avg(angle) from dist_contours_1 WHERE angle < 10 AND angle > -10");
                    double y_mean_t1_1 = sqlite_query_value("SELECT avg(dist) from dist_contours_1 WHERE angle < 10 AND angle > -10");
                    double count_t1_1 = sqlite_query_value("SELECT count(dist) from dist_contours_1 WHERE angle < 10 AND angle > -10");

                    double count_t1 = count_t1_0 + count_t1_1;
                    double x_mean_t1 = ((x_mean_t1_0 * count_t1_0) + (x_mean_t1_1 * count_t1_1)) / (count_t1);
                    double y_mean_t1 = ((y_mean_t1_0 * count_t1_0) + (y_mean_t1_1 * count_t1_1)) / (count_t1);

                    double** contourValues_t1 = sqlite_query_to_array("SELECT * FROM dist_contours_0 WHERE angle < 10 AND angle > -10 UNION ALL SELECT * FROM dist_contours_1 WHERE angle < 10 AND angle > -10");

                    double* linRegVals_t1 = linReg(contourValues_t1, x_mean_t1, y_mean_t1, count_t1);

                    aod_center = VIEW_WIDTH/IMAGE_WIDTH * linRegVals_t1[1] * HR;
                    aod_results[1] = aod_center;

                    // 30 deg target
                    y_mean_0_adjusted = 0; // = y_mean_t2_0 + (x_mean_t2_0 - (30))/tempHR;
                    y_mean_1_adjusted = 0; // = y_mean_t2_1 + (x_mean_t2_1 - (30))/tempHR;

                    dist_array_0 = sqlite_query_to_array("SELECT * from dist_contours_0 WHERE angle < 35 AND angle > 25");
                    dist_array_rows_0 = sqlite_get_rows("SELECT * from dist_contours_0 WHERE angle < 35 AND angle > 25");
                    for (int i=0; i<dist_array_rows_0; i++) {
                        y_mean_0_adjusted = y_mean_0_adjusted + dist_array_0[i][1] + (dist_array_0[i][3] - (-30))/tempHR;
                    }
                    y_mean_0_adjusted = y_mean_0_adjusted / dist_array_rows_0;

                    dist_array_1 = sqlite_query_to_array("SELECT * from dist_contours_1 WHERE angle < 35 AND angle > 25");
                    dist_array_rows_1 = sqlite_get_rows("SELECT * from dist_contours_1 WHERE angle < 35 AND angle > 25");
                    for (int i=0; i<dist_array_rows_1; i++) {
                        y_mean_1_adjusted = y_mean_1_adjusted + dist_array_1[i][1] + (dist_array_1[i][3] - (-30))/tempHR;
                    }
                    y_mean_1_adjusted = y_mean_1_adjusted / dist_array_rows_1;

                    aod_right = VIEW_WIDTH/IMAGE_WIDTH * (y_mean_0_adjusted - y_mean_1_adjusted) * HR;
                    aod_results[2] = aod_right;
                }

                fprintf(statistics,"Linear Regression Equations: y = bx + a\n");
                fprintf(statistics,"#0: b = %f\t a = %f\t SE = %f\n",linRegVals_0[0],linRegVals_0[1],linRegVals_0[2]);
                fprintf(statistics,"#1: b = %f\t a = %f\t SE = %f\n",linRegVals_1[0],linRegVals_1[1],linRegVals_1[2]);

                fprintf(statistics,"\nCalculated HR Values and AOD:\n");
                fprintf(statistics,"HR_0 = %f PD/mm\n", HR_0);
                fprintf(statistics,"HR_1 = %f PD/mm\n", HR_1);

                // Angle of deviation values
                fprintf(statistics,"\nAngle of Deviation (-30 deg target): %f PD\n",aod_left);
                fprintf(statistics,"\nAngle of Deviation (0 deg target): %f PD\n",aod_center);
                fprintf(statistics,"\nAngle of Deviation (30 deg target): %f PD\n",aod_right);

                delete[] linRegVals_0;
                delete[] linRegVals_1;

                delete[] clusterSort;
                delete[] contourValues_0;
                delete[] contourValues_1;
        }

        fprintf(error_log,"Finished executing main\n");
        printf("End of Program\n");

        time_t t_end = time(NULL);
        fprintf(statistics,"\nTotal time for program: %f sec\n", (double)(t_end-t_begin));

        progressBar->setValue(100);
        qApp->processEvents();

        if (HRCalculation) {
            fprintf(dist_contours_0_filtered,"fNum\tdist\tcluster\tangle\n");
            sqlite_output_table("SELECT * FROM dist_contours_0",dist_contours_0_filtered);

            fprintf(dist_contours_1_filtered,"fNum\tdist\tcluster\tangle\n");
            sqlite_output_table("SELECT * FROM dist_contours_1",dist_contours_1_filtered);
        }
        recordLogs();

        return aod_results;
}

void recordLogs() {
        fprintf(stdout,"\nStatistical Summary:\n");

        // total frames analyzed
        int nframes = MAX_FRAME-MIN_FRAME;

        fprintf(stdout,"total frames : %i\n", nframes );

        // number of paired PRs
        string query = "SELECT count(*) from clustered_pr_contours WHERE cluster = 0";
        double count_0 = sqlite_query_value(query);

        query = "SELECT count(*) from clustered_pr_contours WHERE cluster = 1";
        double count_1 = sqlite_query_value(query);

        float percentPR_0 = 100*count_0/nframes;
        float percentPR_1 = 100*count_1/nframes;

    sqlite_print_table_info(stdout);
    sqlite_print_table_info(statistics);

        fprintf(statistics,"\npercent good PRs: Right = %f, Left = %f\n", percentPR_0, percentPR_1);
        fprintf(stdout,"\npercent good PRs: Right = %f, Left = %f\n", percentPR_0, percentPR_1);

        // Save down databases to txt files
        fprintf(pr_contours,"fNum\tx\ty\tw\th\tcluster\n");
        sqlite_output_table("SELECT * FROM pr_contours",pr_contours);

        // Save down databases to txt files
        if (prFilter) {
                fprintf(clustered_pr_contours,"fNum\tx\ty\tw\th\tcluster\n");
                sqlite_output_table("SELECT * FROM clustered_pr_contours",clustered_pr_contours);
        }

        if (pupilFilter) {
                fprintf(clustered_pupil_contours,"fNum\tx\ty\tw\th\tcluster\n");
                sqlite_output_table("SELECT * FROM clustered_pupil_contours",clustered_pupil_contours);
        }

    fprintf(pupil_contours_right,"fNum\tx\ty\tw\th\tcluster\n");
    sqlite_output_table("SELECT * FROM pupil_contours WHERE cluster = 0",pupil_contours_right);

    fprintf(pupil_contours_left,"fNum\tx\ty\tw\th\tcluster\n");
    sqlite_output_table("SELECT * FROM pupil_contours WHERE cluster = 1",pupil_contours_left);

    // Release the SQLite database
    fprintf(error_log,"Releasing database\n");
    sqlite3_close(db);

    // Close text files
    fclose(error_log);
    fclose(pr_contours);
    fclose(pr_contours_expanded);
    fclose(clustered_pr_contours);
    fclose(clustered_pupil_contours);
    fclose(pupil_contours_right);
    fclose(pupil_contours_left);
    fclose(dist_contours_0);
    fclose(dist_contours_1);
    fclose(dist_contours_0_filtered);
    fclose(dist_contours_1_filtered);
    fclose(statistics);
    fclose(TEMP_expanded_pr);
}

void ImageAnalysisPR()
{
        char* filename;

        for (int frameNum = MIN_FRAME; frameNum <= MAX_FRAME; frameNum++) {
                // Switch to frame directory
                chdir(inputDir);

                // Generate input filename
                filename = new char[255];
                sprintf(filename, "%i.bmp", frameNum);

                // Import original frame
                Mat img_face = imread(filename,0);
                if(img_face.data != NULL) {

                    locatePurkinje(img_face,frameNum);

                    //	Release memory allocations
                    img_face.release();
                }
                else {
                    fprintf(error_log,"Could not load image file for PR Analysis: %s\n",filename);

                }
                delete[] filename;

                progressBar->setValue((int)(73*((float)(frameNum-MIN_FRAME) / (float)(MAX_FRAME-MIN_FRAME))));
                qApp->processEvents();
        }
}

// function ImageAnalysisPupil
// uses table clustered_pr_contours in image analysis
// to find pupils
void ImageAnalysisPupil()
{
    // used to store sql queries
    char* cmd;
    // used to store filename where images will be stored
    char *filename;
    // used to load image data from file
    Mat img_face;

    // For all the image frames
    for (int frameNum = MIN_FRAME; frameNum <= MAX_FRAME; frameNum++) {

        cmd = new char[255];
        sprintf(cmd, "SELECT * FROM clustered_pr_contours WHERE frameNumber = %i", frameNum);
        // get the number of PR contours in the frame
        int numRows = sqlite_get_rows(cmd);

        // get the number of columns in the result
        int numCols = sqlite_get_columns(cmd);
        // get the values of the PR contours in the frame
        //double** prContours = sqlite_query_to_array(cmd);
        delete[] cmd;

        // make sure the query has enough columns at least 5
        // needed for the pupil location to work
        if( numCols >= 5 && numRows == 2 ) {
            for( int cluster = 0 ;  cluster <= 1 ; cluster++) {
                /* Perform pupil analysis routine */
                chdir(inputDir);

                // Generate input filename
                filename = new char[255];
                sprintf(filename, "%i.bmp", frameNum);

                // Import original frame
                Mat img_face = imread(filename,0);
                if(img_face.data != NULL) {

                        double pr_x = 0;
                        double pr_y = 0;

                        // get contour information for the current contour
                        cmd = new char[255];
                        sprintf(cmd, "SELECT x from clustered_pr_contours WHERE frameNumber = %i AND cluster = %i",frameNum,cluster);
                        pr_x = sqlite_query_value(cmd);
                        delete[] cmd;

                        cmd = new char[255];
                        sprintf(cmd, "SELECT y from clustered_pr_contours WHERE frameNumber = %i AND cluster = %i",frameNum,cluster);
                        pr_y = sqlite_query_value(cmd);
                        delete[] cmd;

                        // testing: display the current frame counter and
                        // current pupil contour number for the frame
                        printf("\nFRAME %i, CLUSTER %i\n", frameNum, cluster);

                        // locate the pupil with the current pr contour information
                        fprintf(stderr,"\n PR VALUES: %f, %f",pr_x,pr_y);
                        locatePupil(img_face, pr_x, pr_y, frameNum, cluster);

                        //	Release memory allocations
                        img_face.release();
                }
                else {
                        fprintf(error_log,"Could not load image file for PR Analysis: %s\n",filename);

                }
                delete[] filename;

            } // end inner for (for all contours)
        } // end if (column size constraint)
        //delete[] prContours;

        progressBar->setValue((int)(75+22*((float)(frameNum-MIN_FRAME) / (float)(MAX_FRAME-MIN_FRAME))));
        qApp->processEvents();

    } // end outer for (for all frames)

    return;
}

/*
 * Function LocatePurjinke: finds PRs of the whole image, replacing the Haar Cascade
 *
 */

// TODO: some PR contours are rejected because the patients eyes are moving, causing the contour to be blurred
// investigate a way to fix this.
// TODO: investigate use of uchar binary images 8UCI for all images
void locatePurkinje(Mat img_face, int frameNum) {

    /* FFT Determination of distance

    // Determine the distance of the patient.

            // Pad the image to multiple of 2,3, and 5 to speed up DFT
            Mat padded; //expand input image to optimal size

            int m = getOptimalDFTSize( img_face.rows );
            int n = getOptimalDFTSize( img_face.cols ); // on the border add zero pixels
            copyMakeBorder(img_face, padded, 0, m - img_face.rows, 0, n - img_face.cols, BORDER_CONSTANT, Scalar::all(0));

            // DFT returns complex number, store as two planes
            Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
            Mat complex_img_face;
            merge(planes, 2, complex_img_face);         // Add to the expanded another plane with zeros

            // Actual transform (dicrete fourier transform)
            dft(complex_img_face, complex_img_face);            // this way the result may fit in the source

            split(complex_img_face, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
            magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
            Mat dftmag_img_face = planes[0];

            // Switch to log to compress range
            dftmag_img_face += Scalar::all(1);                    // switch to logarithmic scale
            log(dftmag_img_face, dftmag_img_face);

            // Remove the values from padding (Crop image)
            dftmag_img_face = dftmag_img_face(Rect(0, 0, dftmag_img_face.cols & -2, dftmag_img_face.rows & -2));
            int cx = dftmag_img_face.cols/2;
            int cy = dftmag_img_face.rows/2;

            Mat q0(dftmag_img_face, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
            Mat q1(dftmag_img_face, Rect(cx, 0, cx, cy));  // Top-Right
            Mat q2(dftmag_img_face, Rect(0, cy, cx, cy));  // Bottom-Left
            Mat q3(dftmag_img_face, Rect(cx, cy, cx, cy)); // Bottom-Right

            Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
            q0.copyTo(tmp);
            q3.copyTo(q0);
            tmp.copyTo(q3);

            q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
            q2.copyTo(q1);
            tmp.copyTo(q2);

            // For display, normalize
            normalize(dftmag_img_face, dftmag_img_face, 0, 255, CV_MINMAX);

            saveImage("pr_contours_frame_%i_DFT.jpg", prDir, type , dftmag_img_face);

            // Initialize bins for determining focus
            double pixDist, pixValue;

            int NUM_RADIAL_BINS = 3;
            double *fft_bins_sum = new double[(img_face.rows/2) / 3];
            int *fft_bins_count = new int[(img_face.rows/2) / 3];

            for (int i=0; i<NUM_RADIAL_BINS; i++) {
                    fft_bins_sum[i] = 0;
                    fft_bins_count[i] = 0;
            }

            // Bin the DFT values using radial averaging
            for (int i=0; i<img_face.rows - 1; i++) {
                    for (int j=((img_face.cols / 2) - (img_face.rows / 2)) ; j < ((img_face.cols / 2) + (img_face.rows / 2)); j++) {

                            pixDist = sqrt(pow((double)(img_face.rows/2-i),2)+pow((double)(img_face.cols/2-j),2));

                            int binNum = floor(pixDist / ((img_face.rows/2) / NUM_RADIAL_BINS));
                            if (binNum >= 0 && binNum < NUM_RADIAL_BINS) {
                                    pixValue = ELEM(float,dftmag_img_face.data,dftmag_img_face.step,dftmag_img_face.elemSize(),j,i);

                                    fft_bins_sum[binNum] += pixValue;
                                    fft_bins_count[binNum]++;
                            }
                    }
            }
            for (int i=0; i<NUM_RADIAL_BINS; i++) {
                    fft_bins_sum[i] = fft_bins_sum[i] / fft_bins_count[i];
                    fprintf(stderr,"\nRadial Bin #%i: %f, Count = %i",i,fft_bins_sum[i],fft_bins_count[i]);
            }
*/

        // Gaussian Filter to remove noise
        Mat img_eye_gaussian = Mat(img_face.size(),img_face.type());
        double sigma = 2.5;
        // calculate the kernel size in terms of sigma
        int kSize =  round(2*((sigma-0.8)/.3 + 1)+1);
        // make sure that the kernel size is odd
        kSize = kSize + (1 - kSize % 2);
        Mat gaussianK = getGaussianKernel(kSize,-1,CV_32F);
        sepFilter2D(img_face,img_eye_gaussian,CV_32F,gaussianK,gaussianK);

        // saveImage("pr_contours_frame_%i_1_gauss.jpg", prDir, type , img_eye_gaussian);

        Mat img_eye_laplacian = Mat(img_face.size(),img_face.type());
        Laplacian(img_eye_gaussian, img_eye_laplacian, CV_32F, 7 );

        img_eye_laplacian = abs(img_eye_laplacian);
        normalize(img_eye_laplacian,img_eye_laplacian,0,255,NORM_MINMAX);

        Mat img_eye_contour = Mat(img_face.size(),img_face.type());

        if (frameNum < 40) {
                // Initialize bins for binary thresholding
                int counter = 0;
                float min, max, avgPixel;
                min = -1; max = -1; avgPixel = 0;
                int BIN_SIZE = 1;
                int *bins = new int[255 / BIN_SIZE]; // bin sizes of 5 --> 51 total bins
                for (int i=0; i<51; i++) {
                        bins[i] = 0;
                }

                // Bin the laplacian values
                for (int i=1; i<img_face.rows - 1; i++) {
                        for (int j=1; j<img_face.cols - 1; j++) {
                                double val = ELEM(float,img_eye_laplacian.data,img_eye_laplacian.step,img_eye_laplacian.elemSize(),j,i);

                                if (val < min || min == -1) {
                                        min = val;
                                }
                                if (val > max || max == -1) {
                                        max = val;
                                }
                                avgPixel = avgPixel + val;
                                counter++;

                                float binVal = (int)val / BIN_SIZE;
                                int binFloor = floor(binVal);
                                if (binFloor > ((255 / BIN_SIZE) - 1)) {
                                        binFloor = ((255 / BIN_SIZE) - 1);
                                }
                                else if (binFloor < 0) {
                                        binFloor = 0;
                                }
                                bins[binFloor]++;
                        }
                }
                avgPixel = avgPixel / counter;
                fprintf(stderr,"\nValues: %f, %f, %f\n",min,max,avgPixel);

                // Find optimal bin for thresholding
                float percentage = 0;
                float sum = 0;
                float TOTAL_PIXELS = (img_face.rows - 2) * (img_face.cols - 2);
                int binNum=0;
                while (percentage < .9999) {
                        sum = sum + bins[binNum];
                        percentage = sum / TOTAL_PIXELS;
                        binNum++;
                }
                printf("\nThreshold Value: %f",(float)binNum*BIN_SIZE);

                float tempThresh = binNum*BIN_SIZE;
                if (tempThresh < MIN_THRESH || MIN_THRESH == -1) {
                        MIN_THRESH = tempThresh;
                }
                if (tempThresh > MAX_THRESH || MAX_THRESH == -1) {
                        MAX_THRESH = tempThresh;
                }
                COUNT_THRESH++;
                MEAN_THRESH = (MEAN_THRESH*(COUNT_THRESH - 1) + tempThresh) / COUNT_THRESH;
        }
        fprintf(stdout,"\n MIN_THRESH = %f, MAX_THRESH = %f, MEAN_THRESH = %f\n",MIN_THRESH,MAX_THRESH,MEAN_THRESH);

        // Binary thresholding of image for contouring
        threshold( img_eye_laplacian, img_eye_contour, MEAN_THRESH ,255, CV_THRESH_BINARY );

        img_eye_contour.convertTo(img_eye_contour, CV_8UC1);

        // Save binary thresholded image
        saveImage("pr_contours_frame_%i_binary.jpg", prDir , frameNum, img_eye_contour);

        // PR Filter Parameters
        float 	MIN_SIZE_PR 			= 	2;
        float 	MAX_SIZE_PR 			=	20;
        float 	MIN_ECCENTRICITY_PR  	=	0.5;
        float 	MAX_ECCENTRICITY_PR  	=	1.2;

        // CONTOURS PURKINJE
        vector<RotatedRect>::iterator it;
        vector<RotatedRect> pr_contours = getContours(img_eye_contour, MIN_SIZE_PR, MAX_SIZE_PR, MIN_ECCENTRICITY_PR, MAX_ECCENTRICITY_PR);

        char* cmd;

        if(pr_contours.size() == 0 ) {
            printf("Frame %i ; found 0 PR contours\n",frameNum);
            fprintf(error_log,"Frame %i ; found 0 PR contours\n",frameNum);
        }
        else if (pr_contours.size() < 10) {
            for(it = pr_contours.begin(); it < pr_contours.end() ; it++) {
                // PAINT COUNTOUR ON IMAGE
                ellipse(img_face, *it, Scalar(255,0,0), 1, 8);
                //ellipse(img_face,(*it).center,(*it).size, 0,0,0,CV_RGB(255,255,255));

                // ******************** INSERT INTO SQLITE DATABASE ************************
                cmd = new char[255];
                sprintf(cmd, "REPLACE into pr_contours ( frameNumber, x, y, w, h) values (%i,%f,%f,%f,%f)",
                        frameNum,(*it).center.x, (*it).center.y, (*it).size.width, (*it).size.height);
                sqlite_command(cmd);
                delete[] cmd;

                // ********************* PRINT TO TXT FILE **************************
                fprintf(stderr, "PR: %i\t%f\t%f\t%f\t%f\t\n",frameNum,(*it).center.x, (*it).center.y, (*it).size.width, (*it).size.height);
            }
        }
        else if(pr_contours.size() >= 10) {
            printf("Frame %i ; found %i PR contours\n",frameNum,pr_contours.size());
            fprintf(error_log,"Frame %i ; found %i PR contours\n",frameNum,pr_contours.size());
        }

        // Save image
        saveImage("pr_contours_frame_%i.jpg", prDir , frameNum, img_face);

        return;
}

void printMat(Mat m) {
        for( int row = 0; row < m.rows; row++ )
                {
                for( int col = 0; col < m.cols; col++ )
                {
                        fprintf(stdout,"%f ",m.at<float>(row,col));
                }
                fprintf(stdout,"\n");
        }

        return;
}

void locatePupil( Mat img_face, float PR_x, float PR_y, int frameNum, int cluster) {

        // Set save directory based on eye
        char* saveDir;
        if (cluster == 0) {
                saveDir = rightDir;
        }
        else if (cluster == 1) {
                saveDir = leftDir;
        }
        else {
                fprintf(error_log,"ERROR: Not valid cluster for locatePupil\n");
                fprintf(stderr,"ERROR: Not valid cluster for locatePupil\n");
                saveDir = rightDir;
        }

        // SAMPLING PARAMETERS
        int	ROI_SIZE = 129;
        Mat img_eye = getROI(img_face, PR_x, PR_y, ROI_SIZE);

        Mat thresholded;

        double meanVal = mean(img_eye)[0];
        double maxVal;
        double minVal;
        minMaxLoc(img_eye,&minVal,&maxVal);

        double thresh = meanVal + ( maxVal - meanVal ) * 0.5;
        threshold(img_eye,thresholded,thresh,0,THRESH_TOZERO_INV);

        // Gaussian Filter to remove noise
        Mat gaussian;
        double sigma = 3.5;
        // calculate the kernel size in terms of sigma
        int kSize =  round(2*((sigma-0.8)/.3 + 1)+1);
        // make sure that the kernel size is odd
        kSize = kSize + (1 - kSize % 2);
        Mat gaussianK = getGaussianKernel(kSize,-1,CV_32F);
        sepFilter2D(img_eye,gaussian,CV_32F,gaussianK,gaussianK);

        //saveImage("frame_%i_1_gaussian.jpg", saveDir, 0 , gaussian);

        int rows = gaussian.rows;
        int cols = gaussian.cols;

        // Find x derivative
        Mat dx(rows,cols,CV_64F);
        for (int i=0; i<rows; i++) {
            for (int j=0; j<cols; j++) {
                if (j == 0) {
                    double val = ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j+1,i) - ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i);
                    dx.at<double>(i,j) = val;
                }
                else if (j>0 && j<cols-1){
                    double val = ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j+1,i) - ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j-1,i);
                    dx.at<double>(i,j) = val;
                }
                else { // j = width-1 (end of array)
                    double val = ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i) - ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j-1,i);
                    dx.at<double>(i,j) = val;
                }
            }
        }

/*	namedWindow("Display",CV_WINDOW_AUTOSIZE);
        imshow("Display", dx);
        waitKey(0);*/

        //saveImage("frame_%i_2_dx.jpg", saveDir, 0 , dx);

        // Find y derivative
        Mat dy(rows,cols,CV_64F);
        for (int j=0; j<cols; j++) {
            for (int i=0; i<rows; i++) {
                if (i == 0) {
                    double val = ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i+1) - ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i);
                    dy.at<double>(i,j) = val;
                }
                else if (i>0 && i<rows-1){
                    double val = ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i+1) - ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i-1);
                    dy.at<double>(i,j) = val;
                }
                else { // j = width-1 (end of array)
                    double val = ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i) - ELEM(float,gaussian.data,gaussian.step,gaussian.elemSize(),j,i-1);
                    dy.at<double>(i,j) = val;
                }
            }
        }

/*	imshow("Display", dy);
        waitKey(0);*/

        //saveImage("frame_%i_3_dy.jpg", saveDir, 0 , dy);

        // Find magnitude
        Mat magnitude(rows,cols,CV_64F);
        for (int i=0; i<rows; i++) {
            for (int j=0; j<cols; j++) {
                double dx_val = ELEM(double,dx.data,dx.step,dx.elemSize(),j,i);
                double dy_val = ELEM(double,dy.data,dy.step,dy.elemSize(),j,i);
                magnitude.at<double>(i,j) = (double)sqrt( pow(dx_val,2) + pow(dy_val,2));
            }
        }

/*	imshow("Display", magnitude);
        waitKey(0);*/

        //saveImage("frame_%i_4_magnitude.jpg", saveDir, 0 , magnitude);

        // Find direction in radians
        Mat direction(rows,cols,CV_64F);
        for (int i=0; i<rows; i++) {
            for (int j=0; j<cols; j++) {
                double dx_val = ELEM(double,dx.data,dx.step,dx.elemSize(),j,i);
                double dy_val = ELEM(double,dy.data,dy.step,dy.elemSize(),j,i);

                if (dx_val == 0 && dy_val == 0) {
                    direction.at<double>(i,j) = 0;
                }
                else {
                    direction.at<double>(i,j) = atan(dy_val / dx_val);
                }

                double dir_val = ELEM(double,direction.data,direction.step,direction.elemSize(),j,i);

                if (dx_val >= 0) {
                    if (dy_val < 0) {
                        direction.at<double>(i,j) = 2*M_PI - dir_val;
                    }
                }
                else {
                    if (dy_val >= 0) {
                        direction.at<double>(i,j) = M_PI - dir_val;
                    }
                    else {
                        direction.at<double>(i,j) = M_PI + dir_val;
                    }
                }
            }
        }

        // Non-maximal suppression
        Mat nms(rows,cols,CV_64F);
        for (int i=0; i<rows; i++) {
            for (int j=0; j<cols; j++) {
                if (i==0 || j==0) {
                    nms.at<double>(i,j) = 0;
                }

                // ERROR: sometimes the value pulled from here is extremely high...
                double dir_val = ELEM(double,direction.data,direction.step,direction.elemSize(),j,i) * 180 / M_PI;

                if (dir_val > 360) {
                    dir_val = dir_val - 360;
                }
                else if (dir_val < 0) {
                    dir_val = dir_val + 360;
                }

                double diff1 = 0;
                double diff2 = 0;

                double dx_val = ELEM(double,dx.data,dx.step,dx.elemSize(),j,i);
                double dy_val = ELEM(double,dy.data,dy.step,dy.elemSize(),j,i);

                double slp = dy_val/dx_val;
                double invslp = 1/slp;

                double val = ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j,i);

                if ( (dir_val >= 0 && dir_val < 45) || (dir_val >= 180 && dir_val < 225) ) {
                    diff1 = slp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j+1,i+1)) + (1-slp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j+1,i));
                    diff2 = slp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j-1,i-1)) + (1-slp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j-1,i));
                }
                else if ( (dir_val >= 45 && dir_val < 90) || (dir_val >= 225 && dir_val < 270) ) {
                    diff1 = invslp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j+1,i+1)) + (1-invslp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j,i+1));
                    diff2 = invslp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j-1,i-1)) + (1-invslp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j,i-1));
                }
                else if ( (dir_val >= 90 && dir_val < 135) || (dir_val >= 270 && dir_val < 315) ) {
                    diff1 = invslp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j-1,i+1)) + (1-invslp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j,i+1));
                    diff2 = invslp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j+1,i-1)) + (1-invslp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j,i-1));
                }
                else if ( (dir_val >=135 && dir_val < 180) || (dir_val >= 315 && dir_val <= 360) ) {
                    diff1 = slp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j-1,i+1)) + (1-slp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j-1,i));
                    diff2 = slp * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j+1,i-1)) + (1-slp) * (val-ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j+1,i));
                }
                else {
                    //printf("\n ERROR Current degree: %f",dir);
                    diff1 = -1;
                    diff2 = -1;
                }

                if (diff1 >= 0 && diff2 >= 0) {
                    // Possible edge detected
                    nms.at<double>(i,j) = POSSIBLE_EDGE;
                }
                else {
                    // No edge detected
                    nms.at<double>(i,j) = NO_EDGE;
                }

                /*
                float tempVal;
                // Throw out any values that were originally thresholded out
                for (int m = -5; m <= 5; m++) {
                        for (int n = -5; n <= 5; n++) {
                        tempVal = ELEM(uchar,thresholded.data,thresholded.step,thresholded.elemSize(),i+m,j+n);
                        if (tempVal == 0) {
                                nms.at<double>(j,i) = NO_EDGE;
                        }
                    }
                }
                */
            }
        }

/*	imshow("Display", nms);
        waitKey(0);*/

    saveImage("frame_%i_5_nms.jpg", saveDir, frameNum, nms);

    Mat edges(rows,cols,CV_64F);
    // Set border pixels for image = 0
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            if ( i == 0 || i == rows - 1 || j == 0 || j == cols - 1 ) {
                edges.at<double>(i,j) = NO_EDGE;
            }
            else {
                edges.at<double>(i,j) = ELEM(double,nms.data,nms.step,nms.elemSize(),j,i);
            }
        }
    }

    vector<float>* possibleEdgeMagnitudes = new vector<float>();

    // add all possible edge magnitudes to the vector
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            if (ELEM(double,edges.data,edges.step,edges.elemSize(),j,i) == POSSIBLE_EDGE) {
                possibleEdgeMagnitudes->push_back(ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j,i));
            }
        }
    }

    // sort the vector according to magnitudes
    sort(possibleEdgeMagnitudes->begin(), possibleEdgeMagnitudes->end());

    // get the number of edges
    int totalCount = possibleEdgeMagnitudes->size();
    float thresholdMagnitude = 0;

    float tLow = 0.2; // range from 0 to 1 TEMP 0.2
    float tHigh = 0.81; // range from 0 to 1 TEMP 0.61

    if (totalCount > 0) {
            // get the maximum magnitude
            float maximumMagnitude = possibleEdgeMagnitudes->back();

            // count the number of magnitudes that are:
            // less than the maximum magnitude
            // with a percentage of the total magnitude
            int percentageIndex = round(totalCount*tHigh);

            float percentageMagnitude = possibleEdgeMagnitudes->at(percentageIndex);
                     thresholdMagnitude = 0; //

            if ( percentageMagnitude < maximumMagnitude ) {
                    thresholdMagnitude = percentageMagnitude;
            }
            // if the value is too large, find the first index less than the value
            else {
                    // lower_bound returns an iterator to the lowest magnitude
                    // subtracting off the vector.begin() values gives its index
                    // subtracting 1 gives the value smaller than this index
                    fprintf(stderr,"percentage value too high!\n");
                    int thresholdIndex = int(lower_bound( possibleEdgeMagnitudes->begin(), possibleEdgeMagnitudes->end(), maximumMagnitude ) -
                                    possibleEdgeMagnitudes->begin()) - 1;
                    // verify index is non-negative
                    if( thresholdIndex < 0 ) {
                            thresholdIndex = 0;
                    }
                    thresholdMagnitude = possibleEdgeMagnitudes->at(thresholdIndex);
            }
    }

    // remove the edges vector
    possibleEdgeMagnitudes->clear();
    //free(possibleEdgeMagnitudes);

    int edgeCounter = 0; // want to be near 1000
    int desiredEdges = 200; // total number of edges desired in image
    int desiredEdgesError = .2 * desiredEdges; // amount of edges that the image can vary by (1000 +/- 200)
    float highT_upperBound = 10; // varies the high threshold
    float highT_lowerBound = 0;
    float highT = 1;
    float highThreshold;
    float lowThreshold;

    int iteratorCounter = 0;
    int tempEdgeCounter = -1;

    while ( abs(edgeCounter-desiredEdges) > desiredEdgesError && highT > 0.0001 && iteratorCounter < 10 ) {
        if (edgeCounter == tempEdgeCounter) {
            iteratorCounter++;
        }
        else {
            iteratorCounter = 0;
        }
        tempEdgeCounter = edgeCounter;

        // Set new thresholds
        highThreshold = thresholdMagnitude*highT;
        lowThreshold = highThreshold * tLow;

        // Double thresholding
        for (int i=1; i<rows-1; i++) {
            for (int j=1; j<cols-1; j++) {
                if ( (ELEM(double,edges.data,edges.step,edges.elemSize(),j,i) == POSSIBLE_EDGE) && (ELEM(double,magnitude.data,magnitude.step,magnitude.elemSize(),j,i) >= highThreshold) ) {
                    edges.at<double>(i,j) = EDGE;
                    followEdges(magnitude,i,j,lowThreshold);
                }
            }
        }
        edgeCounter = 0;
        for (int i=0; i<rows; i++) {
            for (int j=0; j<cols; j++) {
                if (ELEM(double,edges.data,edges.step,edges.elemSize(),j,i) == EDGE) {
                    edgeCounter++;
                }
            }
        }
        // adjust thresholding using binary algorithm
        // decreasing highT will increase the number of edges
        if ( edgeCounter < desiredEdges) {
            float temp = (highT_upperBound + highT_lowerBound )/ 2;
            highT_upperBound = highT;
            highT = temp;

        }
        else {
            float temp = (highT_upperBound + highT_lowerBound )/ 2;
            highT_lowerBound = highT;
            highT = temp;
        }
    }

    // Set all remaining POSSIBLE_EDGE to NO_EDGE
    for (int i=1; i<rows-1; i++) {
        for (int j=1; j<cols-1; j++) {
            if (ELEM(double,edges.data,edges.step,edges.elemSize(),j,i) != EDGE) {
                edges.at<double>(i,j) = NO_EDGE;
            }
        }
    }

    saveImage("frame_%i_6_edges.jpg", saveDir, frameNum, edges);

    // Pupil Filter Parameters
    float 	MIN_SIZE_PUPIL 			= 	30;
    float 	MAX_SIZE_PUPIL 			=	60;
    float 	MIN_ECCENTRICITY_PUPIL  	=	0.5;
    float 	MAX_ECCENTRICITY_PUPIL  	=	2;

    edges.convertTo(edges, CV_8UC1);
    normalize(img_eye,img_eye,0,1500,NORM_MINMAX);

    // CONTOURS PUPIL
    vector<RotatedRect>::iterator it;
    vector<RotatedRect> pupil_contours = getContours(edges, MIN_SIZE_PUPIL, MAX_SIZE_PUPIL, MIN_ECCENTRICITY_PUPIL, MAX_ECCENTRICITY_PUPIL);

    char* cmd;

    if(pupil_contours.size() == 0 ) {
        printf("Frame %i ; found 0 pupil contours\n",frameNum);
        fprintf(error_log,"Frame %i ; found 0 pupil contours\n",frameNum);
    }
    else if (pupil_contours.size() < 10) {
        for(it = pupil_contours.begin(); it < pupil_contours.end() ; it++) {

            // pull out center of ellipse contour
            Point center(cvRound((*it).center.x), (*it).center.y);

            // draw ellipse on the eye image
            ellipse(img_eye, *it, Scalar(255,0,0), 1, 8);

            // add the contours to the pupil contours table
            cmd = new char[255];
            fprintf(stderr,"\n%f, %f",(float)(center.x + PR_x - ROI_SIZE/2), PR_x);
            sprintf(cmd, "REPLACE INTO pupil_contours (frameNumber,x,y,w,h) VALUES (%i, %f, %f, %f, %f)",frameNum,(float)(center.x + PR_x - ROI_SIZE/2),(float)(center.y + PR_y - ROI_SIZE/2),(*it).size.width,(*it).size.height);
            sqlite_command(cmd);

            fprintf(stderr,"\n Location of contour: %f, %f, %f, %f\n",(*it).center.x,(*it).center.y,(*it).size.width,(*it).size.height);
        }

        saveImage("frame_%i_7_hough.jpg", saveDir, frameNum, img_eye);
    }
    else if(pupil_contours.size() >= 10) {
        printf("Frame %i ; found %i pupil contours\n",frameNum,pupil_contours.size());
        fprintf(error_log,"Frame %i ; found %i pupil contours\n",frameNum,pupil_contours.size());
    }

        //	Release memory allocations
        img_eye.release();
        gaussian.release();
        gaussianK.release();
        dx.release();
        dy.release();
        magnitude.release();
        direction.release();
        nms.release();
        edges.release();

        return;
}

/*
 * Method getROI
 * inputs: potential new centers
 * output: ROI submatrix
 */
Mat getROI(Mat img, float x, float y, int ROI_SIZE) {

        int h = ROI_SIZE/2;
        int w = ROI_SIZE/2;

        int minRow = y-h;
        int maxRow = y+h+(ROI_SIZE%2);
        int minCol = x-w;
        int maxCol = x+w+(ROI_SIZE%2);

        // adjust ROI size if ROI is out of range
        if( minRow < 0 )
        {
                ROI_offset_Y = ((float)ROI_SIZE / 2) - y;
                //ROI_offset_X = minRow;
                minRow = 0;
                maxRow = ROI_SIZE;
        }
        else if( maxRow > img.rows )
        {
                ROI_offset_Y = img.rows - y - ((ROI_SIZE + 1) / 2);
                //ROI_offset_Y = img.rows - maxRow;
                maxRow = img.rows - 1;
                minRow = maxRow - ROI_SIZE;
        }
        else {
                ROI_offset_Y = minRow - (y - ((float)ROI_SIZE / 2));
        }

        if( minCol < 0 )
        {
                ROI_offset_X = ((float)ROI_SIZE / 2) - y;
                //ROI_offset_Y = minCol;
                minCol = 0;
                maxCol = ROI_SIZE;
        }
        else if( maxCol > img.cols)
        {
                ROI_offset_X = img.cols - maxCol;
                maxCol = img.cols - 1;
                minCol = maxCol - ROI_SIZE;
        }
        else {
                ROI_offset_X = minCol - (x - ((float)ROI_SIZE / 2));
        }

        Mat ROI = img(Range(minRow,maxRow),Range(minCol,maxCol));

        return ROI;
}

/*
 * Method getContours
 * inputs: input Mat image and width/eccentricity ranges
 * output: vector of contours
 */
vector<RotatedRect> getContours( Mat input, float minWidth, float maxWidth, float minEccentricity, float maxEccentricity) {

        /* Contours for PR or Pupil Detection */
        vector< vector<Point> > contours ;
        vector< vector<Point> >::iterator it;

        findContours(input,contours,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);

        vector<RotatedRect> contourBox;

        for(size_t i = 0; i < contours.size(); i++)
        {
                size_t iNumPoints = contours[i].size();

                Mat pointsf;
                Mat(contours[i]).convertTo(pointsf, CV_32F);

                if ( iNumPoints >= 6)
                {
                        // filter the contour
                        // Continue if contour does not pass the filter
                        if( minSizeFilterContour( pointsf, minWidth) &&
                                        maxSizeFilterContour( pointsf, maxWidth) &&
                                        eccentricityFilterContour( pointsf, minEccentricity, maxEccentricity) )
                        {
                                contourBox.push_back(fitEllipse(pointsf));
                        }
                }
        }

        // return vector of contours
        return contourBox;
}

/*
 * Method saveImage
 * inputs: filename, directory, input Mat image
 * output: saves image to output folder
 */
void saveImage(const char* fName, const char* dName, int frameNum, Mat image) {
        // Update filename to include frame number
        char* filename = new char[255];
        sprintf(filename, fName, frameNum);

        chdir(dName);

        imwrite(filename, image );

        delete[] filename;

        return;
}

/*
 * Method minSizeFilterContour
 * inputs: potential contours and minimum contour width
 * output: boolean whether greater than minWidth
 */
bool minSizeFilterContour(Mat contour, float minWidth)
{
        bool passesFilter = true;

        // get bounding rect of this contour
        RotatedRect rect = fitEllipse(contour);

        // minimum size check
        if ( (rect.size.width < minWidth) || (rect.size.height < minWidth) )
        {
                passesFilter = false;
        }

        return passesFilter;

}

/*
 * Method maxSizeFilterContour
 * inputs: potential contours and maximum contour width
 * output: boolean whether less than maxWidth
 */
bool maxSizeFilterContour(Mat contour, float maxWidth)
{
        bool passesFilter = true;

        // get bounding rect of this contour
        RotatedRect rect = fitEllipse(contour);

        // maximum size check
        if ( (rect.size.width > maxWidth) || (rect.size.height > maxWidth))
        {
                passesFilter = false;
        }

        return passesFilter;

}

/*
 * Method eccentricityFilterContour
 * inputs: potential contours and eccentricity ranges
 * output: boolean whether passes eccentricity filter
 */
bool eccentricityFilterContour(Mat contour, float minEccentricity, float maxEccentricity)
{
        bool passesFilter = true;

        // get bounding rect of this contour
        RotatedRect rect = fitEllipse(contour);

        // ECCENTRICITY CHECK
        if (((float)rect.size.width/(float)rect.size.height > maxEccentricity) || ((float)rect.size.width/(float)rect.size.height < minEccentricity)) {
                passesFilter = false;
        }

        return passesFilter;

}

// auxiliary function to execute command in one line
// catches sqlite exceptions
// assumes global variables exist:
// db: pointer to sqlite3 database
// rc integer return status of sqlite statement
// print_query: static integer (specify output)
static void sqlite_command(string sql_cmd,FILE * file)
{
        zErrMsg = 0;

        rc = sqlite3_exec(db,sql_cmd.c_str(),print_query,file,&zErrMsg);

        sqlite_exception(rc);
}

// auxiliary function to output table to a tab-delimited txt file
// catches sqlite exceptions
// assumes global variables exist:
// db: pointer to sqlite3 database
// rc integer return status of sqlite statement
// print_query: static integer (specify output)
static void sqlite_output_table(string sql_cmd,FILE * file)
{
        zErrMsg = 0;

        rc = sqlite3_exec(db,sql_cmd.c_str(),print_query2,file,&zErrMsg);

        sqlite_exception(rc);
}

void filterContours(char* table) {

        char* cmd;
        cmd = new char[255];
        sprintf(cmd,"DROP TABLE IF EXISTS clustered_%s",table);
        sqlite_command(cmd);
        delete[] cmd;

        cmd = new char[255];
        sprintf(cmd,"CREATE TABLE IF NOT EXISTS clustered_%s AS SELECT * FROM %s",table, table);
        sqlite_command(cmd);
        delete[] cmd;

        cmd = new char[255];
        sprintf(cmd, "DROP TABLE IF EXISTS %s_expanded",table);
        sqlite_command(cmd);
        delete[] cmd;

        cmd = new char[255];
        sprintf(cmd, "CREATE TABLE IF NOT EXISTS %s_expanded (frameNumber INTEGER, x1 REAL, y1 REAL, w1 REAL, h1 REAL, x2 REAL, y2 REAL, w2 REAL, h2 REAL, dist REAL, cluster REAL)",table);
        sqlite_command(cmd);
        delete[] cmd;

        // Expand from single contours to pairs of contours within a given frame
        cmd = new char[500];
        sprintf(cmd, "REPLACE INTO %s_expanded (frameNumber, x1, y1, w1, h1, x2, y2, w2, h2, dist) SELECT l.frameNumber,l.x,l.y,l.w,l.h,r.x,r.y,r.w,r.h,sqrt((r.x-l.x)*(r.x-l.x)+(r.y-l.y)*(r.y-l.y)) AS dist FROM (SELECT * FROM clustered_%s) AS r INNER JOIN (SELECT * FROM clustered_%s) AS l WHERE r.frameNumber = l.frameNumber AND l.x < r.x",table,table,table);
        sqlite_command(cmd);
        delete[] cmd;

        // Only allow pairs of contours that are more than 200 pixels apart
        cmd = new char[255];
        sprintf(cmd, "DELETE FROM %s_expanded WHERE dist < 200", table);
        sqlite_command(cmd);
        delete[] cmd;

        /*
         * Start with extremely high number of clusters
         * Assumption: True PR pair should be the most common
         * Therefore, by "overclustering" the true cluster should appear
         */

        int nclusters = 20;
        int maxClusterCount = 0;
        int maxCluster;

        double mean, stdev;

        // Routine for pr_contours ONLY
        if (table == (char*)"pr_contours") {

                maxClusterCount = 0;
                maxCluster = 0;

                char* table_expanded = new char[255];
                sprintf(table_expanded, "%s_expanded",table);

                // Cluster all of the frames using x diff, y diff, area of each contour, and euclidean distance
                cmd = new char[255];
                sprintf(cmd, "SELECT x2-x1,y2-y1,w1*h1, w2*h2, dist FROM %s_expanded", table);
                cluster_table(cmd,table_expanded,nclusters);
                delete[] cmd;

                // Check each cluster to see if its the largest
                for (int i = 0; i < nclusters; i++) {
                        cmd = new char[255];
                        sprintf(cmd, "SELECT * from %s_expanded WHERE cluster = %i",table,i);
                        int count = sqlite_get_rows(cmd);
                        delete[] cmd;

                        fprintf(stderr, "Cluster #%i: %i\n",i,count);

                        // Designate the largest cluster
                        if (count > maxClusterCount) {
                                maxClusterCount = count;
                                maxCluster = i;
                        }
                }

                delete[] table_expanded;

                // Save down exported txt file with pr_contours_expanded with clustering results
                cmd = new char[255];
                sprintf(cmd, "SELECT * FROM %s_expanded",table);
                sqlite_output_table(cmd,pr_contours_expanded);
                delete[] cmd;


                // Find the min, max, and average of the max cluster dist between contours
                cmd = new char[255];
                sprintf(cmd, "SELECT min(dist) FROM %s_expanded WHERE cluster = %i",table,maxCluster);
                double minDist = sqlite_query_value(cmd);
                delete[] cmd;

                cmd = new char[255];
                sprintf(cmd, "SELECT max(dist) FROM %s_expanded WHERE cluster = %i",table,maxCluster);
                double maxDist = sqlite_query_value(cmd);
                delete[] cmd;

                cmd = new char[255];
                sprintf(cmd, "SELECT avg(dist) FROM %s_expanded WHERE cluster = %i",table,maxCluster);
                double avgDist = sqlite_query_value(cmd);
                delete[] cmd;

                fprintf(stderr,"min = %f, max = %f, mean = %f",minDist,maxDist,avgDist);

                // begin transaction
                sqlite_command("BEGIN TRANSACTION");

                /*
                 * Use the max cluster values to go back to frames
                 * that do not have any member of that cluster
                 * and re-admit contour pairs based on dist values
                 * that are obtained from the max cluster
                 */
                for (int i=MIN_FRAME; i<=MAX_FRAME; i++) {
                        // Find the number of contour pairs that have been marked with the max cluster in a given frame
                        cmd = new char[255];
                        sprintf(cmd, "SELECT * from %s_expanded WHERE cluster = %i AND frameNumber = %i",table,maxCluster,i);
                        int count = sqlite_get_rows(cmd);
                        delete[] cmd;

                        // If at least one contour pair marked with max cluster in a given frame,
                        // delete all other contour pairs that are not max cluster
                        if (count >= 1) {
                                cmd = new char[255];
                                sprintf(cmd, "DELETE FROM %s_expanded WHERE cluster != %i AND frameNumber = %i",table,maxCluster,i);
                                sqlite_command(cmd);
                                delete[] cmd;
                        }

                        // If no contour pair marked with max cluster in a given frame,
                        // delete all contour pairs in that frame that are less than minDist-2 and greater than maxDist+2
                        // which are values obtained from the original overclustering result
                        else {
                                cmd = new char[255];
                                sprintf(cmd, "DELETE FROM %s_expanded WHERE frameNumber = %i AND (dist < %f OR dist > %f)",table,i,minDist-2,maxDist+2);
                                sqlite_command(cmd);
                                delete[] cmd;
                        }
                }

                // commit transaction
                sqlite_command("COMMIT TRANSACTION");

                // get initial mean and SD
                cmd = new char[255];
                sprintf(cmd, "SELECT avg(dist) from %s_expanded WHERE cluster = %i",table,maxCluster);
                mean = sqlite_query_value(cmd);
                delete[] cmd;

                cmd = new char[255];
                sprintf(cmd, "SELECT stdev(dist) from %s_expanded WHERE cluster = %i",table,maxCluster);
                stdev = sqlite_query_value(cmd);
                delete[] cmd;
        }

        if (table == (char*)"pupil_contours") {
                // get initial mean and SD
                cmd = new char[255];
                sprintf(cmd, "SELECT avg(dist) from %s_expanded",table);
                mean = sqlite_query_value(cmd);
                delete[] cmd;

                cmd = new char[255];
                sprintf(cmd, "SELECT stdev(dist) from %s_expanded",table);
                stdev = sqlite_query_value(cmd);
                delete[] cmd;

                // print the initial mean and SD
                printf("Initial Mean, SD for Filtered Values:\n");
                printf("Mean: %f\n",mean);
                printf("SD: %f\n",stdev);

                // set min and max values for distances
                float max = mean + 3*stdev;
                float min = mean - 3*stdev;

                // Delete pairs where the distance between contour pairs
                // is more than three standard deviations from the mean
                cmd = new char[255];
                sprintf(cmd, "DELETE FROM %s_expanded WHERE dist < %f OR dist > %f", table, min, max );
                sqlite_command(cmd);
                delete[] cmd;

                // get updated mean and SD
                cmd = new char[255];
                sprintf(cmd, "SELECT avg(dist) from %s_expanded",table);
                mean = sqlite_query_value(cmd);
                delete[] cmd;

                cmd = new char[255];
                sprintf(cmd, "SELECT stdev(dist) from %s_expanded",table);
                stdev = sqlite_query_value(cmd);
                delete[] cmd;

                int count_prev = 0;
                cmd = new char[255];
                sprintf(cmd, "SELECT * from %s_expanded",table);
                int count = sqlite_get_rows(cmd);
                delete[] cmd;

                // begin transaction
                sqlite_command("BEGIN TRANSACTION");
                char* count_stmt;

                // Repeat filtering until standard deviation is less than 10 pixels
                while ( stdev > 10 && count != count_prev ) {
                        fprintf(stdout,"\nFiltered PR\n");

                        count_stmt = new char[255];
                        sprintf(count_stmt, "SELECT count(*) FROM %s_expanded",table);
                        sqlite_command(count_stmt);

                        cmd = new char[255];
                        sprintf(cmd, "DELETE FROM %s_expanded WHERE dist < %f OR dist > %f", table, min, max );
                        fprintf(stderr, "\n%s", cmd);

                        sqlite_command(cmd);
                        delete[] cmd;

                        fprintf(stdout,"\nFiltered PR AFTER\n");
                        sqlite_command(count_stmt);
                        delete[] count_stmt;

                        // get updated mean and SD
                        cmd = new char[255];
                        sprintf(cmd,"SELECT avg(dist) from %s_expanded",table);
                        mean = sqlite_query_value(cmd);
                        delete[] cmd;

                        cmd = new char[255];
                        sprintf(cmd,"SELECT stdev(dist) from %s_expanded",table);
                        stdev = sqlite_query_value(cmd);
                        delete[] cmd;

                        // print the updated mean and SD
                        printf("Updated Mean, SD for Filtered PRs:\n");
                        printf("Mean: %f\n",mean);
                        printf("SD: %f\n",stdev);

                        max = mean + 3*stdev;
                        min = mean - 3*stdev;

                        count_prev = count;
                        cmd = new char[255];
                        sprintf(cmd,"SELECT * from %s_expanded",table);
                        count = sqlite_get_rows(cmd);
                        delete[] cmd;

                        int counter = 0;
                        char* frameQuery;
                        int frameCount = 0;

                        // Inefficient code: optimize using SQLite statements...
                        for (int i=MIN_FRAME; i<MAX_FRAME; i++) {
                                frameQuery = new char[255];
                                sprintf(frameQuery,"SELECT count(*) from %s_expanded WHERE frameNumber = %i",table,i);
                                frameCount = sqlite_query_value(frameQuery);
                                delete[] frameQuery;

                                if(frameCount == 1) {
                                        cmd = new char[255];
                                        sprintf(cmd,"SELECT dist from %s_expanded WHERE frameNumber = %i",table,i);
                                        mean = mean + sqlite_query_value(cmd);
                                        counter++;
                                        delete[] cmd;
                                }
                        }

                        if (counter > 5) {
                                mean = mean / counter;
                        }

                        // print the target mean
                        printf("Target Mean for Filtered PRs:\n");
                        printf("Mean: %f\n",mean);
                }

                // commit transaction
                sqlite_command("COMMIT TRANSACTION");
        }

        char* select_frame;
        char* delete_stmt;

        // begin transaction
        sqlite_command("BEGIN TRANSACTION");

        for (int frameNum=MIN_FRAME; frameNum<=MAX_FRAME; frameNum++) {

                fprintf(stdout,"Current Frame = %i\n", frameNum );

                select_frame = new char[255];
                sprintf(select_frame, "SELECT * FROM %s_expanded WHERE frameNumber = %i", table, frameNum);
                int counter =  sqlite_get_rows(select_frame);
                fprintf(stdout,"Counter = %i\n", counter);

                if (counter > 1) {
                        fprintf(stdout,"Multiple PR contours found in frame %i\n", frameNum);

                        // DO ALTERNATIVE ROUTINE TO NARROW DOWN TO CLOSEST
                        // This is done by deleting duplicate distance values
                        // that are too far from the mean
                        // and are in the same frame
                        delete_stmt = new char[255];
                        sprintf(delete_stmt, "DELETE FROM %s_expanded WHERE frameNumber = %i AND abs(dist-%f) IS NOT (SELECT min(abs(dist-%f)) FROM %s_expanded where frameNumber = %i)", table, frameNum, mean, mean, table, frameNum);

                        sqlite_command(delete_stmt);
                        delete[] delete_stmt;
                }

                else if (counter == 0) {
                        fprintf(stdout,"No PR contours found in frame %i\n", frameNum);
                }

                counter =  sqlite_get_rows(select_frame);
                fprintf(stdout,"New Counter = %i\n", counter);
                delete[] select_frame;

                double rowID;
                // Remove any remaining duplicate values
                if (counter > 1) {
                        cmd = new char[255];
                        sprintf(cmd,"SELECT ROWID FROM %s_expanded WHERE frameNumber = %i",table,frameNum);
                        rowID = sqlite_query_value(cmd);
                        delete[] cmd;

                        fprintf(stderr, "\n Row ID: %f", rowID);

                        delete_stmt = new char[255];
                        sprintf(delete_stmt,"DELETE FROM %s_expanded WHERE frameNumber = %i AND ROWID != %f",table,frameNum,rowID);
                        sqlite_command(delete_stmt);
                        delete[] delete_stmt;
                }
        }

        // commit transaction
        sqlite_command("COMMIT TRANSACTION");

        cmd = new char[255];
        sprintf(cmd,"DROP TABLE IF EXISTS clustered_%s",table);
        sqlite_command(cmd);
        delete[] cmd;

        cmd = new char[255];
        sprintf(cmd,"CREATE TABLE IF NOT EXISTS clustered_%s (frameNumber INTEGER, x REAL, y REAL, w REAL, h REAL, cluster INTEGER)",table);
        sqlite_command(cmd);
        delete[] cmd;

        // Split expanded values into clustered contour table
        cmd = new char[255];
        sprintf(cmd,"INSERT OR REPLACE INTO clustered_%s (frameNumber, x, y, w, h, cluster) SELECT frameNumber, x1, y1, w1, h1, '0' FROM %s_expanded WHERE x1 < x2",table,table);
        sqlite_command(cmd);
        delete[] cmd;

        cmd = new char[255];
        sprintf(cmd,"INSERT OR REPLACE INTO clustered_%s (frameNumber, x, y, w, h, cluster) SELECT frameNumber, x2, y2, w2, h2, '0' FROM %s_expanded WHERE x1 > x2",table,table);
        sqlite_command(cmd);
        delete[] cmd;

        cmd = new char[255];
        sprintf(cmd,"INSERT OR REPLACE INTO clustered_%s (frameNumber, x, y, w, h, cluster) SELECT frameNumber, x1, y1, w1, h1, '1' FROM %s_expanded WHERE x1 > x2",table,table);
        sqlite_command(cmd);
        delete[] cmd;

        cmd = new char[255];
        sprintf(cmd,"INSERT OR REPLACE INTO clustered_%s (frameNumber, x, y, w, h, cluster) SELECT frameNumber, x2, y2, w2, h2, '1' FROM %s_expanded WHERE x2 > x1",table,table);
        sqlite_command(cmd);
        delete[] cmd;
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
                sqlite3_finalize(stmt);
                return NULL;
        }
        else if (ncolumns == 0) {
                printf("Query returns 0 columns\n");
                sqlite3_finalize(stmt);
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
                                const char* columnValue = (const char*) sqlite3_column_text(stmt,j);
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
                sqlite3_finalize(stmt);
                return data;
        }
}

Mat sqlite_query_to_mat(string query)
{
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

        Mat data;

        // put the results of the query into an array
        if (nrows == 0) {
                printf("Query returns 0 rows\n");
                sqlite3_finalize(stmt);
                return data;
        }
        else if (ncolumns == 0) {
                printf("Query returns 0 columns\n");
                sqlite3_finalize(stmt);
                return data;
        }
        else {
                data = Mat(Size(ncolumns,nrows),CV_32F);

                int i = 0;
                while ( sqlite3_step(stmt) == SQLITE_ROW ) {
                        for( int j = 0 ; j < ncolumns; j++) {
                                const char* columnValue = (const char*) sqlite3_column_text(stmt,j);
                                // handle null pointers by setting value to 0.0
                                if (columnValue == NULL ){
                                        data.at<float>(i,j) = 0.0;
                                }
                                else {
                                        data.at<float>(i,j) = strtof(columnValue,NULL);
                                }
                        }
                        i++;
                }
                sqlite3_finalize(stmt);
        }
        return data;

}

double sqlite_query_value( string query ) {

        double** result = sqlite_query_to_array(query);

        double resultVal = result[0][0];
        delete[] result;

        return resultVal;

}

// assumes db is a global variable
int sqlite_get_rows(string query) {

        // SQL variables
        sqlite3_stmt *stmt;

        // get the number of rows in the query
        char* rowsQuery = new char[255];

        strcpy(rowsQuery,"SELECT count(*) FROM ( ");
        strcat(rowsQuery,query.c_str());
        strcat(rowsQuery," )");

        rc = sqlite3_prepare_v2(db,rowsQuery,-1,&stmt,0);

        delete[] rowsQuery;

        // create exception if error occurs
        sqlite_exception(rc);

        int nrows = 0;
        while ( sqlite3_step(stmt) == SQLITE_ROW ) {
                nrows = atoi((const char*)sqlite3_column_text(stmt,0));
        }

        return nrows;
}

// assumes db is a global variable
void sqlite_exception(int rc) {

        // create exception if error occurs
        if( rc != SQLITE_OK ) {
                fprintf(error_log, "sqlite error: %s\n", sqlite3_errmsg(db));
                fprintf(stdout, "sqlite error: %s\n", sqlite3_errmsg(db));
                sqlite3_close(db);
                exit(1);
        }

        return;

}

int sqlite_get_columns(string query) {

        int ncolumns = 0;

        // SQL variables
        sqlite3_stmt *stmt;
        const char *tail;

        // execute the main query
        int rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

        // create exception if error occurs
        sqlite_exception(rc);

        // get the number of columns in the query
        ncolumns = sqlite3_column_count(stmt);
        sqlite3_finalize(stmt);

        return ncolumns;

}


// prints table info to the specified file
// assumes db is a global variable
// creates, writes to, and closes the file filename
void sqlite_print_table_info(FILE * file) {

        // SQL variables
        sqlite3_stmt *stmt;
        const char *tail;

        // get a list of the table names
        string query = "SELECT name FROM SQLITE_MASTER";
        rc = sqlite3_prepare_v2(db,query.c_str(),query.length(),&stmt,&tail);

        // create exception if error occurs
        sqlite_exception(rc);

        // print header for table info
        fprintf(file,"\nTABLE NAME\t\tROWS\n");

        char* rowsQuery;

        // cycle through the table names, print name and number of rows
        while ( sqlite3_step(stmt) == SQLITE_ROW ) {
                // parse table name
                string tableName = (const char*)sqlite3_column_text(stmt,0);

                // get number of rows in table
                rowsQuery = new char[255];
                sprintf(rowsQuery, "SELECT * FROM %s", tableName.c_str());
                int tableRows = sqlite_get_rows(rowsQuery);

                delete[] rowsQuery;

                // print information to file
                fprintf(file,"%s\t\t%i\n",tableName.c_str(),tableRows);
        }

        // clean_up
        sqlite3_finalize(stmt);

        return;
}

// add custom sqlite functions to DB
void sqlite_register_functions(sqlite3 * db) {
         void *pArg = 0;
         int rc = 0;
         rc = sqlite3_create_function(db,"sqrt", 1, SQLITE_UTF8, pArg, sqrtFunc, 0, 0);
         rc = sqlite3_create_function(db,"stdev",1, SQLITE_ANY, pArg, 0, varianceStep, stdevFinalize);
         rc = sqlite3_create_function(db,"atan",1, SQLITE_UTF8, pArg, atanFunc, 0, 0);
}

// custom sqlite functions

/*
** scalar sqrt Function
*/
static void sqrtFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  double rVal=0.0;
  int iVal=0;
  assert( argc==1 );
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      iVal = sqlite3_value_int64(argv[0]);
      sqlite3_result_int64(context, (int) sqrt(iVal));
      break;
    }
    case SQLITE_NULL: {
      sqlite3_result_null(context);
      break;
    }
    default: {
      rVal = sqlite3_value_double(argv[0]);
      sqlite3_result_double(context, (double) sqrt(rVal));
      break;
    }
  }
}

/*
 * standard deviation functions
 * called for each value received during a calculation of stdev or variance
 */

static void varianceStep(sqlite3_context *context, int argc, sqlite3_value **argv){
  StdevCtx *p;

  double delta;
  double x;

  assert( argc==1 );
  p = (StdevCtx*)sqlite3_aggregate_context(context, sizeof(*p));
  /* only consider non-null values */
  if( SQLITE_NULL != sqlite3_value_numeric_type(argv[0]) ){
    p->cnt++;
    x = sqlite3_value_double(argv[0]);
    delta = (x-p->rM);
    p->rM += delta/p->cnt;
    p->rS += delta*(x-p->rM);
  }
}

/*
 * Returns the stdev value
 */
static void stdevFinalize(sqlite3_context *context){
  StdevCtx *p;
  p = (StdevCtx*)sqlite3_aggregate_context(context, 0);
  if( p && p->cnt>1 ){
    sqlite3_result_double(context, sqrt(p->rS/(p->cnt-1)));
  }else{
    sqlite3_result_double(context, 0.0);
  }
}

int xMods[8] = {1, 1, 0, -1, -1, -1, 0, 1};
int yMods[8] = {0, 1, 1, 1, 0, -1, -1, -1};
void followEdges(Mat magnitude, int i, int j, float lowThreshold) {
        for (int k=0; k<8; k++) {
                //printf("\n Currently looking at position %i, %i ==> %i, %i",i,j, i+x[k],j+y[k]);
                //printf("\n Current counter = %i",globalCounter);
                if ( i+xMods[k] > 0 && i+xMods[k] < edges.rows && j+yMods[k] > 0 && j+yMods[k] < edges.cols) {
                        if ( (ELEM(double,edges.data,edges.step,edges.elemSize(),i+xMods[k],j+yMods[k]) == POSSIBLE_EDGE) && (ELEM(double,edges.data,edges.step,edges.elemSize(),i+xMods[k],j+yMods[k]) > lowThreshold) ) {
                                edges.at<double>(j+yMods[k],i+xMods[k]) = EDGE;
                                followEdges(magnitude,i+xMods[k],j+yMods[k],lowThreshold);
                        }
                }
        }
}

/*
** scalar atan Function
*/
static void atanFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  double rVal=0.0;
  int iVal=0;
  assert( argc==1 );
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      iVal = sqlite3_value_int64(argv[0]);
      sqlite3_result_int64(context, (int) atan(iVal));
      break;
    }
    case SQLITE_NULL: {
      sqlite3_result_null(context);
      break;
    }
    default: {
      rVal = sqlite3_value_double(argv[0]);
      sqlite3_result_double(context, (double) atan(rVal));
      break;
    }
  }
}

void accum_circle(Mat houghImg, int i, int j, int rows, int cols, int radius) {
        int f = 1 - radius;
        int ddF_x = 1;
        int ddF_y = -2 * radius;
        int x = 0;
        int y = radius;

        accum_pixel(houghImg, i, j + radius, rows, cols);
        accum_pixel(houghImg, i, j - radius, rows, cols);
        accum_pixel(houghImg, i + radius, j, rows, cols);
        accum_pixel(houghImg, i - radius, j, rows, cols);

        while (x < y) {
                if (f >= 0) {
                        y--;
                        ddF_y += 2;
                        f += ddF_y;
                }

                x++;
                ddF_x += 2;
                f += ddF_x;

                accum_pixel(houghImg, i + x, j + y, rows, cols);
                accum_pixel(houghImg, i - x, j + y, rows, cols);
                accum_pixel(houghImg, i + x, j - y, rows, cols);
                accum_pixel(houghImg, i - x, j - y, rows, cols);
                accum_pixel(houghImg, i + y, j + x, rows, cols);
                accum_pixel(houghImg, i - y, j + x, rows, cols);
                accum_pixel(houghImg, i + y, j - x, rows, cols);
                accum_pixel(houghImg, i - y, j - x, rows, cols);
        }
}

void accum_pixel(Mat houghImg, int i, int j, int rows, int cols) {
        // Checks boundary
        if (i < 0 || i >= rows || j < 0 || j >= cols ) {
                return;
        }
        houghImg.at<double>(i,j) = ELEM(double,houghImg.data,houghImg.step,houghImg.elemSize(),j,i)+1;
}

void draw_circle(Mat drawPupil, int i, int j, int rows, int cols, int radius) {
        int f = 1 - radius;
        int ddF_x = 1;
        int ddF_y = -2 * radius;
        int x = 0;
        int y = radius;

        draw_pixel(drawPupil, i, j + radius, rows, cols);
        draw_pixel(drawPupil, i, j - radius, rows, cols);
        draw_pixel(drawPupil, i + radius, j, rows, cols);
        draw_pixel(drawPupil, i - radius, j, rows, cols);

        while (x < y) {
                if (f >= 0) {
                        y--;
                        ddF_y += 2;
                        f += ddF_y;
                }

                x++;
                ddF_x += 2;
                f += ddF_x;

                draw_pixel(drawPupil, i + x, j + y, rows, cols);
                draw_pixel(drawPupil, i - x, j + y, rows, cols);
                draw_pixel(drawPupil, i + x, j - y, rows, cols);
                draw_pixel(drawPupil, i - x, j - y, rows, cols);
                draw_pixel(drawPupil, i + y, j + x, rows, cols);
                draw_pixel(drawPupil, i - y, j + x, rows, cols);
                draw_pixel(drawPupil, i + y, j - x, rows, cols);
                draw_pixel(drawPupil, i - y, j - x, rows, cols);
        }
}

void draw_pixel(Mat drawPupil, int i, int j, int rows, int cols) {
        // Checks boundary
        if (i < 0 || i >= rows || j < 0 || j >= cols ) {
                return;
        }

        // Draw pixel
        drawPupil.at<double>(i,j) = EDGE;
}

double* linReg(double** values, double x_mean, double y_mean, double count) {
        double* linRegVals = new double[3]; // b, a, SE

        double Sx = 0;
        for (int i=0; i<count; i++) {
                Sx = Sx + pow((values[i][3]-x_mean),2);
        }

        double Sy = 0;
        for (int i=0; i<count; i++) {
                Sy = Sy + pow((values[i][1]-y_mean),2);
        }

        double Sxy = 0;
        for (int i=0; i<count; i++) {
                Sxy = Sxy + (values[i][3]-x_mean)*(values[i][1]-y_mean);
        }

        double b = Sxy/Sx;
        double a = y_mean - b*x_mean;

        double Sy_hat = 0;
        for (int i=0; i<count; i++) {
                Sy_hat = Sy_hat + pow(values[i][1] - (a + b * values[i][3]),2);
        }

        double SE = sqrt(Sy_hat / (count-2)) / sqrt(Sx);

        linRegVals[0] = b;
        linRegVals[1] = a;
        linRegVals[2] = SE;

        return linRegVals;
}
